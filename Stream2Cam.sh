#!/bin/bash
./run_v4l2_to_http 0.0.0.0 8081 /dev/video0&
./run_v4l2_to_http 0.0.0.0 8082 /dev/video1 
exit 0