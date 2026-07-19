#!/bin/sh

cat reslist* >> result.txt
echo "-------------" >> result.txt;
echo "`date` - `hostname` : RESULT GENERATED" >> result.txt
