import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

# Load data
nodes_df = pd.read_csv('p01_nodes.csv')
clusters_df = pd.read_csv('prox-01-clusters.csv')
ivt1_df = pd.read_csv('prox01-15-15-15_ND1-ivt_0.csv')  # Truck paths
ivt2_df = pd.read_csv('prox01-15-15-15_ND1-ivt_1.csv')  # Motorbike paths
ivt3_df = pd.read_csv('prox01-15-15-15_ND1-ivt_2.csv')  # Drone paths

# Create an undirected graph
G = nx.Graph()

# Add all nodes to the graph with positions
for _, row in nodes_df.iterrows():
    G.add_node(row['NodeID'], pos=(row['Xposition'], row['Yposition']))

# Merge clusters_df with nodes_df to match NodeIDs
clusters_df = clusters_df.merge(nodes_df, left_on=['X', 'Y'], right_on=['Xposition', 'Yposition'], how='left')

# Update nodes with cluster and type information
for _, row in clusters_df.iterrows():
    node_id = row['NodeID']
    if pd.notna(node_id):
        G.nodes[node_id]['cluster'] = row['Cluster']
        G.nodes[node_id]['type'] = row['Type']

# Add edges for trucks, motorbikes, and drones
for _, row in ivt1_df.iterrows():
    G.add_edge(row['StartNode'], row['EndNode'], vehicle='truck')
for _, row in ivt2_df.iterrows():
    G.add_edge(row['StartNode'], row['EndNode'], vehicle='motorbike')
for _, row in ivt3_df.iterrows():
    G.add_edge(row['StartNode'], row['EndNode'], vehicle='drone')

# Define colors for clusters and vehicles
cluster_colors = ['#FFA07A', 'green', 'blue', 'purple', 'magenta', 'cyan', 'lightblue']
vehicle_colors = {'truck': '#FFA07A', 'motorbike': 'green', 'drone': 'gray'}

# Get positions and cluster labels
pos = nx.get_node_attributes(G, 'pos')
clusters = nx.get_node_attributes(G, 'cluster')

# Plot settings
plt.figure(figsize=(12, 12))

# Plot nodes with colors based on clusters
for cluster in set(clusters.values()):
    nodes = [node for node in G.nodes() if G.nodes[node].get('cluster') == cluster]
    nx.draw_networkx_nodes(G, pos, nodelist=nodes, node_color=cluster_colors[cluster % len(cluster_colors)],
                           node_size=6, edgecolors=cluster_colors[cluster % len(cluster_colors)])

# Plot depot nodes separately with a distinct color and larger size
depot_nodes = [node for node in G.nodes() if G.nodes[node].get('type') == 'Depot']
nx.draw_networkx_nodes(G, pos, nodelist=depot_nodes, node_color='#8B0000', node_size=8, label='Depot', edgecolors='black')

# Plot edges with colors based on vehicles
for vehicle, color in vehicle_colors.items():
    edges = [(u, v) for u, v, d in G.edges(data=True) if d['vehicle'] == vehicle]
    style = 'dashed' if vehicle == 'drone' else 'solid'
    nx.draw_networkx_edges(G, pos, edgelist=edges, edge_color=color, width=1, style=style, alpha=0.7)

# Add node labels inside nodes
# labels = {node: str(node) for node in G.nodes()}
# nx.draw_networkx_labels(G, pos, labels=labels, font_color='gray', font_size=10, font_weight='bold')

# Add legend and show plot
plt.legend(fontsize='medium', loc='upper left')
plt.xlabel("X Coordinate", fontsize=14)
plt.ylabel("Y Coordinate", fontsize=14)
plt.grid(True, linestyle='--', linewidth=0.5)
plt.tight_layout()
plt.show()
