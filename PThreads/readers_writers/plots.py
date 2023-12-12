import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


import itertools
import subprocess
import time

initial_insertions = 1000
operations = 500000

names = ['method', 'member', 'insert', 'threads', 'time']
param_grid = [
    ['./rp', './wp'],
    ['0.9', '0.95', '0.999'],
    ['0.001', '0.005', '0.05'],
    ['2', '4', '8', '16']
]

df = pd.DataFrame(columns=names)

for i, (method, members, inserts, threads) in enumerate(list(itertools.product(*param_grid))):
    if float(members) + float(inserts) > 1:
        continue

    ops = f'{initial_insertions}\n{operations}\n{members}\n{inserts}\n'
    start_time = time.time()
    process = subprocess.run([method, threads], input=ops, text=True)
    end_time = time.time() - start_time
    
    new_row = pd.DataFrame({key: [val] for key, val in zip(names, [method, members, inserts, threads, end_time])})
    df = pd.concat([df, new_row], ignore_index=True)

print(df)

df.to_csv('./output/results.csv', index=False)


subset = ['method', 'member', 'insert', 'threads']
df_mean = pd.read_csv('./output/results.csv')
df_mean['time'] = df_mean.groupby(subset)['time'].transform('mean')
df_mean.drop_duplicates(subset=subset, keep='first', inplace=True) 
df_mean = df_mean.reset_index(drop=True)
for df_mem in [group for _, group in df_mean.groupby(['member', 'insert'])]:
    
    members = df_mem["member"].values[0]
    insert = df_mem["insert"].values[0]

    fig, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(f'Member() / Insert() Percentages: {members * 100 :.2f} % / {insert * 100 :.2f} %')
    ax.set_xlabel('threads')
    ax.set_ylabel('time (sec)')

    for df in [group for _, group in df_mem.groupby(['method'])]:
        ax.plot(df['threads'], df['time'], '.-', label=f'Method: {df["method"].values[0][2:]}')

    ax.legend(loc="best")
    plt.savefig(f'./output/plot{members}-{insert}.png', bbox_inches='tight')
    plt.close()