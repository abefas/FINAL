import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

# Load data
df = pd.read_csv('p07_nodes.csv')
df_connections_T = pd.read_csv('prox07-v1-ivt_1.csv')
df_connections_M = pd.read_csv('prox07-v1-ivt_2.csv')
df_connections_D = pd.read_csv('prox07-v1-ivt_3.csv')

# Extract node positions and names
positions = {row['NodeID']: (row['Xposition'], row['Yposition']) for _, row in df.iterrows()}

# Create a graph instance
G = nx.Graph()

# Add nodes
for node_id in positions:
    G.add_node(node_id)

# Add edges for truck routes
for _, connection in df_connections_T.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    G.add_edge(node1, node2, route_type='Truck')  # Add a route_type attribute

# Add edges for motorbike routes
for _, connection in df_connections_M.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    G.add_edge(node1, node2, route_type='Motorbike')  # Add a route_type attribute

# Add edges for drone routes
for _, connection in df_connections_D.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    G.add_edge(node1, node2, route_type='Drone')  # Add a route_type attribute

# Define node colors and sizes
node_colors = ['black'] * len(positions)
node_colors[-4:] = ['red'] * 4  # Last four nodes are depots

node_sizes = [50] * len(positions)
node_sizes[-4:] = [90] * 4  # Last four nodes are larger

# Initialize the plot
plt.figure(figsize=(12, 6))
plt.gca().set_facecolor('#FFFFFF')

# Draw nodes
nx.draw_networkx_nodes(G, positions, node_color=node_colors, node_size=node_sizes)

# Draw edges with different colors and styles based on route_type
edges_T = [(connection['StartNode'], connection['EndNode']) for _, connection in df_connections_T.iterrows()]
edges_M = [(connection['StartNode'], connection['EndNode']) for _, connection in df_connections_M.iterrows()]
edges_D = [(connection['StartNode'], connection['EndNode']) for _, connection in df_connections_D.iterrows()]

nx.draw_networkx_edges(G, positions, edgelist=edges_T, edge_color='green', width=2, style='solid', alpha=0.6)
nx.draw_networkx_edges(G, positions, edgelist=edges_M, edge_color='orange', width=2, style='solid', alpha=0.6)
nx.draw_networkx_edges(G, positions, edgelist=edges_D, edge_color='blue', width=2, style='dotted', alpha=0.6)

# Adjust label positions to be above the nodes
label_positions = {node: (x, y + 100) for node, (x, y) in positions.items()}  # Adjust the y-coordinate

# Add node labels
nx.draw_networkx_labels(G, label_positions, font_size=10, font_color='black')

# Add legend for edges
legend_edges = [
    plt.Line2D([0], [0], color='green', lw=2, linestyle='solid'),
    plt.Line2D([0], [0], color='orange', lw=2, linestyle='solid'),
    plt.Line2D([0], [0], color='blue', lw=2, linestyle='dotted')
]
plt.legend(legend_edges, ['Truck routes', 'Motorbike routes', 'Drone routes'], fontsize=12, loc='upper left')

# Combine legends
plt.gca().add_artist(legend_edges[0])  # Ensure both legends are displayed

# Customize the plot
plt.xlabel('X-axis', fontsize=16, color='#333333')
plt.ylabel('Y-axis', fontsize=16, color='#333333')
plt.title('p07: Initialization Same Speeds +Infinite Truck capacity', fontsize=16, fontweight='bold', color='#333333')
plt.xticks(fontsize=12, color='#333333')
plt.yticks(fontsize=12, color='#333333')
plt.grid(False)
plt.tight_layout()

# Save and show the plot
plt.show()
