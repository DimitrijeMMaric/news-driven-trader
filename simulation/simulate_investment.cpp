// Includes standard C++ libraries for data structures, I/O, algorithms, and time handling
#include <bits/stdc++.h>
using namespace std;
using namespace chrono;

// --- Data structures to store news and price entries ---

// Represents a news event with associated company ticker, text similarity score,
// sentiment score, and publication timestamp
struct NewsEntry
{
    string ticker;                      // Stock symbol associated with the news
    double similarity;                  // Similarity to company's profile
    double sentiment;                  // Sentiment score of the news
    system_clock::time_point timestamp; // Time when the news was published
};

// Represents a price record for a ticker at a specific timestamp
struct PriceEntry 
{
    string ticker;                      // Stock symbol for the price data
    double price;                       // Price of the stock at the given time
    system_clock::time_point timestamp; // Time of the price record
};

// Represents a held position: quantity bought, purchase price and time
struct InvestmentNode 
{
    string ticker;                      // Stock symbol for the investment
    double quantity;                    // Number of shares bought
    double buyPrice;                    // Price per share at purchase
    system_clock::time_point buyTime;   // Timestamp when position was opened
};

// --- Global containers used during simulation ---

// Mapping from ticker to its historical price records (sorted by time)
map<string, vector<PriceEntry>> priceData;
// Sequence of news events to feed into the simulation
vector<NewsEntry> newsData;
// Map of open positions per ticker: each holds a deque of InvestmentNode
map<string, deque<InvestmentNode>> investmentMap;
// Counter for how many simulation runs ended in profit
int profitableSimulations = 0;

// --- Function: simulate ---
// Runs a single simulation with the given strategy parameters over loaded data.
// investPercent: fraction of cash to allocate on each buy
// buyThreshold: minimum sentiment score to trigger a buy
// sellThreshold: maximum sentiment score to trigger a sell
// similarityThreshold: minimum similarity to consider an event
// holdDurationSecs: time (in seconds) to hold before forced sell
// totalTrades: output parameter for count of buy actions
// profitableTrades: output for count of profitable sells
// winTrades/lossTrades: vectors to store P&L values for wins and losses
// Returns final cash balance after running through all events

double simulate(double investPercent,
                double buyThreshold,
                double sellThreshold,
                double similarityThreshold,
                int holdDurationSecs,
                int &totalTrades,
                int &profitableTrades,
                vector<double> &winTrades,
                vector<double> &lossTrades)
{
    double balance = 10000.0;       // Starting cash balance
    totalTrades = profitableTrades = 0;
    winTrades.clear();
    lossTrades.clear();

    // Iterate through each news event chronologically
    for (auto const& news : newsData) 
    {
        // Skip events with low similarity
        if (news.similarity < similarityThreshold) continue;

        // Find the corresponding price at or just after the news timestamp
        auto& prices = priceData[news.ticker];
        auto it = lower_bound(prices.begin(), prices.end(), news.timestamp,
            [](auto const& p, auto const& t){ return p.timestamp < t; });
        if (it == prices.end()) continue;
        double currentPrice = it->price;

        // --- SELL: negative sentiment triggers closing all positions for ticker ---
        if (news.sentiment <= sellThreshold) {
            auto& invList = investmentMap[news.ticker];
            while (!invList.empty()) 
            {
                auto node = invList.front();
                double pnl = node.quantity * (currentPrice - node.buyPrice);
                if (pnl > 0) 
                {
                    ++profitableTrades;
                    winTrades.push_back(pnl);
                } 
                else 
                {
                    lossTrades.push_back(pnl);
                }
                balance += node.quantity * currentPrice;
                invList.pop_front();
            }
        }
        // --- BUY: positive sentiment triggers opening a new position ---
        else if (news.sentiment >= buyThreshold) 
        {
            double toInvest = balance * investPercent;
            double qty = toInvest / currentPrice;
            balance -= toInvest;
            ++totalTrades;
            investmentMap[news.ticker].push_back({
                news.ticker, qty, currentPrice, news.timestamp
            });
        }

        // --- Force SELL: positions exceeding hold duration are closed ---
        for (auto& [_, queue] : investmentMap) 
        {
            auto& invList = queue;
            while (!invList.empty() &&
                   duration_cast<seconds>(news.timestamp - invList.front().buyTime).count()
                       >= holdDurationSecs)
            {
                auto node = invList.front();
                auto sellTime = node.buyTime + seconds(holdDurationSecs);
                // Find price at forced sell time
                auto it2 = lower_bound(prices.begin(), prices.end(), sellTime,
                    [](auto const& p, auto const& t){ return p.timestamp < t; });
                if (it2 == prices.end()) break;
                double sellPrice = it2->price;
                double pnl = node.quantity * (sellPrice - node.buyPrice);
                if (pnl > 0)
                {
                    ++profitableTrades;
                    winTrades.push_back(pnl);
                } 
                else 
                {
                    lossTrades.push_back(pnl);
                }
                balance += node.quantity * sellPrice;
                invList.pop_front();
            }
        }
    }

    // Compute and print trade accuracy
    double accuracy = totalTrades
        ? static_cast<double>(profitableTrades) / totalTrades * 100.0
        : 0.0;
    cout << " -> Trades: " << totalTrades
         << ", Profitable: " << profitableTrades
         << ", Accuracy: " << accuracy << "%\n";

    // Track if this simulation was profitable overall
    if (balance > 10000.0) 
    {
        profitableSimulations++;
    }

    return balance;
}

// --- Utility: parseTime ---
// Converts an ISO-8601 timestamp string into a time_point
system_clock::time_point parseTime(const string& iso) 
{
    tm t{};
    sscanf(iso.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
           &t.tm_year, &t.tm_mon, &t.tm_mday,
           &t.tm_hour, &t.tm_min, &t.tm_sec);
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    return system_clock::from_time_t(mktime(&t));
}

// --- Data loading: loadNews ---
// Reads a CSV of news entries, parses fields, and fills newsData vector
void loadNews(const string& file) 
{
    ifstream in(file);
    if (!in.is_open()) {
        cerr << "Failed to open news file!" << endl;
        return;
    }

    string line;
    int count = 0;
    while (getline(in, line)) 
    {
        if (line.empty()) continue;
        stringstream ss(line);
        string name, ticker, sim, sent, time;

        getline(ss, name, ',');
        getline(ss, ticker, ',');
        getline(ss, sim, ',');
        getline(ss, sent, ',');
        getline(ss, time);

        if (ticker.empty() || sim.empty() || sent.empty() || time.empty()) 
        {
            cerr << "Skipping malformed line: " << line << endl;
            continue;
        }

        newsData.push_back({
            ticker,
            stod(sim),
            stod(sent),
            parseTime(time)
        });
        count++;
    }

    cout << "Loaded " << count << " news entries." << endl;
}

// --- Data loading: loadPrices ---
// Reads a CSV of price entries and appends to priceData map
void loadPrices(const string& file) 
{
    ifstream in(file);
    if (!in.is_open()) 
    {
        cerr << "Failed to open prices file!" << endl;
        return;
    }

    string line;
    int count = 0;
    while (getline(in, line)) 
    {
        stringstream ss(line);
        string ticker, priceStr, time;
        getline(ss, ticker, ',');
        getline(ss, priceStr, ',');
        getline(ss, time);

        priceData[ticker].push_back({
            ticker,
            stod(priceStr),
            parseTime(time)
        });
        count++;
    }

    cout << "Loaded " << count << " price entries." << endl;
}

// --- Helper: medianOf ---
// Returns median element from a vector (assumes non-empty call handled by caller)
template<typename T>
T medianOf(vector<T>& v) 
{
    if (v.empty()) return T(0);
    sort(v.begin(), v.end());
    return v[v.size() / 2];
}

// --- Main: parameter sweep ---
int main() {
    loadNews("news.csv");
    loadPrices("prices.csv");

    double bestResult = 0.0;
    double bestExpectedReturn = 0.0;
    double bestExpectedResult = 0.0;
    tuple<double,double,double,double> bestResultParams, bestExpectedParams;
    int totalSimulations = 0;

    // Outer loops: grid search over strategy parameters
    for (double invest = 0.005; invest <= 0.05; invest += 0.005) 
    {
        for (double buyT = 0.8; buyT <= 0.9; buyT += 0.05) 
        {
            for (double sellT = -0.5; sellT <= -0.1; sellT += 0.1) 
            {
                for (double simT = 0.5; simT <= 0.65; simT += 0.05) 
                {
                    investmentMap.clear();

                    int trades = 0, profitable = 0;
                    vector<double> wins, losses;
                    double result = simulate(
                        invest, buyT, sellT, simT,
                        86400, trades, profitable, wins, losses
                    );
                    ++totalSimulations;

                    // Compute statistics on trade outcomes
                    double bestWin  = wins.empty()  ? 0 : *max_element(wins.begin(), wins.end());
                    double bestLoss = losses.empty()? 0 : *min_element(losses.begin(), losses.end());
                    double medWin   = medianOf(wins);
                    double medLoss  = medianOf(losses);
                    double expectedReturn = profitable * medWin + (trades - profitable) * medLoss;

                    cout << "    Best win: " << bestWin
                         << ", Median win: " << medWin << "\n"
                         << "    Worst loss: " << bestLoss
                         << ", Median loss: " << medLoss << endl;
                    cout << "    Expected return: " << expectedReturn << endl;
                    cout << "Result " << result << " for: "
                         << invest << ' ' << buyT << ' ' << sellT << ' ' << simT << endl << endl;

                    // Track best overall result and best expected-return strategy
                    if (result > bestResult) {
                        bestResult = result;
                        bestResultParams = {invest, buyT, sellT, simT};
                    }
                    if (expectedReturn > bestExpectedReturn) {
                        bestExpectedReturn = expectedReturn;
                        bestExpectedParams = {invest, buyT, sellT, simT};
                        bestExpectedResult = result;
                    }
                }
            }
        }
    }

    // Print summary of top strategies
    auto [i1, b1, s1, sim1] = bestResultParams;
    cout << "\nBest result strategy: invest " << i1 * 100 << "%, buy>= " << b1
         << ", sell<= " << s1 << ", similarity>= " << sim1 << "\n"
         << "Final balance: $" << bestResult << endl;

    auto [i2, b2, s2, sim2] = bestExpectedParams;
    cout << "\nBest expected strategy: invest " << i2 * 100 << "%, buy>= " << b2
         << ", sell<= " << s2 << ", similarity>= " << sim2 << "\n"
         << "Final balance: $" << bestExpectedResult << "\n"
         << "Expected return: " << bestExpectedReturn << endl << endl;

    cout << "Total simulations: " << totalSimulations
         << "\nProfitable simulations: " << profitableSimulations << endl << endl;

    return 0;
}
