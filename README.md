# parflowio
This project is meant to replace the existing pfio code used by pftools.
For now the project is a separate repo, but that will change.

## Installation
    git clone <this repo>
    mkdir parflowio-build
    cd parflowio-build
    cmake ..
    make
    make test
    make docs
    
This will build a library file (libparflowio.a). To use this library you will also need the header files.

The documentation can be found in docs/html/index.html

## Python Installation
Requires [SWIG](http://www.swig.org/)

After performing the above installation, do the following steps from the parflowio-build directory:
```
    cd python/parflowio
    python setup.py install
```

Notes: Suggest using a conda environment for this. 

`conda create -n parflowio_test ipython numpy pip virtualenv -c conda-forge -y`