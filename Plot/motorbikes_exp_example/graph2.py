import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV files containing paths for truck, motorbike, and drone
ivt_0 = pd.read_csv("prox07_init_ivt_0.csv")  # Truck paths
ivt_1 = pd.read_csv("prox07_init_ivt_1.csv")  # Motorbike paths
ivt_2 = pd.read_csv("prox07_init_ivt_2.csv")  # Drone paths

# Load the nodes data containing coordinates
nodes = pd.read_csv("p07_nodes.csv")

# Create a dictionary to map NodeID to their coordinates
node_coords = {row['NodeID']: (row['Xposition'], row['Yposition']) for _, row in nodes.iterrows()}

# Define depot nodes and customer nodes
depot_nodes = {101, 102, 103, 104}
customer_nodes = set(range(1, 101))

# Function to plot paths with specific styling for nodes and lines
def plot_paths(df, line_color, marker_color, linestyle, label, special_handling=False):
    for i, row in df.iterrows():
        start_node = row['StartNode']
        end_node = row['EndNode']
        start_coord = node_coords.get(start_node)
        end_coord = node_coords.get(end_node)

        if start_coord and end_coord:
            # Check if the line is from a depot to a customer for special handling
            if special_handling and ((start_node in depot_nodes and end_node in customer_nodes) or 
                                     (end_node in depot_nodes and start_node in customer_nodes)):
                # Plot dashed gray lines for depot to customer connections
                plt.plot(
                    [start_coord[0], end_coord[0]], [start_coord[1], end_coord[1]],
                    linestyle='--', color='gray', marker='o', markerfacecolor='none',
                    markeredgecolor=marker_color, markersize=5, linewidth=1,
                    label=label if i == 0 else ""
                )
            else:
                # Plot regular lines for other connections
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
plot_paths(ivt_0, line_color='#FFA07A', marker_color='#FFA07A', linestyle='-', label='Truck routes - $M_T=974.80$')

# Plot motorbike paths with special handling for depot-to-customer connections
plot_paths(ivt_1, line_color='green', marker_color='green', linestyle='-', label='Motorbike routes - $M_M=218.13$', special_handling=True)

# Plot drone paths with dashed gray lines and blue nodes
plot_paths(ivt_2, line_color='gray', marker_color='blue', linestyle='--', label='UAV routes - $M_D=0.00$')

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
