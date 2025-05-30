// Includes standard C++ libraries and project-specific headers
#include <bits/stdc++.h>
#include "../include/globals.h"

// === Declare global variables ===

// Global balance in USD
double balance = 10000;

// Global ofstream to write logs to file
ofstream logFile;

// Mutexes to protect global queues
mutex newsQueueMutex;
mutex companyStatusQueueMutex;

// Global queue to store parsed news items
queue<NewsItem> newsQueue;

// Global set to track already-seen URLs
set<string> seenUrls;

// Global queue to store company sentiment analysis results
queue<CompanyStatus> companyStatusQueue;

// Config object to hold configuration settings, loaded from a configuration file
Config cfg;

// Global strategy object with default investment parameters
InvestmentStrategy strategy = {
    0.01,   // Invest 1% of portfolio
    0.9,    // Buy if sentiment >= 0.9
    -0.3,   // Sell if sentiment <= -0.3
    0.7,    // Match if similarity >= 0.7
    86400   // Hold for 24 hours (86400 seconds)
};

// Global custom investment list instance
InvestmentList investmentList;

// Dictionary mapping stock symbols to all investment nodes of that symbol
unordered_map<string, vector<InvestmentNode*>> stockInvestmentsMap;

// === Function definitions for InvestmentList ===

// Insert a new investment at the end of the list
void InvestmentList::insertBack(string symbol, double amount, time_point<system_clock> sellTime)
{
    // Allocate new node with given values
    auto* node = new InvestmentNode{symbol, amount, sellTime, nullptr, tail};
    
    // If the list is empty, both head and tail should point to the new node
    if(tail == nullptr)
    {
        head = node;
        tail = node;
    }
    else
    {
        // If the list is not empty, append to the end and update the tail
        tail->next = node;
        tail = node;
    }
}

// Returns a pointer to the first node in the list
InvestmentNode* InvestmentList::front()
{
    return head;
}

// Removes the first investment from the list
void InvestmentList::popFront()
{
    // If list is already empty, do nothing
    if(head == nullptr)
    {
        return;
    }

    // Move head to the next node
    InvestmentNode* tmp = head;
    head = head->next;
    
    // If list is now empty, reset tail as well
    if(head == nullptr)
    {
        tail = nullptr;
    }
    else
    {
        // If list is not empty, clear head's prev pointer
        head->prev = nullptr;
    }

    // Free memory of the old head
    delete tmp;
}