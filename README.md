[![Build Status](https://travis-ci.com/hydroframe/parflowio.svg?branch=master)](https://travis-ci.com/hydroframe/parflowio)

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


## Building Python Package

From the parflowio-build directory, created in [installation](#Installation) step:
```
cmake ../parflowio -DBUILD_PYTHON=ON
make
```

## Python Installation
Required packages:

* [SWIG](http://www.swig.org/)
* Numpy
* pip
* virtualenv

macOS users should be able to install SWIG using homebrew with the command:
```
brew install swig
```

After performing the steps from [installation](#Installation), do the following steps from the parflowio-build
 directory:
```
$ cd python/parflowio
$ python setup.py install
```

Note: Suggest using a conda or other non-system environment for this. 

Create a new conda environment and activate it:

```
$ conda create -n parflowio_test ipython numpy pip virtualenv swig pydot -y
$ conda activate parflowio_test
```

Start an ipython session and load the PFData class
```
$ ipython
In [1]: from parflowio.pyParflowio import PFData
In [2]: pfb_data = PFData('<path>/parflowio/tests/inputs/press.init.pfb')
In [3]: pfb_data.loadHeader()
In [4]: pfb_data.loadData()
In [5]: data = pfb_data.getDataAsArray()
```

## Testing Python Module

After following [install](#Installation) and [python install](#Python-Installation) steps above, cd to
parflowio/python and run `python test.py` 