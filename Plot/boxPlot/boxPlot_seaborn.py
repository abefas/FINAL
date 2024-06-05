import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Read the CSV data into a DataFrame
data = pd.read_csv('best_results_all.csv')

# Melt the DataFrame to make it suitable for seaborn boxplot
melted_data = pd.melt(data, id_vars='Instance', var_name='Algorithm', value_name='Gap')

# Create a box plot
plt.figure(figsize=(14, 8))
sns.boxplot(x='Algorithm', y='Gap', data=melted_data)

# Customize the plot
plt.xlabel('Technique', fontsize=14)
plt.ylabel('Result', fontsize=14)
plt.title('Distribution of results using proximity clustering', fontsize=16, weight='bold')
plt.xticks(fontsize=12)

# Set y-axis interval of 5 including negatives
y_min = int(melted_data['Gap'].min() // 5 * 5)  # Find minimum y value rounded down to nearest multiple of 5
y_max = int(melted_data['Gap'].max() // 5 * 5 + 5)  # Find maximum y value rounded up to nearest multiple of 5
plt.yticks(range(y_min, y_max + 5, 5), fontsize=12)

# Add horizontal grid lines
plt.grid(axis='y', linestyle='--', linewidth=0.7)

plt.tight_layout()

# Show the plot
plt.show()
