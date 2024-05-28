import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.ticker import FuncFormatter

# Read data from CSV file
df = pd.read_csv('gaps_to_BKS.csv', na_values='-')  # Replace 'your_data.csv' with the path to your CSV file

# Convert columns to numeric data types
df['AACOAVG'] = pd.to_numeric(df['AACOAVG'])
df['AACOWOR'] = pd.to_numeric(df['AACOWOR'])
df['HEURISTIC'] = pd.to_numeric(df['HEURISTIC'])

# Get number of instances after dropping empty results
num_instances = len(df)

# Filter instances to skip empty ones
non_empty_instances = [f'p{str(i).zfill(2)}-C' for i in range(1, num_instances + 1)]

# Extract results for each instance
results_1 = df['AACOAVG']
results_2 = df['AACOWOR']
results_3 = df['HEURISTIC']

# Combine all results to find the maximum value
max_value = max(results_1.max(), results_2.max(), results_3.max())

# Data for plotting
instances = np.arange(num_instances)
width = 1  # Width of each bar
space_between_instances = 1  # Initial space between each instance's bars
space_between_groups = 1  # Initial space between every three bars

# Define custom formatting function to add '%' sign
def percentage(x, pos):
    return f'{x}%'

# Plot
fig, ax = plt.subplots()
for i, (instance, result_1, result_2, result_3) in enumerate(zip(non_empty_instances, results_1, results_2, results_3)):
    instance_position = i * (3 * width + space_between_instances + space_between_groups)
    if np.isnan(result_1) or np.isnan(result_2) or np.isnan(result_3):
        ax.bar(instance_position, 0, width, color='gray')  # Plot an empty bar for empty instances
    else:
        ax.bar(instance_position - width, result_1, width, color='#00008B')
        ax.bar(instance_position, result_2, width, color='#6495ED')
        ax.bar(instance_position + width, result_3, width, color='lightgreen')

# Add labels and title
ax.set_xlabel('MD-mfcmTSP Instances')
ax.set_ylabel('Gap (%)')
ax.set_title('Comparison to the best solutions found by AACONC+')

ax.set_xticks(np.arange(num_instances) * (3 * width + space_between_instances + space_between_groups))
ax.set_xticklabels(non_empty_instances, rotation=45, ha='right')

ax.legend(['AACONC+ Average', 'AACONC+ Worst', 'Heuristic'])

# Set y-axis limits and ticks
ax.set_ylim(-6, max_value)
ax.yaxis.set_major_locator(plt.MultipleLocator(2))

# Apply custom formatting function to y-axis ticks
ax.yaxis.set_major_formatter(FuncFormatter(percentage))

# Increase font size
plt.rc('font', size=14)

# Show plot
plt.tight_layout()  # Adjust layout to prevent labels from being cut off
plt.show()

