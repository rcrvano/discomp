#!/bin/sh
cd client
#make clean
make
cd ..

cd server
#make clean
make
cd ..

cd rpc_client
#make clean
make
cd ..

cd httpserver
#make clean
make
cd ..


#cp ../bin/bin/discomp_server server/
#cp ../bin/bin/discomp_client client/
#cd ../bin/bin/
#cp * /usr/share/discomp/bin/
