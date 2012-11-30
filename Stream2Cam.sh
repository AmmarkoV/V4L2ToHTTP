#!/bin/bash
./v4l2tohttpcompat 0.0.0.0 8081 /dev/video0&
./v4l2tohttpcompat 0.0.0.0 8082 /dev/video1 
exit 0