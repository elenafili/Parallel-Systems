import pandas as pd
import matplotlib.pyplot as plt
import itertools
import subprocess

scl_types = ['auto', 'static', 'static cs', 'dynamic', 'dynamic cs', 'guided', 'guided cs', 'runtime']
chunk_sizes = ['1', '2', '4', '8', '16', '32', '64', '128']
threads = ['1', '2', '4', '6', '8', '12', '16']
n = '4096'

scl_map = {val: str(i) for i, val in enumerate(scl_types)}

param_grid = [
    scl_types[2::2],
    chunk_sizes
]

combs = [(x, '0') for x in [scl_types[0]] + scl_types[1::2]] + list(itertools.product(*param_grid))
combs = [(x,y,z) for (x,y), z in itertools.product(combs, threads)]

output_csv = './output/results.csv'

with open(output_csv, 'w') as file:
    file.write('scl_type,chunk_size,threads,time\n')

for scl_type, cs, threads in combs:
    scl_val = scl_map[scl_type]
    print(f'Scheduling: {scl_type}\tChunk Size: {cs}\tThreads: {threads}')
    for _ in range(4):
        subprocess.run(['./mvm', threads, n, scl_val, cs, output_csv])

subset = ['scl_type', 'chunk_size', 'threads']
df_mean = pd.read_csv(output_csv)
df_mean['time'] = df_mean.groupby(subset)['time'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)
df_mean['scl_type'] = df_mean['scl_type'].apply(lambda x : scl_types[x])

# print(df_mean.head(100))

df_mean.to_csv('./output/mean_results.csv', index=False, sep='\t')

for dfs_dim in [group for _, group in df_mean.groupby('chunk_size')]:
    # print(dfs_dim)
    chunk_size = dfs_dim['chunk_size'].values[0]

    fig, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(f'Chunk Size = {chunk_size}')
    ax.set_xlabel('threads')
    ax.set_ylabel('time (sec)')

    for df in [group for _, group in dfs_dim.groupby('scl_type')]:
        ax.plot(df['threads'], df['time'], '.-', label=f'Scheduling: {df["scl_type"].values[0]}')

    ax.legend(loc="best")
    plt.savefig(f'./output/plot{chunk_size}.png', bbox_inches='tight')
    plt.close()