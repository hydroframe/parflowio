#!/usr/bin/env bash
set -ev

# We must install numpy for each wheel...
pip install numpy==1.19.4

if [[ "$(uname)" != "Linux" ]] && [[ "$(uname)" != "Darwin" ]]; then
  # Windows links to the python libraries, so we need to re-build
  # each time with the current version of python.
  rm -rf build
  mkdir -p build
  cd build
  cmake \
    -DPACKAGE_TESTS=OFF \
    -DBUILD_PYTHON=ON \
    -DPython_EXECUTABLE=$(which python) \
    ..
  cmake --build . --config Release
fi
