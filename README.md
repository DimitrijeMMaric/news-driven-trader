# News-Based Automated Investment Strategy

## Overview
This project implements an automated pipeline that:
1. **Fetches** real-time business news from the MarketAux API.
2. **Parses** and extracts full article text via a Python scraper.
3. **Detects** companies mentioned using OpenAI embeddings and AstraDB.
4. **Analyzes** sentiment with a fine-tuned DistilBERT model.
5. **Executes** a simple buy/sell investment strategy based on sentiment signals.
6. **Simulates** various strategy parameters over historical data for performance evaluation.

## Architecture
```
├── include/               # Public headers (globals, utils, strategy, etc.)
├── src/                   # Core C++ implementation files
│   ├── utils.cpp
│   ├── news_fetcher.cpp
│   ├── news_parser.cpp
│   ├── news_processing.cpp
│   ├── debug.cpp
│   ├── investment_strategy.cpp
│   └── globals.cpp
├── main.cpp               # Entry point for the application
├── scripts/               # Python helpers and third-party integrations
│   ├── marketaux_fetcher.py
│   ├── article_scraper.py
│   ├── company_matcher.py
│   └── sentiment_analyzer.py
├── simulation
│   └── simulate_investment.cpp # Standalone simulation tool
├── settings.cfg           # Configuration file (not versioned)
└── README.md              # Project overview and usage instructions
```

## Prerequisites
- **C++17** compiler (e.g., `g++`, `clang++`)
- **CMake** (optional, if you prefer out-of-source builds)
- **Python 3.8+** with the following packages:
  - `requests`
  - `newspaper3k`
  - `transformers`
  - `openai`
  - `astrapy`

## Configuration
Create a `settings.cfg` in the project root with the following keys:
```ini
lookBackSeconds=3600
delaySeconds=60
debug=true
logToFile=true
useGPU=false
pythonInterpreter=/usr/bin/python3
marketAuxBaseApi=https://api.marketaux.com/v1/news?api_token=YOUR_TOKEN&language=en
astraDBApplicationToken=YOUR_ASTRA_TOKEN
astraDBApiEndpoint=https://YOUR_DB_ID-abc123.apps.astra.datastax.com/api/rest/v2
openAIApi=YOUR_OPENAI_KEY
```

## Building
```bash
# From project root
g++ -std=c++17 -pthread -o main main.cpp src/*.cpp
g++ -std=c++17 -pthread -o simulate simulate_investment.cpp
```

## Usage
```bash
# Run the main news-processing bot
./main

# In parallel, you can open the debug monitor (if enabled)
# Simulate investment strategies over historical CSVs
./simulate
```

## Logging & Debugging
- Logs are printed to console and, if enabled, also saved under `logs/` with timestamped filenames.
- The debug monitor (spawned when `debug=true`) prints queue sizes every 10 seconds.

## Simulation Tool
Use `simulate_investment.cpp` to run parameter sweeps on historical `news.csv` and `prices.csv`. Adjust the CSV paths inside the source as needed.

## Contributing
Feel free to open issues or submit pull requests for enhancements, bug fixes, or additional strategy rules.

## License
SPDX-License-Identifier: MIT
© 2025 Dimitrije Marić
