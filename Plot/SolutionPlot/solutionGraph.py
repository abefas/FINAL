import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

# Load data
nodes_df = pd.read_csv('p11_nodes.csv')
clusters_df = pd.read_csv('kmeans-11-clusters.csv')
ivt1_df = pd.read_csv('kmeans-11-ivt_1.csv')
ivt2_df = pd.read_csv('kmeans-11-ivt_2.csv')
ivt3_df = pd.read_csv('kmeans-11-ivt_3.csv')

# Create an undirected graph
G = nx.Graph()

# Add all nodes to the graph from p01_nodes.csv
for _, row in nodes_df.iterrows():
    G.add_node(row['NodeID'], pos=(row['Xposition'], row['Yposition']))

# Merge clusters_df with nodes_df to get the NodeID for each cluster entry
clusters_df = clusters_df.merge(nodes_df, left_on=['X', 'Y'], right_on=['Xposition', 'Yposition'], how='left')

# Update nodes with cluster information from kmeans-01-clusters.csv
for _, row in clusters_df.iterrows():
    node_id = row['NodeID']
    if pd.notna(node_id):
        if row['Type'] == 'Point':
            G.nodes[node_id]['cluster'] = row['Cluster']
            G.nodes[node_id]['type'] = row['Type']
        elif row['Type'] == 'Depot':
            G.nodes[node_id]['type'] = row['Type']

# Add edges for trucks
for _, row in ivt1_df.iterrows():
    G.add_edge(row['StartNode'], row['EndNode'], vehicle='truck')

# Add edges for motorbikes
for _, row in ivt2_df.iterrows():
    G.add_edge(row['StartNode'], row['EndNode'], vehicle='motorbike')

# Add edges for drones
for _, row in ivt3_df.iterrows():
    G.add_edge(row['StartNode'], row['EndNode'], vehicle='drone')

# Define colors for clusters and vehicles
cluster_colors = ['red', 'green', 'blue', 'purple', 'magenta', 'cyan', 'lightblue']
vehicle_colors = {'truck': 'orange', 'motorbike': 'cyan', 'drone': 'magenta'}

# Get positions and cluster labels
pos = nx.get_node_attributes(G, 'pos')
clusters = nx.get_node_attributes(G, 'cluster')

# Plot settings
plt.figure(figsize=(12, 10))

# Plot nodes with colors based on clusters
for cluster in set(clusters.values()):
    nodes = [node for node in G.nodes() if G.nodes[node].get('cluster') == cluster]
    nx.draw_networkx_nodes(G, pos, nodelist=nodes, node_color=cluster_colors[cluster],
                           node_size=100, label=f'Cluster {cluster}', alpha=0.8, edgecolors='black')

# Plot depot nodes separately
depot_nodes = [node for node in G.nodes() if G.nodes[node].get('type') == 'Depot']
nx.draw_networkx_nodes(G, pos, nodelist=depot_nodes, node_color='black', node_size=200, label='Depot', edgecolors='black')

# Plot edges with colors based on vehicles
for vehicle, color in vehicle_colors.items():
    edges = [(u, v) for u, v, d in G.edges(data=True) if d['vehicle'] == vehicle]
    if vehicle == 'drone':
        nx.draw_networkx_edges(G, pos, edgelist=edges, edge_color=color, width=2, style='dashed', label=f'{vehicle.capitalize()} route', alpha=0.7)
    else:
        nx.draw_networkx_edges(G, pos, edgelist=edges, edge_color=color, width=2, label=f'{vehicle.capitalize()} route', alpha=0.7)

# Extract centroid positions for plotting separately
centroid_pos = [(row['X'], row['Y']) for _, row in clusters_df.iterrows() if row['Type'] == 'Centroid']
centroid_x, centroid_y = zip(*centroid_pos)
plt.scatter(centroid_x, centroid_y, color='grey', s=100, label='Centroid', alpha=0.8, edgecolors='black')

# Add node labels inside nodes
#labels = {node: str(node) for node in G.nodes()}
#nx.draw_networkx_labels(G, pos, labels=labels, font_color='gray', font_size=10, font_weight='bold')

# Add legend and show plot
plt.legend(fontsize='medium', loc='upper left')
#plt.title("Vehicle Routes and Clusters", fontsize=18, fontweight='bold')
plt.xlabel("X-axis", fontsize=16)
plt.ylabel("Y-axis", fontsize=16)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.tight_layout()
#plt.savefig('vehicle_routes_clusters.png', dpi=600)
plt.show()
