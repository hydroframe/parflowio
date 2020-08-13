# parflowio
This project is meant to replace the existing pfio code used by pftools.
For now the project is a separate repo, but that will change.

## Installation
    git clone <this repo>
    mkdir parflowio-build
    cd parflowio-build
    cmake ../parflowio
    make
    make test
    make docs
    
This will build a library file (libparflowio.a). To use this library you will also need the header files.

The documentation can be found in docs/html/index.html


