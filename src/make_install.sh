#!/bin/sh
cd ../bin/bin/
cp * /usr/share/discomp/server/bin/
cp discomp_client /common/tmp/

all_node_command "killall -9 discomp_client"

all_node_command "cp /common/tmp/discomp_client /usr/share/discomp/client1/bin/discomp_client"
all_node_command "cp /common/tmp/discomp_client /usr/share/discomp/client2/bin/discomp_client"
// vim: set fenc=utf-8 tabstop=8 :
