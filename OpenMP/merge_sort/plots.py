import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


import itertools
import subprocess


names = ['n', 'threads']
param_grid = [
    [1e7, 2e7, 5e7, 1e8],
    [1, 2, 4, 8, 12, 16, 32],
]

output_csv = './output/results.csv'
# with open(output_csv, 'w') as file:
#     file.write('n,threads,time\n')

# for i, (n, threads) in enumerate(list(itertools.product(*param_grid))):
#     for _ in range(4):
#         subprocess.run(['./merge_sort', str(int(n)), str(threads), output_csv])


log = lambda values : [np.log10(x) for x in values]
subset = ['n', 'threads']
df_mean = pd.read_csv(output_csv)
df_mean['time'] = df_mean.groupby(subset)['time'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)

fig, ax = plt.subplots()
ax.grid(visible=True)
ax.set_xticks(np.arange(0, 11, 1))
ax.set_xlabel('log_10 (n)')
ax.set_ylabel('time (sec)')


dfs = [group for _, group in df_mean.groupby('threads')]
for i, (threads, df) in enumerate(zip(param_grid[1], dfs)):
    ax.plot(log(df['n']), df['time'], '.-', label=f'Threads: {threads}')


ax.legend(loc="best")
plt.savefig(f'./output/plot.png', bbox_inches='tight')

fig, ax = plt.subplots()
ax.grid(visible=True)
ax.set_xticks([1, 2] + list(range(4, 33, 4)))
ax.set_xlabel('threads')
ax.set_ylabel('time (sec)')

dfs = [group for _, group in df_mean.groupby('n')]
for i, (n, df) in enumerate(zip(param_grid[0], dfs)):
    ax.plot(df['threads'], df['time'], '.-', label=f'n: {int(n)}')


ax.legend(loc="best")
plt.savefig(f'./output/plot_threads.png', bbox_inches='tight')