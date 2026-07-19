#!/bin/sh

params=`cat file.in`;
#chmod -R 700 /tmp/.wine*
export LD_LIBRARY_PATH='/home/isidorov/task5/task5/lib'


./main --chart=file.out $params
