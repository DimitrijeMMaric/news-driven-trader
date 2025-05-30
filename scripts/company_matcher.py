# Import required libraries
import sys
import json
import openai
from astrapy import DataAPIClient

# (Removed hard-coded constants)

# Function to create an embedding vector for a given text
def embed_text(text):
    response = openai.Embedding.create(
        input=text,
        model="text-embedding-3-large"
    )
    return response['data'][0]['embedding']

# Function to fetch top 5 matching companies for given text
def match_companies(collection, title, text):
    try:
        query = title + " " + text
        vector = embed_text(query)

        results = collection.find(
            filter={},
            sort={"$vector": vector},
            projection=["name", "ticker"],
            limit=1,
            include_similarity=True
        )

        # Filter and format matches
        matches = []
        for r in results:
            matches.append({
                "name": r["name"],
                "ticker": r["ticker"],
                "similarity": round(r["$similarity"], 4)
            })

        # Print JSON output
        print(json.dumps(matches))
        
    except Exception as e:
        # On error, print to stderr and exit with failure code
        print(f"[ERROR] {e}", file=sys.stderr)
        sys.exit(-1)

# Run if executed directly
if __name__ == "__main__":
    # Expect: script.py <ASTRA_TOKEN> <ASTRA_ENDPOINT> <OPENAI_KEY> <title> <text>
    if len(sys.argv) < 6:
        print("Usage: python company_matcher.py <ASTRA_TOKEN> <ASTRA_ENDPOINT> <OPENAI_KEY> <title> <text>", file=sys.stderr)
        sys.exit(-2)

    # Pull credentials/endpoints in fixed order
    ASTRA_DB_APPLICATION_TOKEN = sys.argv[1]
    ASTRA_DB_API_ENDPOINT      = sys.argv[2]
    OPENAI_API_KEY             = sys.argv[3]
    articleTitle               = sys.argv[4]
    articleText                = sys.argv[5]

    # Configure API keys
    openai.api_key = OPENAI_API_KEY

    # Initialize AstraDB client and collection
    client = DataAPIClient(ASTRA_DB_APPLICATION_TOKEN)
    db = client.get_database_by_api_endpoint(ASTRA_DB_API_ENDPOINT)
    collection = db.get_collection("companiesdb")

    # Call matching function with provided title and text
    match_companies(collection, articleTitle, articleText)
