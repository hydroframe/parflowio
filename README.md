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
Required packages:

* [SWIG](http://www.swig.org/)
* Numpy
* pip
* virtualenv

After performing the steps from [installation](#Installation), do the following steps from the parflowio-build
 directory:
```
    cd python/parflowio
    python setup.py install
```

Note: Suggest using a conda or other non-system environment for this. 

A new conda environment and activate it:

```
$ conda create -n parflowio_test ipython numpy pip virtualenv swig -c conda-forge -y`
$ conda activate parflowio_test`
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
