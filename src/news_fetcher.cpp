// Includes standard C++ libraries and project-specific headers
#include "../include/news_fetcher.h"
#include "../include/utils.h"
#include "../include/news_parser.h"
#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

// Function to run the Python script with the given MarketAux API URL and capture its output
string fetchNewsFromAPI(const string apiUrl) 
{
    string result;

    // Build the command:
    // - Calls the specific Python interpreter
    // - Passes the full API URL to marketaux_fetcher.py
    // - Redirects stderr (2) to stdout (1) to capture all output
    string command = cfg.pythonInterpreter + " \"scripts/marketaux_fetcher.py\" \"" + apiUrl + "\" 2>&1";

    // Open a pipe to execute the command
    FILE* pipe = _popen(command.c_str(), "r");
    if(!pipe) 
    {
        throw runtime_error("Failed to open pipe.");
    }

    // Buffer for reading chunks of output from the pipe
    char buffer[4096];

    // Read and accumulate the output
    while(fgets(buffer, sizeof(buffer), pipe) != nullptr) 
    {
        result += buffer;
    }

    // Close the pipe and retrieve the exit status of the script
    int returnCode = _pclose(pipe);

    // If the script returned an error code, throw an exception with the code
    if(returnCode != 0)
    {
        throw runtime_error("marketaux_fetcher.py failed with code " + to_string(returnCode));
    }

    // Return the result from the script
    return result;
}

// Function to continuously poll news data
void newsPolling()
{
    // Log the start of the newsPolling thread
    safeCout("[INFO] ", "Starting thread: newsPolling\n\n");

    // Get the initial start time for the first polling window
    string previousTime = getUTCTimeOffset(cfg.lookBackSeconds);

    // Start polling loop
    while(true) 
    {
        // Get current time for the upper bound of the query window
        string currentTime = getUTCTimeOffset(0);

        // Construct API request URL with published_after filter
        string apiUrl = cfg.marketAuxBaseApi + "&published_after=" + previousTime + "&published_before=" + currentTime;;
        
        // Print log with the time window being requested
        safeCout("[INFO] ", "Requesting news published between " + previousTime + " and " + currentTime + "\n\n");
        try
        {
            // Fetch MarketAux news for the given time window
            string response = fetchNewsFromAPI(apiUrl);
            
            // Extract news items from the raw JSON string and add them to the global queue (newsQueue)
            // Process the response in a new thread so it's non-blocking 
            thread processorThread(extractNewsFromResponse, response);

            // We don't wait for it to finish
            processorThread.detach();
        } 
        catch(const exception& e) 
        {
            // Handle errors
            safeCerr("[Error] ", string(e.what()) + "\n");
        }

        // Update the starting point for the next request window
        previousTime = currentTime;

        // Wait before making the next request
        safeCout("[INFO] ", "Waiting " + to_string(cfg.delaySeconds) + " seconds before next request..." + "\n\n");
        
        // Pause execution for the configured delay time
        this_thread::sleep_for(seconds(cfg.delaySeconds));
    }
}
