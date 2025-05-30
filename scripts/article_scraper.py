# Import the Article class from the newspaper3k library
from newspaper import Article

# Import sys to access command-line arguments and standard error output
import sys

# Define a function to fetch and print the full article text from a given URL
def fetch_article(url):
    try:
        # Create an Article object using the provided URL
        article = Article(url)

        # Download the article's HTML content
        article.download()

        # Parse the downloaded HTML to extract the article's text
        article.parse()

        # Print the full article text to standard output
        print(article.text)

    except Exception as e:
        # If any error occurs (e.g., network issue, bad URL, etc.):
        # - Print the error message to standard error
        # - Exit with non-zero status so C++ side knows the script failed
        print(f"[ERROR] {e}", file=sys.stderr)
        sys.exit(-1)

# Run this block only if the script is being executed directly (not imported)
if __name__ == "__main__":
    # Check if the user passed a URL argument to the script
    if len(sys.argv) < 2:
        # If not, print usage instructions and exit with error code 1
        print("Usage: python article_scraper.py <url>", file=sys.stderr)
        sys.exit(-2)

    # Pass the provided URL to the scraping function
    fetch_article(sys.argv[1])
