#!/bin/bash
cmake -Bbuild -H. -GNinja -DPROCESSLIB_ENABLE_PYTHON=1 -DPROCESSLIB_ENABLE_TESTS=1 -DCMAKE_INSTALL_PREFIX=$PREFIX -DPYTHON_SITE_PACKAGES_DIR=$SP_DIR -DCMAKE_FIND_ROOT_PATH=$PREFIX -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --target install
