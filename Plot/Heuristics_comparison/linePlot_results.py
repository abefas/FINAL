import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Read the CSV data into a DataFrame
data = pd.read_csv('heuristics_comp_kmeans.csv')

# Set the style and color palette
sns.set(style="whitegrid")
palette = sns.color_palette("husl", 4)  # Using a Husl color palette

# Create a line plot with enhanced aesthetics
plt.figure(figsize=(14, 8))

# Plot each algorithm's results
for idx, column in enumerate(data.columns[1:]):
    sns.lineplot(data=data, x='Instance', y=column, marker='o', label=column, color=palette[idx])

# Customize the plot
plt.xlabel('Instance', fontsize=14)
plt.ylabel('Result', fontsize=14)
plt.title('Results of Algorithms using k-means Clustering', fontsize=16, weight='bold')
plt.legend(title='Algorithm', fontsize=12)
plt.xticks(rotation=45, fontsize=12)

# Set y-axis interval appropriately
y_min = int(data.iloc[:, 1:].min().min() // 100 * 100)  # Find minimum y value rounded down to nearest multiple of 100
y_max = int(data.iloc[:, 1:].max().max() // 100 * 100 + 100)  # Find maximum y value rounded up to nearest multiple of 100
plt.yticks(range(y_min, y_max + 100, 100), fontsize=12)

# Add horizontal grid lines
plt.grid(axis='y', linestyle='--', linewidth=0.7)

plt.tight_layout()

# Show the plot
plt.show()
