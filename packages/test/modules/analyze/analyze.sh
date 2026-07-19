#!/bin/sh

outv=`cat out | sed s/\|\n//`;
if [ "$outv" == "5" ]; then
	echo "TRUE" > result;
else
	echo "FALSE" > result;
fi;
