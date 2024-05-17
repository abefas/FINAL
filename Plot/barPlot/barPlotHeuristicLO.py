import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.ticker import FuncFormatter

# Read data from CSV file
df = pd.read_csv('Heuristic_localOpt_comparison.csv', na_values='-')  # Replace 'your_data.csv' with the path to your CSV file

# Convert columns to numeric data types
df['Gap_Full'] = pd.to_numeric(df['Gap_Full'])
df['Gap_NoLO'] = pd.to_numeric(df['Gap_NoLO'])
df['Gap_NoSwap'] = pd.to_numeric(df['Gap_NoSwap'])
df['Gap_NoFull'] = pd.to_numeric(df['Gap_NoFull'])

# Get number of instances after dropping empty results
num_instances = len(df)

# Filter instances to skip empty ones
non_empty_instances = [f'p{str(i).zfill(2)}-C' for i in range(1, num_instances + 1)]

# Extract results for each instance
results_1 = df['Gap_Full']
results_2 = df['Gap_NoLO']
results_3 = df['Gap_NoSwap']
results_4 = df['Gap_NoFull']

# Combine all results to find the maximum value
max_value = max(results_1.max(), results_2.max(), results_3.max(), results_4.max())

# Data for plotting
instances = np.arange(num_instances)
width = 1  # Width of each bar
space_between_instances = 1  # Initial space between each instance's bars
space_between_groups = 1  # Initial space between every four bars

# Define custom formatting function to add '%' sign
def percentage(x, pos):
    return f'{x}%'

# Plot
fig, ax = plt.subplots()
for i, (instance, result_1, result_2, result_3, result_4) in enumerate(zip(non_empty_instances, results_1, results_2, results_3, results_4)):
    instance_position = i * (4 * width + space_between_instances + space_between_groups)
    if np.isnan(result_1) or np.isnan(result_2) or np.isnan(result_3) or np.isnan(result_4):
        ax.bar(instance_position, 0, width, color='gray')  # Plot an empty bar for empty instances
    else:
        ax.bar(instance_position - 1.5 * width, result_1, width, color='#1f77b4')
        ax.bar(instance_position - 0.5 * width, result_2, width, color='#ff7f0e')
        ax.bar(instance_position + 0.5 * width, result_3, width, color='#aec7e8')
        ax.bar(instance_position + 1.5 * width, result_4, width, color='#ffbb78')

# Add labels and title
ax.set_xlabel('MD-mfcmTSP Instances')
ax.set_ylabel('Gap (%)')
ax.set_title('Local optimization impact on the Heuristic')

ax.set_xticks(np.arange(num_instances) * (4 * width + space_between_instances + space_between_groups))
ax.set_xticklabels(non_empty_instances, rotation=45, ha='right')

ax.legend(['Full', 'No local opt', 'Only final', 'No final'])

# Set y-axis limits and ticks
ax.set_ylim(0, max_value)
ax.yaxis.set_major_locator(plt.MultipleLocator(5))

# Apply custom formatting function to y-axis ticks
ax.yaxis.set_major_formatter(FuncFormatter(percentage))

# Show plot
plt.tight_layout()  # Adjust layout to prevent labels from being cut off
plt.show()

