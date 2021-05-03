/* parflowio.i */
%module parflowio

%{
#define SWIG_FILE_WITH_INIT
#include "parflow/pfdata.hpp"
%}

%include "std_string.i"
%include "std_array.i"
%include "numpy.i"
%include "typemaps.i"
%include "std_vector.i"

%init %{
    import_array();
%}

%apply (double* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {
    (double* data, int nz, int ny, int nx)
}

//Instantiate std::array<int, 3> template
namespace std {
    %template(IntArray3) array<int, 3>;
}

//Mark diffIndex as OUTPUT
%apply std::array<int, 3>* OUTPUT {std::array<int, 3>* diffIndex};
PFData::differenceType PFData::compare(const PFData& otherObj, std::array<int, 3>* diffIndex);

//Ignore this constructor, and replace it with our own down below
%ignore PFData::PFData(double* data, int nz, int ny, int nx);

//Ignore these functions
%ignore PFData::getData();
%ignore PFData::getData() const;
%ignore PFData::setData(double*);

%include "parflow/pfdata.hpp"

%extend PFData {
    #include <cstdlib>
    #include <cstring>

    #include "Python.h"
    #include "numpy/arrayobject.h"

    PFData(PyObject* pyObj){
        PyArrayObject* pyArray = reinterpret_cast<PyArrayObject*>(PyArray_FromAny(pyObj, PyArray_DescrFromType(NPY_DOUBLE), 3, 3, NPY_ARRAY_OUT_ARRAY, nullptr));

        npy_intp* arr_shape = PyArray_SHAPE(pyArray);

        npy_intp ind[3] = {0,0,0};
        return new PFData(static_cast<double*>(PyArray_GetPtr(pyArray, ind)), arr_shape[0], arr_shape[1], arr_shape[2]);
    }

    void setDataArray(PyObject * pyObjIn){
        PyArrayObject* pyArrayIn = reinterpret_cast<PyArrayObject*>(PyArray_FromAny(pyObjIn, PyArray_DescrFromType(NPY_DOUBLE), 3, 3, NPY_ARRAY_OUT_ARRAY, nullptr));

        npy_intp ind[3] = {0,0,0};
        $self->setData(static_cast<double*>(PyArray_GetPtr(pyArrayIn, ind)));
        $self->setIsDataOwner(false);

        npy_intp* arr_shape = PyArray_SHAPE(pyArrayIn);
        $self->setNZ(arr_shape[0]);
        $self->setNY(arr_shape[1]);
        $self->setNX(arr_shape[2]);
    }

    PyObject* moveDataArray(){
        double* data = $self->getData();
        if(!data) return Py_None;

        $self->setData(nullptr);
        $self->setIsDataOwner(false);

        npy_intp strides[3] = {$self->getNZ(), $self->getNY(), $self->getNX()};

        PyObject* pyarray = PyArray_SimpleNewFromData(3, strides, NPY_DOUBLE, data);
        PyArray_ENABLEFLAGS(reinterpret_cast<PyArrayObject*>(pyarray), NPY_ARRAY_OWNDATA);
        return pyarray;
    }

    PyObject* copyDataArray(){
        double* data = $self->getData();
        if(!data) return Py_None;

        const int size = $self->getNZ() * $self->getNY() * $self->getNX();
        double* dataCopy = static_cast<double*>(std::malloc(size * sizeof(double)));
        memcpy(dataCopy, data, size*sizeof(double));    //Note: #include <cstring> doesnt bring in std::memcpy, but does bring in memcpy. ????

        npy_intp strides[3] = {$self->getNZ(), $self->getNY(), $self->getNX()};
        PyObject* pyarray = PyArray_SimpleNewFromData(3, strides, NPY_DOUBLE, dataCopy);
        PyArray_ENABLEFLAGS(reinterpret_cast<PyArrayObject*>(pyarray), NPY_ARRAY_OWNDATA);
        return pyarray;
    }

    PyObject* viewDataArray(){
        double* data = $self->getData();
        if(!data) return Py_None;

        npy_intp strides[3] = {$self->getNZ(), $self->getNY(), $self->getNX()};
        return PyArray_SimpleNewFromData(3, strides, NPY_DOUBLE, data);
    }

    %pythoncode %{
    def __str__(self):
        s = str(self.__class__.__name__) + "(X={}, Y={}, Z={}, NX={}, NY={}, NZ={}, DX={}, DY={}, DZ={}, indexOrder={})".format(
                self.getX(), self.getY(), self.getZ(), self.getNX(), self.getNY(), self.getNZ(), self.getDX(),
                self.getDY(), self.getDZ(), self.getIndexOrder())
        return s
    %}

};

