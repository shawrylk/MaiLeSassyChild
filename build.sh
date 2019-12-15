#!/bin/bash

FOLDER=./Fingerprint/

trap "echo \"*****Exit program******\"; rm -f Finger;" SIGINT SIGTERM

echo "*****Start compiling*****"
rm -f Finger
mv $FOLDER/Fingerprint.ino $FOLDER/Fingerprint.cpp
g++ -o Finger $FOLDER/*.cpp ./stub/*.cpp -I ./stub/ -std=gnu++17 -Wno-narrowing  -Wno-write-strings 
mv $FOLDER/Fingerprint.cpp $FOLDER/Fingerprint.ino
echo "*****Compile done*****"
echo "*****Start running program*****"
./Finger
