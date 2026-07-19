#!/bin/sh

TASK_NAME=test.JavaScript

#discomp_rpc_process_stop $TASK_NAME > /dev/null
tasks -d all
killall -9 discomp_server > /dev/null 2>&1
discomp_cluster_rsh "killall -9 -u `whoami`" > /dev/null 2>&1
rm -rf discomp_client.*
rm -rf machinefile*
touch finished.flag
