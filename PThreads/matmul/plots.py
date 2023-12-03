import pandas as pd
import matplotlib.pyplot as plt


import itertools
import subprocess


dims = ['m', 'n', 'p']
methods = ['./fs', './2d', './pad_var', './priv_var']

s1, s2, s3 = '8', '8000', '8000000'

sizes = [
    [s2, s2, '80'],
    [s2, s2,  '1'],
    [s2, s2,  '2'],
    [s2, s2,  '4'],
    [s2, s2,  '8'],
    [s1, s3,  '1'],
    [s1, s3,  '2'],
    [s1, s3,  '4'],
    [s1, s3,  '8'],
]

threads = ['2', '4', '8']
padding_sizes = ['2', '4', '8', '16']

combs = [[name] + size for name in methods for size in sizes if name != './pad_var']
combs += [['./pad_var'] + size + [padding] for size in sizes for padding in padding_sizes]
combs = [x + [thread] for x in combs for thread in threads]

output_csv = './output/results.csv'

# with open(output_csv, 'w') as file:
#     file.write('n,threads,time\n')

# for params in combs:
#     process = subprocess.run(list(params) + [output_csv])


fig, ax = plt.subplots()
ax.grid(visible=True)
ax.set_xlabel('n')
ax.set_ylabel('time (sec)')

dfs = [group for _, group in .groupby('threads')]
for i, (threads, df) in enumerate(zip(param_grid[1], dfs)):
    ax.plot(df['n'], df['time'], '.-', label=f'Threads: {threads}')


ax.legend(loc="best")
plt.savefig(f'./output/plot.png', bbox_inches='tight')