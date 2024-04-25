import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('TkAgg')

df = pd.read_csv('p01-H.csv')
df_connections_T = pd.read_csv('p01_1_ivt_1.csv')
df_connections_M = pd.read_csv('p01_1_ivt_2.csv')
df_connections_D = pd.read_csv('p01_1_ivt_3.csv')

x = df['Xposition']
y = df['Yposition']
node_names = df['NodeID']

node_colors = ['black'] * len(df)  # Default color for all nodes
node_colors[-4:] = ['red'] * 4  # Last four nodes are red

node_sizes = [200] * len(df)  # Default size for all nodes
node_sizes[-4:] = [300] * 4  # Last four nodes are bigger

dark_gray = '#333333'
light_gray = '#DDDDDD'

plt.figure(figsize=(20, 20), facecolor=light_gray)
plt.gca().set_facecolor(light_gray)

# Add legend information using scatter plots
plt.scatter(x[:-4], y[:-4], s=150, c=node_colors[:-4],
            label='Customers', marker='o')
plt.scatter(x[-4:], y[-4:], s=150, c=node_colors[-4:],
            label='Depots', marker='o')
plt.scatter([], [], color='green', label='Truck routes', marker='_')
plt.scatter([], [], color='orange', label='Motorbike routes', marker='_')
plt.scatter([], [], color='blue', label='Drone routes', marker='_')

for i, name in enumerate(node_names):
    plt.annotate(name, (x[i], y[i]),
                 textcoords="offset points",
                 xytext=(0, 0),
                 ha='center',
                 va='center',
                 color='white',
                 fontsize=6)

for _, connection in df_connections_T.iterrows():
    node1 = connection['StartNode']
    node2 = connection['EndNode']
    plt.plot([x[node1-1], x[node2-1]], [y[node1-1], y[node2-1]],
             color='green',
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
             color='blue',
             linestyle='dotted',
             linewidth=1)


plt.xlabel('X-axis', color='black')
plt.ylabel('Y-axis', color='black')
plt.title('p01-H WORST', color='black')
plt.grid(True, color=dark_gray)
plt.xticks(color=dark_gray)
plt.yticks(color=dark_gray)
plt.legend()
plt.show()
