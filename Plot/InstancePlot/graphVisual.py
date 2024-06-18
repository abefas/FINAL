import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

# Read the CSV file
file_path = 'p01_nodes.csv'  # Update this path to the correct file path
data = pd.read_csv(file_path)

# Separate customers and depots
customers = data[data['NodeID'] <= 50]
depots = data[data['NodeID'] > 50]

# Function to plot graph
def plot_graph():
    fig, ax = plt.subplots(figsize=(10, 10))
    
    # Plot depots
    ax.scatter(depots['Xposition'], depots['Yposition'], c='blue', marker='s', label='Depots')

    # Plot customers
    ax.scatter(customers['Xposition'], customers['Yposition'], c='orange', label='Customers', alpha=0.7)

    # Create a graph
    G = nx.Graph()

    # Add nodes
    for index, row in data.iterrows():
        G.add_node(row['NodeID'], pos=(row['Xposition'], row['Yposition']))

    # Add edges for nodes 1 to 50 (fully connected)
    customer_ids = customers['NodeID'].tolist()
    for i in range(len(customer_ids)):
        for j in range(i + 1, len(customer_ids)):
            G.add_edge(customer_ids[i], customer_ids[j])

    # Add edges for depots to customers (but not to each other)
    depot_ids = depots['NodeID'].tolist()
    for depot_id in depot_ids:
        for customer_id in customer_ids:
            G.add_edge(depot_id, customer_id)

    # Draw edges
    pos = nx.get_node_attributes(G, 'pos')
    nx.draw_networkx_edges(G, pos, ax=ax, alpha=0.5)

    ax.set_title(f'p01-C')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.legend()

    plt.tight_layout()
    plt.show()

# Plot the graph
plot_graph()
