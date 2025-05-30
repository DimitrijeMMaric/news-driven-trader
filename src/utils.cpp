// Includes standard C++ libraries and project-specific headers
#include "../include/utils.h"
#include "../include/globals.h"
#include <bits/stdc++.h>

using namespace std;

// === Thread-Safe Output Functions ===

// Define the global mutex declared in the header
mutex ioMutex;

// Prevents output overlap when multiple threads print simultaneously
void safeCout(string label, string value)
{
    // Ensure thread-safe access to output
    lock_guard<mutex> lock(ioMutex);

    // Construct the full message with timestamp and label
    string fullMessage = "[" + getUTCTimeOffset(0) + "] " + label + value;

    // Print to console
    cout << fullMessage;

    // If logging to file is enabled and the file is open, also write to the log file
    if(cfg.logToFile && logFile.is_open())
    {
        logFile << fullMessage;
        logFile.flush();  // Ensure the message is immediately written
    }
}

void safeCerr(string label, string value)
{
    // Ensure thread-safe access to output
    lock_guard<mutex> lock(ioMutex);

    // Construct the full message with timestamp and label
    string fullMessage = "[" + getUTCTimeOffset(0) + "] " + label + value;

    // Print to console
    cerr << fullMessage;

    // If logging to file is enabled and the file is open, also write to the log file
    if(cfg.logToFile && logFile.is_open())
    {
        logFile << fullMessage;
        logFile.flush();  // Ensure the message is immediately written
    }
}

// === Time Utility Functions ===

// Returns UTC time offset by `secondsAgo` in ISO 8601 format
string getUTCTimeOffset(int secondsAgo)
{
    time_t now = time(nullptr) - secondsAgo;
    tm gmtm = *gmtime(&now);
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &gmtm);
    return string(buffer);
}

// === Settings Functions ===

// Loads configuration settings from a file into a Config struct
Config loadSettings(string filename)
{
    // Create an empty Config object to hold the settings
    Config cfg; 

    // Open the specified file for reading
    ifstream file(filename); 
    if(!file.is_open())
    {
        // If the file couldn't be opened, throw an exception
        throw runtime_error("Failed to open settings file: " + filename);
    }
    
    // Read the file line by line
    string line;
    while(getline(file, line))
    {
        // Find the position of the '=' delimiter
        size_t delimiterPos = line.find('=');

        // Skip the line if no '=' is found (invalid format)
        if(delimiterPos == string::npos) continue;

        // Split the line into key and value
        string key = line.substr(0, delimiterPos);
        string value = line.substr(delimiterPos + 1);

        // Assign the value to the correct field in the Config struct
        if(key == "lookBackSeconds") cfg.lookBackSeconds = stoi(value);     // convert to int
        else if(key == "delaySeconds") cfg.delaySeconds = stoi(value);      // convert to int
        else if(key == "debug") cfg.debug = (value == "true");              // convert to bool
        else if(key == "logToFile") cfg.logToFile = (value == "true");      // convert to bool
        else if(key == "useGPU") cfg.useGPU = (value == "true");            // convert to bool
        else if(key == "pythonInterpreter") cfg.pythonInterpreter = value;
        else if(key == "marketAuxBaseApi") cfg.marketAuxBaseApi = value;
        else if(key == "astraDBApplicationToken") cfg.astraDBApplicationToken = value;
        else if(key == "astraDBApiEndpoint") cfg.astraDBApiEndpoint = value;
        else if(key == "openAIApi") cfg.openAIApi = value;
    }

    return cfg; // Return the populated Config struct
}
