#!/usr/bin/bash

mkdir -p binaries &&
./bin2srec -a 0x8004000 -i ../cellboard/build/cellboard.bin -o binaries/cellboard.srec
