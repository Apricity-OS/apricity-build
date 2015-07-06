#!/usr/bin/python
# -*- coding: UTF-8 -*-

import subprocess

# get free space
df = subprocess.Popen(["df", "/dev/sda1"], stdout=subprocess.PIPE)
device, size, used, available, percent, mountpoint = \
 df.communicate()[0].decode("utf-8").split("\n")[1].split()
print(size)

df = subprocess.Popen(["df", "/dev/sdb1"], stdout=subprocess.PIPE)
device, size, used, available, percent, mountpoint = \
 df.communicate()[0].decode("utf-8").split("\n")[1].split()
print(size)






lsblk = subprocess.Popen(["lsblk", "-lnb"], stdout=subprocess.PIPE)
output = lsblk.communicate()[0].decode("utf-8").split("\n")

max_size = 0

for item in output:
	col = item.split()
	if len(col) >= 5:
		if col[5] == "disk" or col[5] == "part":
			size = int(col[3])
			if size > max_size:
				max_size = size

print (max_size)

#print(output)
