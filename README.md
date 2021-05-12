[![Build Status](https://travis-ci.com/hydroframe/parflowio.svg?branch=master)](https://travis-ci.com/hydroframe/parflowio)

# `parflowio`
This project is meant to replace the existing `pfio` code used by `pftools`.
For now the project is a separate repo, but that will change.  
If you just need to use the python package, you can install the latest version with `pip` from PyPi:
```
pip install parflowio
```

## Dependencies
### C++ Library
1. CMake 3.15 or newer
2. A C++11 capable compiler
3. (Optional) Doxygen, for documentation

### Python Package
1. All [C++ dependencies](#c-library)
2. Python 3, and Python 3 development files (generally the `-dev` or `-develop` package).
3. The following python packages: numpy\*, pip, wheel
4. SWIG 3 or newer (SWIG 4+ if Python docs are desired)
5. (Recommended) conda or similar virtual environment.

\* If you installed numpy using your system package manager (like `apt` or `pacman`) you may need to also install the development version of numpy.


macOS users should be able to install SWIG using homebrew with the command:
```
brew install swig
```

SWIG can also be installed with conda when creating the environment.

## Building C++ Library
Clone the repository:
```
git clone git@github.com:hydroframe/parflowio.git
cd parflowio
```
Install the required dependencies.
Generate build files, and build the main target:
```
cmake -S . -B build
cmake --build build
```

This will build a library file (`libparflowio.a`). To use this library you will also need the header files.

To build the docs (requires doxygen):
```
cmake --build build -t docs
```

The documentation can be found in `docs/html/index.html`

## Testing C++ Library
To run the C++ tests:
```
cmake --build build -t test
```

## Building Python Package

Clone the repository, if needed:
```
git clone git@github.com:hydroframe/parflowio.git
cd parflowio
```

If using conda, create a new environment:
```
conda create -n parflowio numpy pip wheel
conda activate parflowio
```

Install the required dependencies. If installing SWIG with conda, ensure the created environment is active and install as shown:
```
conda install swig
```

Build with the python package enabled:
```
cmake -S . -B build -DBUILD_PYTHON=ON
cmake --build build
```

## Installing Python Package
Build the Python package, as detailed in in the [previous section](#building-python-package)  
Install the built package:
```
pip install build/python/parflowio/
```

The package can now be used from python:
```
>>> from parflowio.pyParflowio import PFData
>>> pfdata = PFData('tests/inputs/press.init.pfb')
>>> pfdata.loadHeader()
>>> pfdata.loadData()
>>> data = pfdata.copyDataArray()
```

## Testing Python Package
After [building the python package](#building-python-package) and [installing it](#installing-python-package), `cd` into the `python` directory and run `python test.py`.
