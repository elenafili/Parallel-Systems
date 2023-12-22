import sys
import numpy as np

if len(sys.argv) != 2:
    print('Usage: python ./verify.py <n>')
    exit(0)

n = int(sys.argv[1])

A = np.fromfile('./files/A.bin', dtype=np.float64).reshape(n, n)
x = np.fromfile('./files/x.bin', dtype=np.float64)
b = np.fromfile('./files/b.bin', dtype=np.float64)

# print(A)
# print(x)
# print(b)

print(f'|Ax - b|_1 = {np.sum(np.abs(A @ x - b))}')

# for i in range(n):
#     for j in range(i+1, n):
#         ratio = A[j][i] / A[i][i]
#         for k in range(i, n):
#             A[j][k] -= ratio * A[i][k]
#             # print(f'{i} {j} {k}: {A[j][k]:.6f} {ratio:.6f} {A[i][k]:.6f}')
#         b[j] -= ratio * b[i]

# print(A)
# print(b)

# A1 = np.fromfile('./files/A_trig.bin', dtype=np.float64).reshape(n, n)
# b1 = np.fromfile('./files/b_solv.bin', dtype=np.float64).reshape(n)
# print(A1)
# print(b1)

# print(np.sum(np.abs(A-A1)))
# print(np.abs(b-b1))