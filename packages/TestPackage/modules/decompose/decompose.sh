#!/bin/sh

COUNT=$1
num=0
for (( num=0; num<$COUNT; num++ )) do
	echo "`date` - `hostname` : $num"  > genlist$num.txt;
done;

