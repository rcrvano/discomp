#!/bin/sh

#killing evrything what we can kill
#discomp_cluster_rsh "killall -9 minisat2; killall -9 discomp_client" > /dev/null 2>&1

tasks -d all
rm -rf discomp_client*
rm -rf machinefile*

#starting server
killall -9 discomp_client
killall -9 discomp_server > /dev/null 2>&1


