// Includes standard C++ libraries and project-specific headers
#include "../include/debug.h"
#include "../include/utils.h"
#include "../include/globals.h"
#include "bits/stdc++.h"

using namespace std;
using namespace chrono;

// Function to log debug data
void startDebugMonitor()
{
    while(true)
    {
        {
            // Safely lock and report the current number of news items waiting to be processed
            lock_guard<mutex> lock(newsQueueMutex);
            safeCout("[DEBUG] ", "Current newsQueue size: " + to_string(newsQueue.size()) + "\n\n");
        }

        {
            // Safely lock and report the number of processed company status entries
            lock_guard<mutex> lock(companyStatusQueueMutex);
            safeCout("[DEBUG] ", "Current companyStatusQueue size: " + to_string(companyStatusQueue.size()) + "\n\n");
        }

        // Wait for 10 seconds before logging again to avoid flooding the output
        this_thread::sleep_for(seconds(10));
    }
}
