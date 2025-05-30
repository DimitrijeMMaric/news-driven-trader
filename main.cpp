// Includes standard C++ libraries and project-specific headers
#include "include/utils.h"
#include "include/news_fetcher.h"
#include "include/news_parser.h"
#include "include/globals.h"
#include "include/news_processing.h"
#include "include/debug.h"
#include "include/investment_strategy.h"
#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

int main() 
{
    // Vector to store all threads
    vector<thread> threads;

    // Loads configuration settings from file
    try
    {
        // Load configuration settings into the Config struct
        string filename = "settings.cfg";
        cfg = loadSettings(filename); 
        safeCout("[INFO] ", "Settings successfully loaded from: " + filename + "\n\n");
        
        // Start the startDebugMonitor function in another thread if debug is enabled in settings
        if(cfg.debug)
        {
            thread(startDebugMonitor).detach();  // Debug thread runs independently
        }
        
        // Creates a log file if logging is enabled in settings
        if(cfg.logToFile)
        {
            // Create the "logs" directory if it doesn't already exist
            filesystem::create_directory("logs");

            // Create log file with timestamp as name
            string logFileName = "logs/" + getUTCTimeOffset(0) + ".log";
            replace(logFileName.begin(), logFileName.end(), ':', '-'); // Windows doesn't allow ":" in filenames
            logFile.open(logFileName);

            // Log the start of the application
            safeCout("[INFO] ", "Log file initialized: " + logFileName + "\n\n");
        }
        
        // Start the newsPolling function in a new thread
        threads.push_back(thread(newsPolling)); 

        // Start the processNewsArticles function in another thread
        threads.push_back(thread(processNewsArticles)); 

        // Start the executeInvestmentStrategy function in another thread
        threads.push_back(thread(executeInvestmentStrategy));

        // Join all threads before exiting the main function to ensure they finish execution
        for (auto& t : threads) 
        {
            t.join();  // Wait for each thread to finish execution
        }
    }
    catch(const exception& e)
    {
        // Handle errors and display the exception message
        safeCerr("[Error] ", string(e.what()) + "\n\n");
        return 0;
    }

    // Log that the application is exiting normally after all threads have completed
    safeCout("[INFO] ", "Application shutting down cleanly.\n");

    return 0;
}
