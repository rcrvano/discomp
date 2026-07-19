#!/bin/sh

all_node_command "rm -rf /tmp/discomp"


cd /store1/discomp/client/modules/
./update_modules.sh 


cd /store1/discomp/server/packages/
chown -R isidorov:users *
chmod -R 777 *
