import numpy as np
import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
mpl.style.use('seaborn-darkgrid')


output_csv = './output/out.csv'
df = pd.read_csv(output_csv)
df['nodes'] = [i for _ in range(75 // 3) for i in [1, 4, 16] for _ in range(4)]


log = lambda values : [np.log10(x) for x in values]
subset = ['nodes', 'tasks', 'n']
df['time'] = df.groupby(subset)['time'].transform('mean')
df.drop_duplicates(subset=subset, keep='first', inplace=True) 
df = df.reset_index(drop=True)

df = df[subset + ['time']]


df_single = df[df['nodes'] == 1]
df_multi2 = df[df['nodes'] == 4]
df_multi4 = df[df['nodes'] == 16]

log = lambda values : [np.log10(x) for x in values]

def plot_n(df, name, title):
    _, ax = plt.subplots()
    ax.grid(visible=True)
    ax.set_title(title)
    ax.set_xticks(np.arange(0, 17, 1))
    ax.set_xlabel('Tasks')
    ax.set_ylabel('time (sec)')

    for df_ns in [group for _, group in df.groupby('n')]:
        n = df_ns['n'].values[0]
        ax.plot(df_ns['tasks'], df_ns['time'], '.-', label=f'n: {n}')

    ax.legend(loc="best")
    plt.savefig(f'./output/{name}.png', bbox_inches='tight')
    plt.close()

plot_n(df_single, 'plot_single', 'Monte Carlo on Single Node')
plot_n(df_multi2, 'plot_multi2', 'Monte Carlo on 2 Nodes')
plot_n(df_multi4, 'plot_multi4', 'Monte Carlo on 4 Nodes')