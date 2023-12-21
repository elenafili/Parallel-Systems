import pandas as pd
import matplotlib.pyplot as plt
import itertools
import subprocess


type_map = { 
    0: 'No Parallezation',
    1: 'Back Only',
    2: 'Gauss Elim - Second loop',
    3: 'Gauss Elim - Second loop + Back',
    4: 'Gauss Elim - Third loop',
    5: 'Gauss Elim - Third loop + Back',
}

var_map = { 
    '': 'No Parallezation',
    '-DBACK': 'Back Only',
    '-DTRI1': 'Gauss Elim - Second loop',
    '-DTRI1 -DBACK': 'Gauss Elim - Second loop + Back',
    '-DTRI2': 'Gauss Elim - Third loop',
    '-DTRI2 -DBACK': 'Gauss Elim - Third loop + Back',
}


ns = ['1024', '4096']
threads = ['1', '2', '4', '8', '12']
# ns = ['10240']
# threads = ['1', '2', '4', '8']
vars = ['', '-DTRI1 -DBACK', '-DTRI2 -DBACK']

# var_map = { x: type_map[i] for i, x in enumerate([vars[0]] + ['-DBACK'] + vars[1:]) }

param_grid = [
    ns,
    threads[1:],
    vars[1:]
]

combs = [(n, '1', '') for n in ns] + list(itertools.product(*param_grid))

output_csv = f'./output/results.csv'

with open(output_csv, 'w') as file:
    file.write('n,threads,time_trig,time_rev,type\n')

for n, threads, var in combs:
    print(f'n: {n}\tThreads: {threads}\tType: {var_map[var]}')
    subprocess.run(['make', 'clean'])
    subprocess.run(['make', '-s', 'gauss', f'FLAGS={var if var != "" else "-DS"}'])
    for _ in range(4):
        subprocess.run(['./gauss', threads, n, output_csv])


subset = ['n', 'threads', 'type']
df_mean = pd.read_csv(output_csv)
df_mean['time_trig'] = df_mean.groupby(subset)['time_trig'].transform('mean')
df_mean['time_rev'] = df_mean.groupby(subset)['time_rev'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)

df_mean.to_csv('./output/mean_results.csv', index=False, sep='\t')

df_base = df_mean[df_mean['type'] == 0]
df_mean = df_mean[df_mean['type'] != 0]

for type in df_mean['type'].unique():
    df_base['type'] = type
    df_mean = pd.concat([df_base, df_mean], ignore_index=True)

for dfs_dim in [group for _, group in df_mean.groupby('n')]:
    n = dfs_dim['n'].values[0]

    fig, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(f'Trigonalization, n = {n}')
    ax.set_xlabel('threads')
    ax.set_ylabel('time (sec)')
    ax.set_xticks([1] + list(range(2, 17, 2)))

    for df in [group for _, group in dfs_dim.groupby('type')]:
        if df["type"].values[0] % 2 != 0:
            continue
        ax.plot(df['threads'], df['time_trig'], '.-', label=type_map[df["type"].values[0]])

    ax.legend(loc='best')
    plt.savefig(f'./output/plot-trig-{n}.png', bbox_inches='tight')
    plt.close()

for dfs_dim in [group for _, group in df_mean.groupby('n')]:

    n = dfs_dim['n'].values[0]

    fig, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(f'Back-Substitution, n = {n}')
    ax.set_xlabel('threads')
    ax.set_ylabel('time (sec)')
    ax.set_xticks([1] + list(range(2, 17, 2)))

    # for df in [group for _, group in dfs_dim.groupby('type')]:
    #     ax.plot(df['threads'], df['time_rev'], '.-', label=type_map[df["type"].values[0]])
        
    # ax.legend(loc='best')

    subset = ['threads']
    dfs_dim['time_rev'] = dfs_dim.groupby(subset)['time_rev'].transform('mean')
    dfs_dim.drop_duplicates(subset=subset, keep='first', inplace=True) 
    dfs_dim = dfs_dim.reset_index(drop=True)
    ax.plot(dfs_dim['threads'], dfs_dim['time_rev'], '.-')

    plt.savefig(f'./output/plot-back-{n}.png', bbox_inches='tight')
    plt.close()