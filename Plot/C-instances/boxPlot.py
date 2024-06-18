import pandas as pd
import matplotlib.pyplot as plt

# Assuming the CSV file is named 'gaps.csv' and is in the same directory as the script
file_path = 'best_results_all.csv'

# Read the CSV file into a DataFrame
data = pd.read_csv(file_path)

# Display the first few rows of the DataFrame to ensure it's loaded correctly
print(data.head())

# Create a box plot for the three columns: AACOAVG, AACOWOR, and HEURISTIC
plt.figure(figsize=(12, 6))
data.boxplot(column=['AACONC+', 'heuristic_prox', 'heuristic_kmeans'])
plt.title('Comparison of best results')
plt.ylabel('Result')
plt.xlabel('Algorithm')
plt.grid(True)

# Show the plot
plt.show()
