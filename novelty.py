import subprocess
import numpy as np
import matplotlib.pyplot as plt
file = open("./novelty.txt", "r")
line1 =  file.readline()
xdim = int(line1)
print (xdim)
a = np.zeros((xdim),dtype=float)
j = 0
line = file.readline().split(' ')
while(j < xdim):
    # print (line[j])
    print (j)
    a[j] = float(line[j])
    j += 1
plt.plot(np.arange(0, xdim, 1),a)
plt.show()
file.close()
