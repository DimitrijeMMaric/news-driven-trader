#ifndef UTILS_H
#define UTILS_H

// Includes standard C++ libraries and project-specific headers
#include <bits/stdc++.h>
#include "globals.h"

using namespace std;

// === Thread-Safe Output Utilities ===

// Global mutex declaration to synchronize output across threads
extern mutex ioMutex;

// Safely prints a labeled message to standard output (stdout)
void safeCout(string label, string value);

// Safely prints a labeled message to standard error (stderr)
void safeCerr(string label, string value);

// === Time Utility Functions ===

// Returns the current UTC time minus a given number of seconds,
// formatted in ISO 8601 (e.g., "2025-03-28T15:00:00")
string getUTCTimeOffset(int secondsAgo);

// === Settings Functions ===

// Loads configuration settings from a file into a Config struct
Config loadSettings(string filename);

#endif // UTILS_H
