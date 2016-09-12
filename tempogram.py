import subprocess
import numpy as np
import matplotlib.pyplot as plt
file = open("./tempogram.txt", "r")
line1 =  file.readline().split(' ')
xdim = int(line1[1])
ydim = int(line1[0])
print (xdim,ydim)
i = 0
a = np.zeros((ydim,xdim),dtype=float)
while (i < ydim):
    j = 0
    line = file.readline().split(' ')
    if len(line) < xdim:
        break
    while(j < xdim):
        # print (line[j])
        # print (i,j)
        a[i][j] = float(line[j])
        j += 1
    i += 1
plt.imshow(a)
plt.gca().invert_yaxis()
plt.show()
file.close()
