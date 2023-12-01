import sys
import numpy as np

if len(sys.argv) != 4:
    print('Usage: python ./verify.py <m> <n> <p>')
    exit(0)

m, n, p = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])

A = np.fromfile('./files/A.bin', dtype=np.float64).reshape(m, n)
B = np.fromfile('./files/B.bin', dtype=np.float64).reshape(n, p)
C = np.fromfile('./files/C.bin', dtype=np.float64).reshape(m, p)

print(f'|AB - C|_1 = {np.sum(np.abs(A @ B - C))}')
