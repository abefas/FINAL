import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MultipleLocator

# Load the data from CSV
data = pd.read_csv('H-AACONC_localOpt_comparison_to_Standard_gaps.csv')

# Create a bar width and positions
bar_width = 0.2
index = np.arange(len(data['Instance']))

# Define color palette
colors = {
    'bar1': '#1f77b4',  # Navy Blue
    'bar2': '#17becf',    # Sky Blue
    'bar3': '#9467bd',  # Purple
    'bar4': '#ff7f0e'   # Orange
}

# Plotting
fig, ax = plt.subplots(figsize=(14, 8))

# Bars for each algorithm with chosen colors and a thin border
bars1 = ax.bar(index - 1.5*bar_width, data['+Swap Best'], bar_width, label='+Swap Best', color=colors['bar1'], edgecolor='black', linewidth=0.5)
bars2 = ax.bar(index - 0.5*bar_width, data['No Local Opt Best'], bar_width, label='No Local Opt Best', color=colors['bar2'], edgecolor='black', linewidth=0.5)
#bars3 = ax.bar(index + 0.5*bar_width, data['No Local Opt Best'], bar_width, label='No Local Opt Best', color=colors['bar3'], edgecolor='black', linewidth=0.5)
#bars4 = ax.bar(index + 1.5*bar_width, data['heuristic_kmeans'], bar_width, label='Heuristic k-means', color=colors['bar4'], edgecolor='black', linewidth=0.5)

# Add labels, title, and legend
ax.set_title('H-AACONC Local Opt Comparison')
ax.set_xlabel('Instance')
ax.set_ylabel('Gap (%)')

ax.set_xticks(index)
ax.set_xticklabels(data['Instance'], rotation=45, ha='right')
ax.legend()

# Set y-axis interval to every 5%
ax.yaxis.set_major_locator(MultipleLocator(5))

# Add grid for better readability with dashed lines
ax.yaxis.grid(True, linestyle='--', linewidth=0.5, color='grey')

# Adjust layout
plt.tight_layout()

# Show plot
plt.show()
