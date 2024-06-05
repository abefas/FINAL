import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Read the CSV data into a DataFrame
data = pd.read_csv('heuristics_comp_gaps_to_bks_proximity.csv')

# Melt the DataFrame to make it suitable for seaborn barplot
melted_data = pd.melt(data, id_vars='Instance', var_name='Algorithm', value_name='Gap')

# Create a bar plot
plt.figure(figsize=(14, 8))
sns.barplot(x='Instance', y='Gap', hue='Algorithm', data=melted_data)

# Customize the plot
plt.xlabel('Instance', fontsize=14)
plt.ylabel('Gap (%)', fontsize=14)
plt.title('Gaps Between Algorithm Results using proximity clustering and AACONC+ Best', fontsize=16, weight='bold')
plt.legend(title='Algorithm', fontsize=12)
plt.xticks(rotation=45, fontsize=12)

# Set y-axis interval of 5 including negatives
y_min = int(melted_data['Gap'].min() // 5 * 5)  # Find minimum y value rounded down to nearest multiple of 5
y_max = int(melted_data['Gap'].max() // 5 * 5 + 5)  # Find maximum y value rounded up to nearest multiple of 5
plt.yticks(range(y_min, y_max + 5, 5), fontsize=12)

# Add horizontal grid lines
plt.grid(axis='y', linestyle='--', linewidth=0.7)

plt.tight_layout()

# Show the plot
plt.show()
