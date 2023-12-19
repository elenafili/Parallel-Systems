import pandas as pd
import matplotlib.pyplot as plt
import itertools
import subprocess


type_map = { 
    0: 'No Parallezation',
    1: 'Reverse Only',
    2: 'Gauss Elim - Second loop',
    3: 'Gauss Elim - Third loop',
    4: 'Gauss Elim - Second loop + Reverse',
    5: 'Gauss Elim - Third loop + Reverse',
}


ns = ['1024', '4096']
threads = ['1', '2', '4', '8', '12', '16']
vars = ['', '-DTRI1', '-DTRI2', '-DTRI1 -DREV', '-DTRI2 -DREV']

var_map = { x: type_map[i] for i, x in enumerate([vars[0]] + ['-DREV'] + vars[1:]) }

param_grid = [
    ns,
    threads[1:],
    vars[1:]
]

combs = list(itertools.product(*param_grid)) + [(n, '1', '') for n in ns]

output_csv = f'./output/results.csv'

with open(output_csv, 'w') as file:
    file.write('n,threads,time_trig,time_rev,type\n')

for n, threads, var in combs:
    print(f'n: {n}\tThreads: {threads}\tType: {var_map[var]}')
    subprocess.run(['make', 'clean'])
    subprocess.run(['make', '-s', 'gauss', f'FLAGS={var if var != "" else "-DS"}'])
    for _ in range(4):
        subprocess.run(['./gauss', threads, n, output_csv])


subset = ['n', 'threads']
df_mean = pd.read_csv(output_csv)
df_mean['time_trig'] = df_mean.groupby(subset)['time_trig'].transform('mean')
df_mean['time_rev'] = df_mean.groupby(subset)['time_rev'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)

df_mean.to_csv('./output/mean_results.csv', index=False, sep='\t')

print(df_mean)

# for dfs_dim in [group for _, group in df_mean.groupby('chunk_size')]:
#     fig, ax = plt.subplots()
#     ax.grid(visible=True)
#     ax.set_xlabel('threads')
#     ax.set_ylabel('time (sec)')
#     ax.set_xticks([1] + list(range(2, 17, 2)))

#     for df in [group for _, group in dfs_dim.groupby('scl_type')]:
#         ax.plot(df['threads'], df['time'], '.-', label=f'Scheduling: {df["scl_type"].values[0]}')

#     ax.legend(loc='best')
#     plt.savefig(f'./output/plot-{chunk_size}-{n}.png', bbox_inches='tight')
#     plt.close()