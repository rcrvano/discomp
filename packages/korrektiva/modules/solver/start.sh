#!/bin/sh

chmod -R 700 /tmp/.wine*
DISPLAY= wine solver.exe file.mps file.out
