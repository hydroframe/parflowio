#!/usr/bin/env bash
set -ev

if [[ "$(uname)" == "Darwin" ]]; then
  # Target deployment for OS X 10.9
  export MACOSX_DEPLOYMENT_TARGET=10.9
fi

if [[ "$(uname)" == "Linux" ]]; then
  # We must build using an older toolchain. Use docker to accomplish this.
  docker run --entrypoint=bash --rm -v $PWD:/project $CIBW_MANYLINUX_X86_64_IMAGE -c 'export PATH=/opt/python/cp37-cp37m/bin:$PATH && pip install numpy==1.18.5 && mkdir -p /project/build && cd /project/build && cmake .. -DPACKAGE_TESTS=OFF -DBUILD_PYTHON=ON && make'
else
  # Mac and Windows...
  mkdir -p build
  cd build
  # Set the python executable so it won't assume we are looking
  # for Python 2.
  cmake \
    -DPACKAGE_TESTS=OFF \
    -DBUILD_PYTHON=ON \
    -DPython_EXECUTABLE=$(which python) \
    ..
    cmake --build .
fi
