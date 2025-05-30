#ifndef GLOBALS_H
#define GLOBALS_H

// Includes most standard C++ libraries
#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

// === Define global structures === 

// Structure to hold a single news item retrieved from an API
struct NewsItem
{
    string title;        // Title of the news article
    string url;          // URL to the full news article
    string publishedAt;  // Publication timestamp in string format
    string text;         // Full article text
};

// Structure to represent the sentiment analysis result for a company
struct CompanyStatus
{
    string companyName;       // Detected company name
    string stockSymbol;       // Ticker symbol of the company
    double similarity;        // Confidence level in company match (0 to 1)
    double sentimentScore;    // Sentiment polarity score (-1 to 1)
    string timeAdded;         // Timestamp when this item was added to the queue
};

// Structure to hold application-wide configuration settings
struct Config
{
    // Time window (in seconds) to look back when querying for news.
    int lookBackSeconds;

    // Delay (in seconds) between successive API requests to avoid rate limits.
    int delaySeconds;

    // If true, enables verbose debug logging to stdout.
    bool debug;

    // If true, also writes logs to an external file.
    bool logToFile;

    // If true, program will use GPU for sentiment analysis
    bool useGPU;

    // Full path to the Python interpreter to use when invoking scripts.
    string pythonInterpreter;

    // Base URL (including API key) for MarketAux news API.
    string marketAuxBaseApi;

    // Authentication token for AstraDB (DataStax) application.
    string astraDBApplicationToken;

    // REST API endpoint URL for AstraDB.
    string astraDBApiEndpoint;

    // API key for OpenAI services.
    string openAIApi;
};

// Structure to define the parameters of an investment strategy
struct InvestmentStrategy
{
    double investmentPercentage;        // Portion of portfolio to invest per action
    double positiveSentimentThreshold;  // Threshold above which to buy
    double negativeSentimentThreshold;  // Threshold below which to sell
    double similarityThreshold;         // Similarity confidence to accept a match
    int holdTimeSeconds;                // Duration to hold stock before selling (in seconds)
};

// Node structure representing an individual investment
struct InvestmentNode
{
    string stockSymbol;                   // Company ticker
    double stockAmount;                   // Number of shares purchased
    time_point<system_clock> sellTime;    // Scheduled sell time
    InvestmentNode* next;                 // Pointer to the next node in the list
    InvestmentNode* prev;                 // Pointer to the previous node
};

// A custom doubly-linked list to track all active investments
struct InvestmentList
{
    InvestmentNode* head = nullptr;  // Points to the first investment in the list
    InvestmentNode* tail = nullptr;  // Points to the last investment

    // Insert a new investment at the end of the list
    void insertBack(string symbol, double amount, time_point<system_clock> sellTime);

    // Returns a pointer to the first node in the list
    InvestmentNode* front();

    // Removes the first investment from the list
    void popFront();
};

// === Declare global variables === 

// Global balance 
extern double balance;

// Global ofstream to write logs to file
extern ofstream logFile;

// Mutexes to protect global queues
extern mutex newsQueueMutex;
extern mutex companyStatusQueueMutex;

// Global queue to store parsed news items
extern queue<NewsItem> newsQueue;

// Global set to track URLs that have already been processed
extern set<string> seenUrls;

// Global queue for storing company sentiment analysis results
extern queue<CompanyStatus> companyStatusQueue;

// Config object to hold configuration settings, loaded from a configuration file
extern Config cfg;

// Global strategy parameters object
extern InvestmentStrategy strategy;

// Global custom investment list instance
extern InvestmentList investmentList;

// Dictionary mapping stock symbols to all investment nodes of that symbol
extern unordered_map<string, vector<InvestmentNode*>> stockInvestmentsMap;

#endif // GLOBALS_H
