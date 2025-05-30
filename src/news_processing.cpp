// Includes standard C++ libraries and project-specific headers
#include "../include/news_processing.h"
#include "../include/globals.h"
#include "../include/utils.h"
#include <bits/stdc++.h>

using namespace std;

// Function to detect the most relevant company mentioned in the news article
tuple<string, string, double> detectCompanyInNews(string articleTitle, string articleText)
{
    string command = cfg.pythonInterpreter + " \"scripts/company_matcher.py\""
                     + " \"" + cfg.astraDBApplicationToken + "\""
                     + " \"" + cfg.astraDBApiEndpoint        + "\""
                     + " \"" + cfg.openAIApi                 + "\""
                     + " \"" + articleTitle                  + "\""
                     + " \"" + articleText                   + "\""
                     + " 2>&1";

    // Open a pipe to execute the command and capture its output
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        // If the pipe couldn't be opened, throw an exception
        throw runtime_error("Failed to open pipe to company_matcher.py");
    }

    // Read the script's output into a result string
    string result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }

    // Close the pipe and check the script's exit status
    int returnCode = _pclose(pipe);
    if (returnCode != 0)
    {
        throw runtime_error("company_matcher.py failed with code " + to_string(returnCode));
    }

    // Remove newline characters and outer brackets from the JSON array
    result.erase(remove(result.begin(), result.end(), '\n'), result.end());
    result.erase(remove(result.begin(), result.end(), '['), result.end());
    result.erase(remove(result.begin(), result.end(), ']'), result.end());

    // Initialize variables for extracted values
    string name, ticker;
    float similarity = 0.0f;

    // Locate the positions of each field in the JSON string
    size_t namePos = result.find("\"name\":");
    size_t tickerPos = result.find("\"ticker\":");
    size_t simPos = result.find("\"similarity\":");

    // Validate that all required fields were found
    if (namePos == string::npos || tickerPos == string::npos || simPos == string::npos)
    {
        throw runtime_error("Failed to parse company_matcher.py output");
    }

    // Extract company name value
    namePos = result.find('"', namePos + 7) + 1;
    size_t nameEnd = result.find('"', namePos);
    name = result.substr(namePos, nameEnd - namePos);

    // Extract ticker symbol value
    tickerPos = result.find('"', tickerPos + 9) + 1;
    size_t tickerEnd = result.find('"', tickerPos);
    ticker = result.substr(tickerPos, tickerEnd - tickerPos);

    // Extract similarity score and convert to float
    simPos = result.find(':', simPos) + 1;
    string simStr = result.substr(simPos, result.find('}', simPos) - simPos);
    similarity = stof(simStr);

    // Return extracted values as a tuple
    return {name, ticker, similarity};
}


// Sentiment analysis function to extract positivity and score
float analyzeSentiment(string title, string text)
{
    string result;

    // Command to call the Python script
    string command = cfg.pythonInterpreter +
                     " \"scripts/sentiment_analyzer.py\"" +
                     " \"" + (cfg.useGPU ? "True" : "False") + "\"" +
                     " \"" + title + "\"" +
                     " \"" + text  + "\"" +
                     " 2>&1";

    // Open a pipe to execute the command and read its output
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        // If the pipe couldn't be opened, throw an exception
        throw runtime_error("Failed to open pipe.");
    }

    // Buffer for reading the script's output in chunks
    char buffer[4096];

    // Read the output line by line and accumulate it in the result string
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }

    // Close the pipe and get the script's exit status
    int returnCode = _pclose(pipe);

    // If the Python script exited with an error code, throw an exception
    if (returnCode != 0)
    {
        throw runtime_error("sentiment_analysis.py failed with code " + to_string(returnCode));
    }

    // Extract the label (POSITIVE, NEGATIVE, NEUTRAL)
    size_t labelPos = result.find("{'label': '");
    if (labelPos == string::npos)
    {
        throw runtime_error("Failed to find 'label' in sentiment analysis result.");
    }

    labelPos += 11;  // Skip over "{'label': '"
    size_t labelEnd = result.find("'", labelPos);
    if (labelEnd == string::npos)
    {
        throw runtime_error("Failed to extract label.");
    }

    string label = result.substr(labelPos, labelEnd - labelPos);

    // Extract the score
    size_t scorePos = result.find("'score': ", labelEnd);
    if (scorePos == string::npos)
    {
        throw runtime_error("Failed to find 'score' in sentiment analysis result.");
    }

    scorePos += 9;  // Skip over "'score': "
    size_t scoreEnd = result.find('}', scorePos);
    if (scoreEnd == string::npos)
    {
        throw runtime_error("Failed to extract score.");
    }

    float score = stof(result.substr(scorePos, scoreEnd - scorePos));  // Convert string to float

    // Convert to signed value in [-1.0, 1.0]
    if(label == "NEUTRAL")
    {
        score = 0;
    }
    else if(label == "NEGATIVE")
    {
        score = -score;
    }

    // Return the extracted values
    return score;
}

// Function to process news articles from the global queue of parsed news items
void processNewsArticles()
{
    // Log the start of the processNewsArticles thread
    safeCout("[INFO] ", "Starting thread: processNewsArticles\n\n");

    while(true) 
    {
        // Check if there are news articles in the queue to process
        NewsItem currentNews;
        bool hasNews = false;

        {
            lock_guard<mutex> lock(newsQueueMutex);
            if(!newsQueue.empty()) 
            {
                // Get the oldest item from the queue (FIFO order)
                currentNews = newsQueue.front();  // Remove the processed article from the queue
                newsQueue.pop();
                hasNews = true;
            }
        }

        if(hasNews) 
        {
            try
            {
                // Extract the title and text from the news item
                string title = currentNews.title;
                string text = currentNews.text;

                // Detect a company in the article using the detectCompanyInNews function
                tuple<string, string, double> company = detectCompanyInNews(title, text);

                string companyName = get<0>(company);
                string stockSymbol = get<1>(company);
                double similarity = get<2>(company);

                // Create a CompanyStatus object with the results
                CompanyStatus status;
                status.companyName = companyName;
                status.stockSymbol = stockSymbol;
                status.similarity = similarity;
                status.sentimentScore = analyzeSentiment(title, text);   // Perform sentiment analysis for the article
                status.timeAdded = currentNews.publishedAt;         // Add the timestamp
                
                // Add the CompanyStatus to the companyStatusQueue for further processing
                {
                    lock_guard<mutex> lock(companyStatusQueueMutex);
                    companyStatusQueue.push(status);
                }

                // Confirm successful processing
                safeCout("[INFO] ", "Sentiment for " + companyName + " (" + stockSymbol + ") from url " + currentNews.url + "\n - successfully processed and added to queue.\n\n");
            }
            catch (const exception& e)
            {
                // Handle errors during sentiment analysis
                safeCerr("[Error] ", string(e.what()) + "\n");
            }
        }
    }
}

