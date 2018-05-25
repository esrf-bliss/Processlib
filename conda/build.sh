#!/bin/bash
cmake -Bbuild -H. -DPROCESSLIB_ENABLE_PYTHON=1 -DPROCESSLIB_ENABLE_TESTS=1 -DCMAKE_INSTALL_PREFIX=$PREFIX -DPYTHON_SITE_PACKAGES_DIR=$SP_DIR -DCMAKE_FIND_ROOT_PATH=$PREFIX
cmake --build build --target install

# Conda requires the library to be installed in /lib not /lib64 regardeless of the HFS standard.
mv -f $PREFIX/lib64/* $PREFIX/lib
rmdir $PREFIX/lib64
