#!/bin/bash

  if [ -d VideoInput ] 
   then
     echo "Video Input dir already exists.."
     else
     echo "Could not find Video Input , cloning a fresh copy from github!"
     git clone git://github.com/AmmarkoV/VideoInput.git
   fi

  if [ -d AmmarServer ] 
   then
     echo "AmmarServer dir already exists.."
     else
     echo "Could not find AmmarServer , cloning a fresh copy from github!"
     git clone git://github.com/AmmarkoV/AmmarServer.git
   fi

  if [ -e VideoInput/make ] 
   then
     echo "Make bash script for Video Input is OK !"
   else
     echo "The script was unable to find the VideoInput library or to automatically download it "
     echo "Please try installing git ( sudo apt-get install git ) or unzipping manually a recent version of VideoInput"
     echo "https://github.com/AmmarkoV/VideoInput   or http://ammar.gr"
     exit 1  
fi

  if [ -e AmmarServer/make ] 
   then
     echo "Make bash script for AmmarServer is OK !"
   else
     echo "The script was unable to find the VideoInput library or to automatically download it "
     echo "Please try installing git ( sudo apt-get install git ) or unzipping manually a recent version of VideoInput"
     echo "https://github.com/AmmarkoV/VideoInput   or http://ammar.gr"
     exit 1  
fi


cd AmmarServer
./make
cd ..

cd VideoInput
./make
cd ..

OUTPUT="v4l2tohttp"

if [ -e $OUTPUT ]
then
  rm $OUTPUT
fi 

CFLAGS="-s " 
OURLIBRARIES="AmmarServer/src/AmmServerlib/libAmmServerlib.a VideoInput/libVideoInput.a" 
LIBRARIES="-pthread -lpng -ljpeg -lrt -lz"

g++  src/main.cpp -L. $OURLIBRARIES $LIBRARIES -o $OUTPUT

if [ -e $OUTPUT ]
then
  echo "Success.."
else
  echo "Failure.."
fi

exit 0