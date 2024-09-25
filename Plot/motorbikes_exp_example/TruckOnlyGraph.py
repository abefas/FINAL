import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV file containing truck paths
truck_only = pd.read_csv("p07-15-15-15_run-5_TRUCK.csv")  # Truck-only routes

# Load the nodes data containing coordinates
nodes = pd.read_csv("p07_nodes.csv")

# Create a dictionary to map NodeID to their coordinates
node_coords = {row['NodeID']: (row['Xposition'], row['Yposition']) for _, row in nodes.iterrows()}

# Define depot nodes and customer nodes
depot_nodes = {101, 102, 103, 104}
customer_nodes = set(range(1, 101))

# Function to plot paths with specific styling for nodes and lines
def plot_paths(df, line_color, marker_color, linestyle, label):
    for i, row in df.iterrows():
        start_node = row['StartNode']
        end_node = row['EndNode']
        start_coord = node_coords.get(start_node)
        end_coord = node_coords.get(end_node)

        if start_coord and end_coord:
            # Plot regular lines for truck connections
            plt.plot(
                [start_coord[0], end_coord[0]], [start_coord[1], end_coord[1]],
                linestyle=linestyle, color=line_color, marker='o',
                markerfacecolor='none', markeredgecolor=marker_color, markersize=5,
                linewidth=1, label=label if i == 0 else ""
            )

            # Plot start and end nodes with distinct styles if they are depots
            if start_node in depot_nodes:
                plt.plot(start_coord[0], start_coord[1], 'o', markersize=8, markerfacecolor='#8B0000', markeredgecolor='#8B0000')
            else:
                plt.plot(start_coord[0], start_coord[1], 'o', markersize=1, markerfacecolor='none', markeredgecolor=marker_color, markeredgewidth=0.5)

            if end_node in depot_nodes:
                plt.plot(end_coord[0], end_coord[1], 'o', markersize=8, markerfacecolor='#8B0000', markeredgecolor='#8B0000')
            else:
                plt.plot(end_coord[0], end_coord[1], 'o', markersize=1, markerfacecolor='none', markeredgecolor=marker_color, markeredgewidth=0.5)

            label = ""  # Only label the first line to avoid duplicate entries in the legend

# Set the figure size in inches (adjust as needed for display or save resolution)
plt.figure(figsize=(12, 12))

# Plot truck paths with pastel orange lines and markers
plot_paths(truck_only, line_color='#FFA07A', marker_color='#FFA07A', linestyle='-', label='Truck routes')

# Set fixed limits for the axes
plt.xlim(0, 3500)  # Set x-axis from 0 to 3500 units
plt.ylim(0, 4000)  # Set y-axis from 0 to 4000 units

# Add legend with adjusted style
plt.legend(loc='upper left', fontsize=10, frameon=False, fancybox=False, shadow=False)

# Set axis labels and title
plt.xlabel("X Coordinate")
plt.ylabel("Y Coordinate")
plt.title("")

# Display the plot
plt.show()
