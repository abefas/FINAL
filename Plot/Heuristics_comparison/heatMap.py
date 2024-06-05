import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Read the CSV data into a DataFrame
data = pd.read_csv('heuristics_comp_gaps_to_bks_proximity.csv', index_col='Instance')

# Create a heatmap
plt.figure(figsize=(14, 8))
sns.heatmap(data.T, annot=True, cmap='coolwarm', linewidths=.5)

# Customize the plot
plt.xlabel('Instance', fontsize=14)
plt.ylabel('Algorithm', fontsize=14)
plt.title('Heatmap of Gaps Between Algorithm Results and Optimal Solution', fontsize=16, weight='bold')
plt.xticks(rotation=45, fontsize=12)
plt.yticks(fontsize=12)
plt.tight_layout()

# Show the plot
plt.show()
