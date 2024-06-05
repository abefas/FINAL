import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from math import pi

# Read the CSV data into a DataFrame
data = pd.read_csv('heuristics_comp_gaps_to_bks_proximity.csv')

# Number of variables
categories = list(data.columns[1:])
N = len(categories)

# What will be the angle of each axis in the plot?
angles = [n / float(N) * 2 * pi for n in range(N)]
angles += angles[:1]

# Initialise the spider plot
plt.figure(figsize=(14, 8))
ax = plt.subplot(111, polar=True)

# Function to add a radar chart for each instance
def add_radar_chart(instance):
    values = data.loc[data['Instance'] == instance].values.flatten().tolist()[1:]
    values += values[:1]
    ax.plot(angles, values, linewidth=1, linestyle='solid', label=instance)
    ax.fill(angles, values, alpha=0.1)

# Add each instance to the radar chart
for instance in data['Instance']:
    add_radar_chart(instance)

# Customize the plot
plt.xticks(angles[:-1], categories, fontsize=12)
ax.set_rlabel_position(0)
plt.yticks(fontsize=12)
plt.title('Radar Chart of Gaps Between Algorithm Results and Optimal Solution', size=16, color='black', y=1.1)
plt.legend(loc='upper right', bbox_to_anchor=(1.1, 1.1))

# Show the plot
plt.show()
