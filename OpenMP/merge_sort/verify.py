import numpy as np

initialArray = np.fromfile('./files/initialArray.bin', dtype=np.int32)
sortedArray = np.fromfile('./files/sortedArray.bin', dtype=np.int32)

if np.array_equal(np.sort(initialArray), sortedArray):
    print("The result is correct.")
else:
    print("The result is incorrect.")
