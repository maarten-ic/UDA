#!/bin/bash

module purge
module load idl/08.1
module load gcc/4.8.2
module load hdf5/1.8.11

export BOOST_ROOT=/usr/local/depot/boost-1.53

/home/jholloc/fuslw/bin/cmake -Bbuild_fuslw -H. -DCMAKE_BUILD_TYPE=Debug -DTARGET_TYPE=MAST -DIDA_ROOT=/usr/local/depot/ida -DLIBMEMCACHED_ROOT=/home/jholloc/fuslw \
    -DMDSPLUS_DIR=/usr/local/depot/mdsplus-5.0 \
    -DCMAKE_INSTALL_PREFIX=$HOME/fuslw -DBoost_NO_BOOST_CMAKE=ON -DEXTRA_LD_LIBRARY_PATHS=/usr/local/lib \
    -DPYTHON_INCLUDE_DIR=/usr/local/depot/Python-3.3.5/include/python3.3m/ -DPYTHON_LIBRARY=/usr/local/depot/Python-3.3.5/lib/libpython3.3m.so \
    -DEXTRA_LD_LIBRARY_PATHS=/usr/local/depot/ida/lib -DSWIG_DIR=/home/jholloc/fuslw/bin $*

#-DIMAS_ROOT=$HOME/Projects/CPT/installer_hdf5/src/master/3/UAL \
