import pandas as pd
import matplotlib.pyplot as plt
import itertools
import subprocess

scl_types = ['baseline', 'auto', 'static', 'static_cs', 'dynamic', 'dynamic_cs', 'guided', 'guided_cs']
# chunk_sizes = ['4', '8', '16', '32', '64', '128']
# threads = ['1', '2', '4', '8', '12', '16']
chunk_sizes = ['1', '4', '8', '2048']
threads = ['2', '4', '8']
# n = '1024'
n = '4096'
# n = '16384'

scl_map = {val: str(i) for i, val in enumerate(scl_types)}

param_grid = [
    scl_types[3::2],
    chunk_sizes
]

combs = [(x, '0') for x in scl_types[0:2] + scl_types[2::2]] + list(itertools.product(*param_grid))
combs = [(x,y,z) for (x,y), z in itertools.product(combs, threads)]

output_csv = f'./output/results-{n}.csv'
# output_csv = f'./output/results1-{n}.csv'

# with open(output_csv, 'w') as file:
#     file.write('scl_type,chunk_size,threads,time\n')

# for scl_type, cs, threads in combs:
#     if scl_type != 'static_cs' and scl_type != 'static':
#         continue 
#     scl_val = scl_map[scl_type]
#     print(f'Scheduling: {scl_type}\tChunk Size: {cs}\tThreads: {threads}')
#     for _ in range(4):
#         subprocess.run(['./mvm', threads, n, scl_val, cs, output_csv])

subset = ['scl_type', 'chunk_size', 'threads']
df_mean = pd.read_csv(output_csv)
df_mean['time'] = df_mean.groupby(subset)['time'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)
df_mean['scl_type'] = df_mean['scl_type'].apply(lambda x : scl_types[x])

df_mean.to_csv('./output/mean_results.csv', index=False, sep='\t')

df_base = df_mean[df_mean['scl_type'] == 'baseline']
df_mean = df_mean[df_mean['scl_type'] != 'baseline']

for dfs_dim in [group for _, group in df_mean.groupby('chunk_size')]:
    chunk_size = dfs_dim['chunk_size'].values[0]
    chunk_size = chunk_size if chunk_size > 0 else 'Default'

    fig, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(f'Chunk Size = {chunk_size}, n = {n}')
    ax.set_xlabel('threads')
    ax.set_ylabel('time (sec)')
    ax.set_xticks([1] + list(range(2, 17, 2)))

    ax.plot(df_base['threads'], df_base['time'], '.-', label='Baseline')

    for df in [group for _, group in dfs_dim.groupby('scl_type')]:
        ax.plot(df['threads'], df['time'], '.-', label=f'Scheduling: {df["scl_type"].values[0]}')

    ax.legend(loc="best")
    plt.savefig(f'./output/plot-{chunk_size}-{n}.png', bbox_inches='tight')
    plt.close()


for dfs_dim in [group for _, group in df_mean.groupby('scl_type')]:
    scl_type = dfs_dim['scl_type'].values[0]

    if scl_type not in param_grid[0]:
        continue
    
    scl_type = scl_type[:-3]

    df_scl = df_mean[df_mean['scl_type'] == scl_type]

    fig, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(f'Scheduling = {scl_type}, n = {n}')
    ax.set_xlabel('threads')
    ax.set_ylabel('time (sec)')
    ax.set_xticks([1] + list(range(2, 17, 2)))

    for df in [group for _, group in dfs_dim.groupby('chunk_size')]:
        ax.plot(df['threads'], df['time'], '.-', label=f'Chunk Size: {df["chunk_size"].values[0]}')
        
    ax.plot(df_scl['threads'], df_scl['time'], '.-', label=f'Chunk Size: Default')

    ax.legend(loc="best")
    plt.savefig(f'./output/plot_sch_{scl_type}-{n}.png', bbox_inches='tight')
    plt.close()