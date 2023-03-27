#!/bin/bash

pwd
q_verbose=10 OMP_NUM_THREADS=2 time mpirun --np 2 ./qlat.x >log.out 2>log.err
cat log.out | grep -v '^Grid :\|^Timer\|^check_status:\|^display_geo_node : id_node =' >log
cat log.out log.err > log.full
