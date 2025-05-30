# Import sys to access command-line arguments and standard error output
import sys

# Import requests to send HTTP requests to the MarketAux API
import requests

# Import json to parse and pretty-print the API's JSON response
import json

# Define a function to fetch and print MarketAux news data using a provided API URL
def fetch_marketaux_data(api_url):
    try:
        # Send a GET request to the provided MarketAux API URL
        response = requests.get(api_url)

        # Raise an exception for HTTP errors (status codes 4xx and 5xx)
        response.raise_for_status()

        # Parse the JSON response into a Python dictionary
        parsed_json = response.json()

        # Pretty-print the JSON data to standard output so C++ can capture it
        print(json.dumps(parsed_json, indent=4))

    except Exception as e:
        # If any error occurs (e.g., network issue, bad URL, etc.):
        # - Print the error message to standard error
        # - Exit with non-zero status so C++ side knows the script failed
        print(f"[ERROR] {e}", file=sys.stderr)
        sys.exit(-1)

# If this script is being run directly (not imported)
if __name__ == "__main__":
    # Check if the full API request URL was provided as a command-line argument
    if len(sys.argv) < 2:
        # If not, print usage instructions and exit with error code 1
        print("Usage: python marketaux_fetcher.py <full_api_url>", file=sys.stderr)
        sys.exit(-2)

    # Extract the API URL from the first command-line argument
    api_url = sys.argv[1]

    # Call the fetch_marketaux_data function with the provided URL
    fetch_marketaux_data(api_url)