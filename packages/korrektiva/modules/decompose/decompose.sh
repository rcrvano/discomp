#!/bin/bash

#rm -rf /tmp/.wine*
chmod -R 700 /tmp/.wine*
/usr/bin/wine decompose.exe input.xml 1.mps 1>std.out  2>std.err
mv 1.mps 0.mps

