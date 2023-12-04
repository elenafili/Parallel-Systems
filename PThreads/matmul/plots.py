import pandas as pd
import matplotlib.pyplot as plt
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

with open(output_csv, 'w') as file:
    file.write('method,threads,padding,time,m,n,p\n')

for params in combs:
    process = subprocess.run(list(params) + [output_csv])


for dfs_dim in [group for _, group in pd.read_csv(output_csv).groupby(['m', 'n', 'p'])]:
    m = dfs_dim["m"].values[0]
    n = dfs_dim["n"].values[0]
    p = dfs_dim["p"].values[0]

    for pad in padding_sizes:

        fig, ax = plt.subplots()
        ax.grid(visible=True)
        ax.set_title(f'm={m}, n={n}, p={p}')
        ax.set_xlabel('method')
        ax.set_ylabel('time (sec)')

        for df in [group for _, group in dfs_dim.groupby('threads')]:
            uniq_df = df[df['padding'].isin([0, int(pad)])]
            ax.plot(uniq_df['method'].apply(lambda x : x if x != 'padding' else f'padding={pad}'), 
                    uniq_df['time'], '.-', label=f'Threads: {df["threads"].values[0]}')

        ax.legend(loc="best")
        plt.savefig(f'./output/plot{m}-{n}-{p}-{pad}.png', bbox_inches='tight')
        plt.close()

