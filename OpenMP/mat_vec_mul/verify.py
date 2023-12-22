import sys
import numpy as np

if len(sys.argv) != 2:
    print('Usage: python ./verify.py <n>')
    exit(0)

n = int(sys.argv[1])

A = np.fromfile('./files/A.bin', dtype=np.float64).reshape(n, n)
x = np.fromfile('./files/x.bin', dtype=np.float64)
y = np.fromfile('./files/y.bin', dtype=np.float64)

print(f'|Ax - y|_1 = {np.sum(np.abs(A @ x - y))}')
