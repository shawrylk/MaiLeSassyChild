#!/bin/bash

HOME=$(pwd)
SOURCE=src
INCLUDE=inc
STUB=stub

trap "echo \"*****Exit program******\"; rm -f $HOME/Finger;" SIGINT SIGTERM

setup_stub()
{
	echo "*****Setup stub files******"
	cd $HOME/$STUB
       	mv $STUB_CPP $HOME/$SOURCE
       	mv $STUB_H $HOME/$INCLUDE
	mv $HOME/$SOURCE/Fingerprint.ino $HOME/$SOURCE/Fingerprint.cpp
}

clean_stub()
{
	echo "*****Clean stub files*****"
	cd $HOME/$SOURCE
       	mv $STUB_CPP $HOME/$STUB
	cd $HOME/$INCLUDE
       	mv $STUB_H $HOME/$STUB
	mv $HOME/$SOURCE/Fingerprint.cpp $HOME/$SOURCE/Fingerprint.ino
}

rm -f $HOME/Finger

if [ -d "$STUB" ]
then
	STUB_CPP=$(find $HOME/$STUB -name *.cpp -printf "%f ")
	STUB_H=$(find $HOME/$STUB -name *.h -printf "%f ")
	setup_stub
	echo "*****Start compiling*****"
	g++ -o $HOME/Finger $HOME/$SOURCE/*.cpp -I $HOME/$INCLUDE -std=gnu++17 -Wno-narrowing  -Wno-write-strings -pthread
	if [ $? == 1 ]
	then
		echo "Compilation fail, exit script"
		clean_stub
		exit 1
	fi
	clean_stub
else
	echo "Missing stub files, exit script"
	exit 1
fi

echo "*****Compile done*****"
echo "*****Start running program*****"
$HOME/Finger

