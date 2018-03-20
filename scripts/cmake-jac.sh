#!/bin/bash

module purge

/home/jholloc/bin/cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug -DTARGET_TYPE=JET \
    -DNO_WRAPPERS=OFF -DNETCDF_DIR=/usr/local/depot/netcdf-4.1.1 -DHDF5_ROOT=/usr/local/depot/hdf5-1.8.5 \
    -DIDL_ROOT=/usr/local/depot/rsi/idl/idl80 \
    -DSWIG_DIR=/home/jholloc/freia/bin \
    -DCMAKE_INSTALL_PREFIX=$HOME/jac $*

#-DPYTHON_INCLUDE_DIR=/usr/local/depot/Python-3.3.5/include/python3.3m/ -DPYTHON_LIBRARY=/usr/local/depot/Python-3.3.5/lib/libpython3.3m.so