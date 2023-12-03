import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


import itertools
import subprocess


names = ['n', 'threads']
param_grid = [
    [1e7, 1e8, 1e9, 5e9, 1e10],
    [1, 2, 4, 8, 16, 32],
]

output_csv = './output/results.csv'
with open(output_csv, 'w') as file:
    file.write('n,threads,time\n')

for i, (n, threads) in enumerate(list(itertools.product(*param_grid))):
    process = subprocess.run(['./monte_carlo', str(threads), str(int(n)), output_csv])


log = lambda values : [np.log10(x) for x in values]
fig, ax = plt.subplots()
ax.grid(visible=True)
ax.set_xticks(np.arange(0, 11, 1))
ax.set_xlabel('log_10 (n)')
ax.set_ylabel('time (sec)')

dfs = [group for _, group in pd.read_csv(output_csv).groupby('threads')]
for i, (threads, df) in enumerate(zip(param_grid[1], dfs)):
    ax.plot(log(df['n']), df['time'], '.-', label=f'Threads: {threads}')


ax.legend(loc="best")
plt.savefig(f'./output/plot.png', bbox_inches='tight')