import matplotlib.pyplot as plt
import pandas as pd
import os
import numpy as np

def plot_smile(calls_csv: str, puts_csv: str):
    # Load data
    calls = pd.read_csv(calls_csv)
    puts = pd.read_csv(puts_csv)

    S0 = calls['S0'].iloc[0]  # Assuming S0 is the same for all rows

    # Ensure S0 is in the data
    if 'S0' not in calls.columns or 'S0' not in puts.columns:
        raise ValueError("S0 column not found in the provided CSV files.")

    # Plotting
    plt.figure(figsize=(12, 6))

    # Call options
    plt.subplot(1, 2, 1)
    plt.scatter(calls['strike'], calls['impliedVolatility'], color='blue', label='Calls')
    #plt.axvline(S0, color='red', linestyle='--', label='Spot Price (S0)')
    plt.title('Implied Volatility Smile - Calls')
    plt.xlabel('Strike Price')
    plt.ylabel('Implied Volatility')
    plt.legend()
    plt.grid()

    # Put options
    plt.subplot(1, 2, 2)
    plt.scatter(puts['strike'], puts['impliedVolatility'], color='green', label='Puts')
    #plt.axvline(S0, color='red', linestyle='--', label='Spot Price (S0)')
    plt.title('Implied Volatility Smile - Puts')
    plt.xlabel('Strike Price')
    plt.ylabel('Implied Volatility')
    plt.legend()
    plt.grid()

    plt.tight_layout()
    
    # Save plot
    base_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(base_dir, "..", "data")
    os.makedirs(data_dir, exist_ok=True)
    plot_path = os.path.join(data_dir, "implied_volatility_smile.png")
    plt.savefig(plot_path)
    print(f"Plot saved to {plot_path}")
    
    plt.show()

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(base_dir, "..", "data")
    calls_csv = os.path.join(data_dir, "calls.csv")
    puts_csv = os.path.join(data_dir, "puts.csv")
    
    if not os.path.exists(calls_csv) or not os.path.exists(puts_csv):
        raise FileNotFoundError("Calls or Puts CSV files not found in the data directory.")
    
    plot_smile(calls_csv, puts_csv)