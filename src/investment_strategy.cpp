// Includes standard C++ libraries and project-specific headers
#include <bits/stdc++.h>
#include "../include/investment_strategy.h"
#include "../include/utils.h"
#include "../include/globals.h"

using namespace std;
using namespace chrono;

// Executes the logic for buying stocks
void executeBuyLogic(string symbol)
{
    safeCout("[ACTION] ", "Buying " + to_string(strategy.investmentPercentage * 100) + "% of portfolio in " + symbol + " due to positive sentiment.\n\n");
    
    // Implement with stock market API
}

// Executes the logic for selling stocks
void executeSellLogic(string symbol)
{
    safeCout("[ACTION] ", "Selling all holdings of " + symbol + " due to negative sentiment.\n");

    // Implement with stock market API
}

// Main function to continuously evaluate investment decisions
void executeInvestmentStrategy()
{
    safeCout("[INFO] ", "Starting thread: executeInvestmentStrategy\n\n");

    // Infinite loop to continuously process incoming company sentiment data
    while (true)
    {
        CompanyStatus currentStatus;    // Stores the current company status for processing
        bool hasStatus = false;         // Flag indicating availability of new data

        // Thread-safe extraction of the next available company status from the queue
        {
            lock_guard<mutex> lock(companyStatusQueueMutex);
            if (!companyStatusQueue.empty())
            {
                currentStatus = companyStatusQueue.front(); // Retrieve next status
                companyStatusQueue.pop();                   // Remove from queue
                hasStatus = true;                           // Mark that data is ready
            }
        }

        if (hasStatus)
        {
            string symbol = currentStatus.stockSymbol;      // Stock symbol of the company
            double sentimentScore = currentStatus.sentimentScore; // Sentiment score from analysis
            double similarity = currentStatus.similarity;   // Similarity score for company identification

            // Check if the similarity meets the threshold for reliable identification
            if (similarity < strategy.similarityThreshold)
            {
                // Skip to next iteration if similarity is too low
                safeCout("[INFO] ", "Similarity below threshold for " + symbol + ", skipping...\n\n");
                continue;
            }

            // Decide actions based on sentiment thresholds
            if (sentimentScore >= strategy.positiveSentimentThreshold)
            {
                // Trigger buy logic if sentiment is strongly positive
                executeBuyLogic(symbol);
            }
            else if (sentimentScore <= strategy.negativeSentimentThreshold)
            {
                // Trigger immediate sell logic if sentiment is strongly negative
                
                // Implement with stock market API
            }
            else
            {
                // Log ignored signal due to sentiment being neither strongly positive nor negative
                safeCout("[INFO] ", "Sentiment outsite both thresholds for " + symbol + ", skipping... \n\n");
                continue;
            }
        }

        // Check for holdings exceeding the hold duration
        // to do
    }
}
