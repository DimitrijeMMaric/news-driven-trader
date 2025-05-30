#ifndef NEWS_PARSER_H
#define NEWS_PARSER_H

// Includes most standard C++ libraries
#include <bits/stdc++.h>

using namespace std;

// Function to run the Python script with the given article URL and capture its output
string getArticleText(string url);

// Function to extract news items from the raw JSON string and add them to the global queue
void extractNewsFromResponse(const string response);

#endif // NEWS_PARSER_H
