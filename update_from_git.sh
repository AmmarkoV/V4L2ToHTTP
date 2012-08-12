#!/bin/bash

  if [ -d VideoInput ] 
   then
      cd VideoInput
      ./update_from_git.sh
      cd ..
     else
     echo "Could not find Video Input , cloning a fresh copy from github!"
     git clone git://github.com/AmmarkoV/VideoInput.git
   fi

  if [ -d AmmarServer ] 
   then
      cd AmmarServer
      ./update_from_git.sh
      cd ..
     else
     echo "Could not find AmmarServer , cloning a fresh copy from github!"
     git clone git://github.com/AmmarkoV/AmmarServer.git
   fi



#Reminder , to get a single file out of the repo use  "git checkout -- path/to/file.c"
git reset --hard HEAD
git pull
./make
exit 0
