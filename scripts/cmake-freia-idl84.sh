#!/bin/bash

module purge
module load idl/08.4
module load java/1.8
module load hdf5-C/1.8.13

export HDF5_USE_SHLIB=yes
export BOOST_ROOT=/usr/local/depot/boost-1.60

CC=gcc CXX=g++ cmake3 -Bbuild_freia -H. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=/projects/UDA/uda-install-develop \
    -DBUILD_SHARED_LIBS=ON \
    $*
