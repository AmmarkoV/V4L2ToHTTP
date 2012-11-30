#!/bin/bash

echo "System Installation is deactivated.. I havent decided on where to put content yet.."
exit 0

echo "Installing AmmarServer into system"
sudo echo " "

BINPATH="."
BINARY="v4l2tohttp"
  if [ -e $BINPATH/$BINARY ]
  then
    echo "$BINARY App is OK :) , including it to system binaries .."
    sudo cp $BINPATH/$BINARY /usr/bin/$BINARY 
  else
    echo "$BINARY App could not be installed , you probably got a library missing"
    exit 1
  fi
  
BINARY="v4l2tohttpcompat"
  if [ -e $BINPATH/$BINARY ]
  then
    echo "$BINARY App is OK :) , including it to system binaries .."
    sudo cp $BINPATH/$BINARY /usr/bin/$BINARY 
  else
    echo "$BINARY App could not be installed , you probably got a library missing"
    exit 1
  fi
    
   
exit 0