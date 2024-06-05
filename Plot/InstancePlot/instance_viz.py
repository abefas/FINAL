import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
file_path = 'p01_nodes.csv'  # Update this path to the correct file path
data = pd.read_csv(file_path)

# Separate customers and depots
customers = data[data['NodeID'] <= 50]
depots = data[data['NodeID'] > 50]

# Function to plot graph based on access type
def plot_graph(access_type, vehicle_name):
    accessible = customers[customers['Access'].astype(str).str.contains(access_type)]
    not_accessible = customers[~customers['Access'].astype(str).str.contains(access_type)]
    
    fig, ax = plt.subplots(figsize=(10, 10))
    
    # Plot depots
    ax.scatter(depots['Xposition'], depots['Yposition'], c='blue', marker='s', label='Depots')

    # Plot accessible and not accessible customers
    ax.scatter(accessible['Xposition'], accessible['Yposition'], c='orange', label=f'Accessible by {vehicle_name}', alpha=0.7)
    ax.scatter(not_accessible['Xposition'], not_accessible['Yposition'], c='gray', label=f'Not Accessible by {vehicle_name}', alpha=0.7)

    ax.set_title(f'p01-C')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.legend()

    plt.tight_layout()
    plt.show()

# Set the access type and vehicle name
access_type = '2'  # Change this to '1' for Truck, '2' for Motorbike, '3' for Drone
vehicle_name = 'Motorbike'  # Change this to 'Truck', 'Motorbike', or 'Drone' accordingly

# Plot the graph
plot_graph(access_type, vehicle_name)
