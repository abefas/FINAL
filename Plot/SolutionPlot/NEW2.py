import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib.lines import Line2D


df = pd.read_csv('p01_nodes.csv')
df_connections_T = pd.read_csv('prox01-v1-ivt_1.csv')
df_connections_M = pd.read_csv('prox01-v1-ivt_2.csv')
df_connections_D = pd.read_csv('prox01-v1-ivt_3.csv')
x = df['Xposition']
y = df['Yposition']
node_names = df['NodeID']


# Load data from CSV files
def load_data(csv_files):
    dataframes = []
    for file in csv_files:
        df = pd.read_csv(file)
        dataframes.append(df)
    return dataframes

# Plot the nodes and depots
def plot_clusters(nodes_df, depots_df, color='blue', 
                depot_colors=['red', 'green', 'black', 'purple']):
    # Plot nodes
    plt.scatter(nodes_df['Xposition'],
                nodes_df['Yposition'],
                c=color,
                label='Customers')

                # Annotate each node with its ID
    for i, node_id in enumerate(nodes_df['NodeID']):
        plt.annotate(str(node_id), (nodes_df['Xposition'].iloc[i], nodes_df['Yposition'].iloc[i]),
                     textcoords="offset points", xytext=(0,10), ha='center', fontsize=8)

    # Plot depots with colors corresponding to their clusters
    for i, color in enumerate(depot_colors):
        depot = depots_df.iloc[i]
        plt.scatter(depot['Xposition'],
                    depot['Yposition'],
                    c=color,
                    marker='s',
                    s=100)

# Plot edges between nodes
def plot_edges(G):
    pos = nx.get_node_attributes(G, 'pos')
    edges = G.edges()
    nx.draw_networkx_edges(G, pos, edgelist=edges, alpha=0.5)

# Example usage
if __name__ == "__main__":
    # List of CSV files
    csv_files = ['Heuristic/OnlyFinalLocalOpt/dep_0.csv',
                 'Heuristic/OnlyFinalLocalOpt/dep_1.csv',
                 'Heuristic/OnlyFinalLocalOpt/dep_2.csv',
                 'Heuristic/OnlyFinalLocalOpt/dep_3.csv',
                 'Heuristic/OnlyFinalLocalOpt/depots_p01.csv']

    # Load data
    dataframes = load_data(csv_files)

    # Separate nodes and depots dataframes
    nodes_dfs = dataframes[:-1]  # All dataframes except the last one (depots)
    depots_df = dataframes[-1]   # Last dataframe (depots)

    # List of colors for each cluster
    cluster_colors = ['blue', 'green', 'black', 'purple']
    depot_colors = ['blue', 'green', 'black', 'purple']  # Assign depot colors based on cluster colors
    dark_gray = '#333333'
    light_gray = '#DDDDDD'

    # Plot clusters
    plt.figure(figsize=(20, 20), facecolor=light_gray)
    plt.gca().set_facecolor(light_gray)  # Set background color of the plot to light gray
    for nodes_df, color in zip(nodes_dfs, cluster_colors):
        plot_clusters(nodes_df, depots_df, color=color, depot_colors=depot_colors)

    # Plot edges between nodes
    G = nx.Graph()  # Create an empty graph
    for nodes_df in nodes_dfs:
        for idx, row in nodes_df.iterrows():
            G.add_node(row['NodeID'], pos=(row['Xposition'], row['Yposition']))  # Add nodes to the graph

    plot_edges(G)

    # Plot depot approaches and routes
    for _, connection in df_connections_T.iterrows():
        node1 = connection['StartNode']
        node2 = connection['EndNode']
        plt.plot([x[node1-1], x[node2-1]], [y[node1-1], y[node2-1]],
                 color='red',
                 linestyle='-',
                 linewidth=1)

    for _, connection in df_connections_M.iterrows():
        node1 = connection['StartNode']
        node2 = connection['EndNode']
        plt.plot([x[node1-1], x[node2-1]], [y[node1-1], y[node2-1]],
                 color='orange',
                 linestyle='-',
                 linewidth=1)

    for _, connection in df_connections_D.iterrows():
        node1 = connection['StartNode']
        node2 = connection['EndNode']
        plt.plot([x[node1-1], x[node2-1]], [y[node1-1], y[node2-1]],
                 color='gray',
                 linestyle='--',
                 linewidth=1)

    plt.xlabel('X-axis', color='black')
    plt.ylabel('Y-axis', color='black')
    plt.title('p07-H BEST', color='black')
    plt.grid(False)
    plt.xticks(color=dark_gray)
    plt.yticks(color=dark_gray)
     # Customize legend to show only circle for customers and square for depots
    legend_elements = [ Line2D([0], [0], color='red', label='Truck', linestyle='-'),
                        Line2D([0], [0], color='orange', label='Motorbike', linestyle='-'),
                        Line2D([0], [0], color='gray', label='Drone', linestyle='--')]

    plt.legend(handles=legend_elements, loc='upper left')

    plt.axis('equal')
    plt.show()
