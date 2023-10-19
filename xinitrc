#!/bin/bash

xterm&
xclock &
xeyes &
sleep 2 &
export GLOG_logtostderr=1
export GLOG_stderrthreshold=0
export GLOG_log_dir="$HOME/projects/scwm/"
exec ./scwm
