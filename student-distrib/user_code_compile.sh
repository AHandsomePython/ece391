#!/bin/bash
cd ../syscalls
sudo rm $1.exe
sudo rm ./to_fsdir/$1
sudo make $1.exe
sudo make $1
cd ..
sudo cp syscalls/to_fsdir/$1 fsdir/
echo "build"
sudo ./createfs -i fsdir -o student-distrib/filesys_img
cd student-distrib
make clean
make
gdb bootimg
target remote 10.0.2.2:1234

