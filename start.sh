#!/bin/sh

PROCESS_COUNT=1
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
discomp_server_restart

#discomp_cluster_quick_restart


#starting clients

#mpirun -np 1 $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1000 -l 1000
#mpirun -np 1 $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1000 -l 1000
$DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf  &


#mpirun -np 1 $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1000 -l 1000

#mpirun -np 1 $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1000 -l 1000
#mpirun -np 1 $DISCOMP_CLIENT/bin/discomp_client --config $DISCOMP_CLIENT/etc/client.conf -maxtime 1000 -l 1000

#cd korrektiva

#cd vulnerability 
#./start.sh

#rm -rf finished.flag
echo "Please check the nodes list with 'discomp_nodes_list' command"
#sleep 5;
#discomp_nodes_list -nowatch
