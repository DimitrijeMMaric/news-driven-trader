// Includes standard C++ libraries and project-specific headers
#include "../include/news_parser.h"
#include "../include/utils.h"
#include "../include/globals.h"
#include <bits/stdc++.h>

using namespace std;

// Function to run the Python script with the given article URL and capture its output
string getArticleText(string url) 
{
    string result;

    // Build the command:
    // - Uses the specified Python interpreter
    // - Executes article_scraper.py with the provided URL
    // - Redirects stderr (2) to stdout (1) to capture all output together
    string command = cfg.pythonInterpreter + " \"scripts/article_scraper.py\" \"" + url + "\" 2>&1";

    // Open a pipe to execute the command and read its output
    FILE* pipe = _popen(command.c_str(), "r");
    if(!pipe)
    {
        // If the pipe couldn't be opened, throw an exception
        throw runtime_error("Failed to open pipe.");
    }

    // Buffer for reading the script's output in chunks
    char buffer[4096];

    // Read the output line by line and accumulate it in the result string
    while(fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }

    // Close the pipe and get the script's exit status
    int returnCode = _pclose(pipe);

    // If the Python script exited with an error code, throw an exception
    if(returnCode != 0)
    {
        throw runtime_error("article_scraper.py failed with code " + to_string(returnCode));
    }

    // Return the full article text (or output) from the script
    return result;
}

// Function to extract news items from the raw JSON string and add them to the global queue
void extractNewsFromResponse(const string response)
{
    size_t pos = 0;

    while(true)
    {
        // Find the start of a new article by locating the next "title" field
        pos = response.find("\"title\":", pos);
        
        // Exiting the loop if there are no more articles found in the response
        if(pos == string::npos) 
        {
            safeCout("[INFO] ", "No more articles to extract from JSON response.\n\n");
            break;
        }

        NewsItem item;

        // Extract title
        size_t start = response.find("\"", pos + 8) + 1;
        size_t end = response.find("\"", start);
        item.title = response.substr(start, end - start);
        pos = end;

        // Extract url
        pos = response.find("\"url\":", pos);
        if(pos == string::npos) 
        {
            break;
        }
        start = response.find("\"", pos + 6) + 1;
        end = response.find("\"", start);
        item.url = response.substr(start, end - start);
        pos = end;

        
        // Extract published_at
        pos = response.find("\"published_at\":", pos);
        if(pos == string::npos) 
        {
            break;
        }
        start = response.find("\"", pos + 16) + 1;
        end = response.find("\"", start);
        item.publishedAt = response.substr(start, end - start);
        pos = end;

        // Skip duplicates if we've already seen this URL
        if(seenUrls.count(item.url))
        {
            safeCout("[INFO] ", "Skipping duplicate: " + item.url + "\n\n");
            continue;
        }

        // Try to fetch full article text from the URL using article_scraper.py
        try
        {
            // Get the full article text by calling the Python script
            item.text = getArticleText(item.url);
        
            // Push the item into the global queue
            {
                lock_guard<mutex> lock(newsQueueMutex);
                newsQueue.push(item);
            }
            
            // Marks an item as seen
            seenUrls.insert(item.url);
            
            // Confirm successful processing
            safeCout("[INFO] ", "News article from URL: " + item.url + "\n - successfully parsed and added to queue.\n\n");
        }
        catch(const exception& e)
        {
            // Print any error that occurred
            safeCerr("[WARN] ", "News article from URL: " + item.url + "\n - failed to be parsed and was not added to queue.\n");
            safeCerr("[ERROR] ", string(e.what()) + "\n\n");  // print the specific error
        }
    }
}