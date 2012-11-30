#!/bin/bash

BINARY="v4l2tohttp"
  if [ -e /usr/bin/$BINARY ]
  then
    sudo rm  /usr/bin/$BINARY 
  else  
    echo "No $BINARY detected "
  fi

BINARY="v4l2tohttpcompat"
  if [ -e /usr/bin/$BINARY ]
  then
    sudo rm  /usr/bin/$BINARY 
  else  
    echo "No $BINARY detected "
  fi

 
exit 0