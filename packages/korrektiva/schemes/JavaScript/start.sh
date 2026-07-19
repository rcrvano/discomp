#!/bin/sh

PROCESS_COUNT=$2
if [ "$PROCESS_COUNT" == "" ]; then
        echo "You should specify process count";
        echo "Usage example: $0 -np 10"
        exit;
fi;


if [ $PROCESS_COUNT -ge 81 ]; then
        echo "Process count couldn't be greather than 80";
        exit;
fi;


./stop.sh


#killing evrything what we can kill
#discomp_cluster_rsh "killall -9 minisat2; killall -9 discomp_client" > /dev/null 2>&1

#starting server
killall -9 discomp_server > /dev/null 2>&1
discomp_server_restart
#discomp_cluster_quick_restart


#starting clients
mpirun -np $PROCESS_COUNT $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1000 -l 1000
#mpirun -np $PROCESS_COUNT $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1

rm -rf finished.flag

sleep 5;
discomp_rpc_nodes_list
