import yfinance as yf
import pandas as pd
import os

import os
import yfinance as yf
import pandas as pd

def fetch_options_to_csv(symbol: str, expiry: str = None):
    # Path to save CSVs in ../data relative to this script
    base_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(base_dir, "..", "data")

    # Make sure data directory exists
    os.makedirs(data_dir, exist_ok=True)

    # Download ticker object
    ticker = yf.Ticker(symbol)

    # Get spot price (S0)
    try:
        S0 = ticker.fast_info["lastPrice"]
    except Exception:
        # fallback with last close
        S0 = ticker.history(period="1d")["Close"].iloc[-1]

    print(f"Spot price S0 for {symbol}: {S0}")

    # Get available expiration dates
    expirations = ticker.options
    if not expirations:
        print(f"No options found for {symbol}")
        return

    # Take the first available expiration if none provided
    if expiry is None:
        expiry = expirations[0]
        print(f"Chosen expiration: {expiry}")
    else:
        if expiry not in expirations:
            print(f"Expiration {expiry} not found. Available expirations: {expirations}")
            raise ValueError("Invalid expiration date")

    # Get the option chain (calls and puts)
    opt_chain = ticker.option_chain(expiry)
    calls = opt_chain.calls
    puts = opt_chain.puts

    # Ajouter S0 comme colonne pour rappel
    calls.insert(1, "S0", S0)
    puts.insert(1, "S0", S0)

    # Build file paths
    calls_path = os.path.join(data_dir, "calls.csv")
    puts_path = os.path.join(data_dir, "puts.csv")

    # Save to CSV
    calls.to_csv(calls_path, index=False)
    puts.to_csv(puts_path, index=False)

    print(f"✅ Files generated: {calls_path} and {puts_path}")


if __name__ == "__main__":
    # Ask user for ticker symbol
    symbol = input("Enter ticker symbol (e.g., AAPL, TSLA, MSFT): ").strip().upper()
    expiry = input("Enter expiration date (YYYY-MM-DD) or press Enter to use the nearest expiration: ").strip()
    expiry = expiry if expiry else None
    fetch_options_to_csv(symbol, expiry)

