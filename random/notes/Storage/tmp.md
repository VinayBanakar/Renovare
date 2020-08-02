sudo mount -t tmpfs -o size=500M,mode=777 tmpfs /mnt/mytmfs
Or you could just use /dev/shm
other ram disks: mknod -m 660 /dev/ram b 1 1   and   mknod -m 400 /dev/initrd b 1 250 (see man ram and man initrd)