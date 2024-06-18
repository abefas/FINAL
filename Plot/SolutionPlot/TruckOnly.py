import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

# Load data
df = pd.read_csv('p01_nodes.csv')
df_connections_T = pd.read_csv('p01-1_AACONC.csv')

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
    G.add_edge(node1, node2)

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

# Draw edges for truck routes only
edges_T = [(connection['StartNode'], connection['EndNode']) for _, connection in df_connections_T.iterrows()]
nx.draw_networkx_edges(G, positions, edgelist=edges_T, edge_color='green', width=2, style='solid', alpha=0.6)

# Add legend for truck routes only
legend_edges = [
    plt.Line2D([0], [0], color='green', lw=2, linestyle='solid')
]
plt.legend(legend_edges, ['Truck routes'], fontsize=12, loc='upper left')

# Customize the plot
plt.xlabel('X-axis', fontsize=16, color='#333333')
plt.ylabel('Y-axis', fontsize=16, color='#333333')
plt.title('p01 Rt Solution', fontsize=16, fontweight='bold', color='#333333')
plt.xticks(fontsize=12, color='#333333')
plt.yticks(fontsize=12, color='#333333')
plt.grid(False)
plt.tight_layout()

# Save and show the plot
plt.show()
