ARG BASE_IMAGE=quay.io/pypa/manylinux2010_x86_64
FROM ${BASE_IMAGE}

RUN yum install -y \
  wget \
  pcre-devel

# Build and install the latest cmake
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-Linux-x86_64.sh && \
  bash cmake-3.18.2-Linux-x86_64.sh --skip-license --prefix=/usr/local && \
  rm cmake-3.18.2-Linux-x86_64.sh

# Build and install swig
RUN wget -q https://github.com/swig/swig/archive/v3.0.12.tar.gz && \
  tar -xzf v3.0.12.tar.gz && \
  rm v3.0.12.tar.gz && \
  cd swig-3.0.12 && \
  ./autogen.sh && \
  ./configure && \
  make -j5 && \
  make install && \
  cd .. && \
  rm -r swig-3.0.12
