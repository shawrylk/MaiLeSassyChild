#!/bin/bash

trap "echo \"*****Exit program******\"; rm -f Finger;" SIGINT SIGTERM

echo "*****Start compiling*****"
rm -f Finger
mv Fingerprint.ino Fingerprint.cpp
g++ -o Finger ./*.cpp ./stub/*.c -I ./stub/ -std=gnu++17 -Wno-narrowing  -Wno-write-strings 
mv Fingerprint.cpp Fingerprint.ino
echo "*****Compile done*****"
echo "*****Start running program*****"
./Finger
