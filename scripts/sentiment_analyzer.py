# Import the pipeline class from the transformers library
from transformers import pipeline

# Import sys to access command-line arguments and standard error output
import sys

# Define the function to analyze the sentiment of the provided news article
def analyze_sentiment(title, text, use_gpu: bool):
    try:
        # Select device: 0 for GPU, -1 for CPU
        device = 0 if use_gpu else -1
        
        # Load the pre-trained sentiment analysis model (DistilBERT fine-tuned for SST-2)
        sentiment_analysis = pipeline(
            "sentiment-analysis",
            model="distilbert-base-uncased-finetuned-sst-2-english",  # Specify the model
            device=device
        )

        # Combine the title and text of the article for analysis
        article_content = title + " " + text

        # Perform sentiment analysis
        result = sentiment_analysis(article_content)

        # Print the result to standard output
        # Format: [{'label': 'POSITIVE/NEUTRAL/NEGATIVE', 'score': 0-1}]
        print(result)
        
    except Exception as e:
        # If any error occurs (e.g., loading the model, processing the article text, etc.):
        # - Print the error message to standard error
        # - Exit with non-zero status so C++ side knows the script failed
        print(f"[ERROR] {e}", file=sys.stderr)
        sys.exit(-1)

# Run this block only if the script is being executed directly (not imported)
if __name__ == "__main__":
    # Check if the title and text arguments were passed to the script
    if len(sys.argv) < 4:
        # If not, print usage instructions and exit with error code 2
        print("Usage: python sentiment_analysis.py <use_gpu> <title> <text>" + sys.argv[1], file=sys.stderr)
        sys.exit(-2)

    # Extract the title, text, and GPU flag from the command-line arguments
    use_gpu_arg = sys.argv[1].lower()
    if use_gpu_arg in ("true", "1"):
        use_gpu = True
    elif use_gpu_arg in ("false", "0"):
        use_gpu = False
    title = sys.argv[2]
    text = sys.argv[3]

    # Call the analyze_sentiment function with the provided title and text
    analyze_sentiment(title, text, use_gpu)
