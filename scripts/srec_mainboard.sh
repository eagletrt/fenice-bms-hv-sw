#!/usr/bin/bash

mkdir -p binaries &&
./bin2srec -a 0x8004000 -i ../mainboard/build/fenice-bms.bin -o binaries/fenice-bms.srec