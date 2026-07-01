#!/usr/bin/bash

mkdir -p binaries &&
./bin2srec -a 0x8005000 -i ../cellboard/build/release/cellboard.bin -o binaries/cellboard.srec
