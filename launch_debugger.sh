#!/bin/bash

# Name of your C++ program executable
EXECUTABLE="./MOCO"

# Arguments for your C++ program
PROGRAM_ARGS="-b remap_hg_bed/TEAD4/TEAD4_ADNP -j Jaspar_2020/MA0809.2.jaspar -t Genomes/hg38/hg38.2bit -d 1 -f 0.003 -k 10"
            

# Command to launch gdb with your program
gdb --args $EXECUTABLE $PROGRAM_ARGS
