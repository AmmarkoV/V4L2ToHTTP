#!/bin/bash
clear
killall memcheck-amd64-
 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libv4l/v4l2convert.so valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes  --track-origins=yes ./v4l2tohttp 0.0.0.0 8081 /dev/video0  2> V4L2ToHTTPDebug.log&
exit 0
