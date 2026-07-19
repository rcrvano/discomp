#!/bin/sh

#iscomp_cluster_rsh "killall -9 jerusat.exe" > /dev/null 2>&1
rm  -f controlpoints.xml > /dev/null 2>&1

#discomp_cluster_quick_restart
discomp_rpc_process_start test.scheme1 --login admin --password q1w2e3
