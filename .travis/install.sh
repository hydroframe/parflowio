#!/usr/bin/env bash
set -x
set -e

function install-cmake() {
  # need CMake >= 3.14 (for using the newly swig built-in UseSWIG module)
  if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    wget https://cmake.org/files/v3.16/cmake-3.16.2.tar.gz
    tar xzf cmake-3.16.2.tar.gz && rm cmake-3.16.2.tar.gz
    cd cmake-3.16.2 && ./bootstrap --prefix=/opt/cmake-3.16.2
    make -j 2
    sudo make install
    cd .. && rm -rf cmake-3.16.2
    export PATH=/opt/cmake-3.16.2/bin:$PATH
    command -v cmake
    cmake --version
  elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    cmake --version
  fi
}

function install-swig() {
  if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    # apt-get only have swig 2.0.11
    # Need SWIG >= 3.0.8
    cd /tmp/
    wget https://github.com/swig/swig/archive/rel-3.0.12.tar.gz
    tar zxf rel-3.0.12.tar.gz
    cd swig-rel-3.0.12
    ./autogen.sh
    ./configure --prefix=/usr
    make -j 2
    sudo make install
  fi
}

function install-python(){
  if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    # work around https://github.com/travis-ci/travis-ci/issues/8363

    pyenv global 3.7
    python --version
    python -m pip --version
    python -m pip install --upgrade pip
    python -m pip --version
    python -m pip install virtualenv wheel six numpy
  elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    python3 -m pip install virtualenv wheel six numpy
  fi
}

eval "${MATRIX_EVAL}"

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  /usr/bin/x86_64-linux-gnu-ld --version
fi

install-cmake
if [[ "$LANGUAGE" != "cpp" ]]; then
  install-swig
fi

if [[ "$LANGUAGE" == "python" ]]; then
  install-python
fi
