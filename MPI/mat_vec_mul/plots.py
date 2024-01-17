import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
import matplotlib as mpl
mpl.style.use('seaborn')

columns = ['size', 'nodes', 'tasks', 'time w/ comms', 'time', 'error']
offsets = [13, 15, 15]

rows = []

transform = lambda i, x : float(x) if i > 2 else int(x)

with open('./output/out.txt', 'r') as file:
    while True:
        lines = [file.readline() for _ in range(3)]

        if not any(lines):
            break
        
        rows.append([
            transform(*x) for x in enumerate(
                x 
                for xs in [line[offset:-1].split(',') for offset, line in zip(offsets, lines)]
                for x in xs
            )
        ])


df = pd.DataFrame(rows, columns=columns)

subset = ['size', 'nodes', 'tasks']

df_mean = df
df_mean['time w/ comms'] = df_mean.groupby(subset)['time w/ comms'].transform('mean')
df_mean['time'] = df_mean.groupby(subset)['time'].transform('mean')
df_mean['error'] = df_mean.groupby(subset)['error'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)

# df_mean.to_csv('./output/mean_results.csv', index=False, sep='\t')

print(df_mean)

fig, ax = plt.subplots()
ax.grid(visible=True)
# ax.set_title(f'Size = {size}')
ax.set_xlabel('world size')
ax.set_ylabel('time (sec)')
ax.set_xticks([1, 4, 16, 64])

my_cmap = [
    plt.get_cmap(cmap)
    for cmap in ['Reds', 'Blues', 'summer', 'autumn']
]

rand = [
    np.random.uniform(low=.3, high=.7, size=1)
    for _ in range(len(my_cmap))
]

for cmap, color, df in zip(my_cmap, rand, [group for _, group in df_mean.groupby('size')]):
    size = df['size'].values[0]
    ax.plot(df['tasks'], df['time w/ comms'], '.-', color=cmap(color), label=f'Size: {size}')
    # ax.plot(df['tasks'], df['time'], '.-', color=cmap(color), label=f'Size: {size}')

ax.legend(loc="best")
plt.savefig(f'./output/plot.png', bbox_inches='tight')
plt.close()







