import pandas as pd
import matplotlib.pyplot as plt

# Load the data from CSV
data = pd.read_csv('kmeans-01.csv')

# Create a scatter plot
fig, ax = plt.subplots(figsize=(10, 8))

# Plot Points
points = data[data['Type'] == 'Point']
scatter = ax.scatter(points['X'], points['Y'], c=points['Cluster'], cmap='tab10', label='Points', marker='o', edgecolor='k')

# Plot Centroids
centroids = data[data['Type'] == 'Centroid']
ax.scatter(centroids['X'], centroids['Y'], c=centroids['Cluster'], cmap='tab10', label='Centroids', marker='X', s=100, edgecolor='k')

# Plot Depots
depots = data[data['Type'] == 'Depot']
ax.scatter(depots['X'], depots['Y'], c=depots['Cluster'], cmap='tab10', label='Depots', marker='D', s=100, edgecolor='k')

# Add labels and title
ax.set_xlabel('X Coordinate')
ax.set_ylabel('Y Coordinate')
ax.set_title('Cluster Plot')

# Add legend
legend1 = ax.legend(*scatter.legend_elements(), title="Clusters")
ax.add_artist(legend1)
ax.legend(['Points', 'Centroids', 'Depots'], loc='upper right')

# Show the plot
plt.show()

