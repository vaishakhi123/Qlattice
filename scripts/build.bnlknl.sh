#!/bin/bash

set -e

{

./scripts/setenv.bnlknl.sh

# export USE_COMPILER=gcc

./scripts/cuba.sh
./scripts/zlib.sh
./scripts/eigen.sh
./scripts/autoconf.sh
./scripts/automake.sh
./scripts/c-lime.sh
./scripts/hdf5.sh

# export USE_COMPILER=intel

# ./scripts/openmpi.sh

./scripts/fftw_mpi.sh
./scripts/fftwf_mpi.sh

./scripts/qlat.sh
./scripts/grid.knl.sh
./scripts/gpt.sh

} |& tee $prefix/log.build.txt
