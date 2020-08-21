#!/usr/bin/env bash
set -x
set -e

#################
##  CONFIGURE  ##
#################
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  export PATH=/opt/cmake-3.16.2/bin:$PATH
fi

cmake --version
if [[ "$LANGUAGE" == "cpp" ]]; then
  LDFLAGS=-v cmake -S. -Bbuild
elif [[ "$LANGUAGE" == "python" ]]; then
  python --version
  cmake -S. -Bbuild -DBUILD_PYTHON=ON -DPython_ADDITIONAL_VERSIONS=3.7
fi

#############
##  BUILD  ##
#############
cmake --build build --target all -- VERBOSE=1

############
##  TEST  ##got
############
cmake --build build --target test
# vim: set tw=0 ts=2 sw=2 expandtab:

cd build/python/parflowio && python setup.py install && cd ../../../python && python test.py