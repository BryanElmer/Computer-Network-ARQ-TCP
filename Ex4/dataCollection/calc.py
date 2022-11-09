from genericpath import isfile
import os
filePath = "data.txt"
f = open(filePath, "r")

totalTime = 0.00
totalRate = 0.00
arr = f.readlines()
print(arr)
totalLines = len(arr) - 1

# PUT TIME HERE

for i in range(totalLines):
    if i % 2 == 0:
        totalTime += float(arr[i].replace('\n', ''))
    else:
        totalRate += float(arr[i].replace('\n', ''))

# a = float(f.readline())
# v = float(f.readline())
# b = float(f.readline())
# w = float(f.readline())
# c = float(f.readline())
# x = float(f.readline())
# d = float(f.readline())
# y = float(f.readline())
# e = float(f.readline())
# z = float(f.readline())

f.close()
if os.path.isfile(filePath):
    os.remove(filePath)

# tAvg = (a+b+c+d+e)/5
# rAvg = (v+w+x+y+z)/5
tAvg = totalTime / (totalLines / 2)
rAvg = totalRate / (totalLines / 2)
# print("Times:", a, b, c, d, e)
print(f'Avg Time: {tAvg:.3f}')
# print("Rates:", v, w, x, y, z)
print(f'Avg Rate: {rAvg:.3f}')
