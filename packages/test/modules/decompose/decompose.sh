#!/bin/bash

let count=`cat decompose_num | sed 's/\r|\n//'`;

for  ((i=1; i<$count; i++)); do
	echo $i > input$i;
done;
