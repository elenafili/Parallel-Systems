import sys
import numpy as np

if len(sys.argv) != 2:
    print('Usage: python ./verify.py <n>')
    exit(0)

n = int(sys.argv[1])

A = np.fromfile('./files/A.bin', dtype=np.float64).reshape(n, n)
x = np.fromfile('./files/x.bin', dtype=np.float64).reshape(n)
b = np.fromfile('./files/b.bin', dtype=np.float64).reshape(n)

# print(A)
# print(x)
# print(b)

print(f'|Ax - b|_1 = {np.sum(np.abs(A @ x - b))}')


# A = np.fromfile('./files/A_trig.bin', dtype=np.float64).reshape(n, n)
# b = np.fromfile('./files/b_solv.bin', dtype=np.float64).reshape(n)
# print(A)
# print(b)