import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv('p01_nodes.csv')
df_connections_T = pd.read_csv('prox01-v1-ivt_1.csv')
df_connections_M = pd.read_csv('prox01-v1-ivt_2.csv')
df_connections_D = pd.read_csv('prox01-v1-ivt_3.csv')

# Extract node positions and names
x = df['Xposition']
y = df['Yposition']
node_names = df['NodeID']

# Define node colors and sizes
node_colors = ['black'] * len(df)
node_colors[-4:] = ['red'] * 4  # Last four nodes are depots

node_sizes = [200] * len(df)
node_sizes[-4:] = [300] * 4  # Last four nodes are larger

# Define plot colors
background_color = '#FFFFFF'
grid_color = '#CCCCCC'
dark_gray = '#333333'

# Initialize the plot
plt.figure(figsize=(12, 6), facecolor=background_color)
plt.gca().set_facecolor(background_color)

# Add legend information using scatter plots
plt.scatter(x[:-4], y[:-4], s=150, c=node_colors[:-4], label='Customers', marker='o')
plt.scatter(x[-4:], y[-4:], s=150, c=node_colors[-4:], label='Depots', marker='o')
plt.scatter([], [], color='green', label='Truck routes', marker='_')
plt.scatter([], [], color='orange', label='Motorbike routes', marker='_')
plt.scatter([], [], color='blue', label='Drone routes', marker='_')

# Plot truck routes
for _, connection in df_connections_T.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    plt.plot([x[node1 - 1], x[node2 - 1]], [y[node1 - 1], y[node2 - 1]], color='green', linestyle='-', linewidth=3, alpha=0.6)

# Plot motorbike routes
for _, connection in df_connections_M.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    plt.plot([x[node1 - 1], x[node2 - 1]], [y[node1 - 1], y[node2 - 1]], color='orange', linestyle='-', linewidth=3, alpha=0.6)

# Plot drone routes
for _, connection in df_connections_D.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    plt.plot([x[node1 - 1], x[node2 - 1]], [y[node1 - 1], y[node2 - 1]], color='blue', linestyle='dotted', linewidth=3, alpha=0.6)

# Customize the plot
plt.xlabel('X-axis', fontsize=16, color=dark_gray)
plt.ylabel('Y-axis', fontsize=16, color=dark_gray)
plt.title('p01 Solution (original)', fontsize=16, fontweight='bold', color=dark_gray)
plt.grid(False)
plt.xticks(fontsize=12, color=dark_gray)
plt.yticks(fontsize=12, color=dark_gray)
plt.legend(fontsize=12, loc='upper left')
plt.tight_layout()

# Save and show the plot
plt.show()
