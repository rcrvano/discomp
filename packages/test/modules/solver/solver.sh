#!/bin/sh

input=`cat input | sed 's/\r|\n//'`;
sleep $input;
echo "Output " $input  > output
