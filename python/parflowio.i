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

%include "parflow/pfdata.hpp"

%extend PFData {
    
    #include "Python.h"
    #include "numpy/arrayobject.h"

    PyObject * getDataAsArray(){        
        npy_intp dims[3] = {0};
        dims[0]=$self->getNZ();
        dims[1]=$self->getNY();
        dims[2]=$self->getNX();
        return PyArray_SimpleNewFromData(3, dims, NPY_DOUBLE, static_cast<void*>($self->getData()));
    }

    void setDataArray(PyObject * pyObjIn){
        PyArrayObject *pyArrayIn;
        pyArrayIn = (PyArrayObject *) PyArray_FromAny(pyObjIn,PyArray_DescrFromType(NPY_DOUBLE),3,3,NPY_ARRAY_INOUT_ARRAY, nullptr);
        npy_intp ind[3] = {0,0,0};
        $self->setData((double *) PyArray_GetPtr(pyArrayIn, ind));
        npy_intp * arr_shape = PyArray_SHAPE(pyArrayIn);
        $self->setNZ(arr_shape[0]);
        $self->setNY(arr_shape[1]);
        $self->setNX(arr_shape[2]);

    }

    PFData(PyObject* pyObj){
        PFData* data = new PFData();
        PyArrayObject* pyArray = (PyArrayObject*) PyArray_FromAny(pyObj, PyArray_DescrFromType(NPY_DOUBLE), 3, 3, NPY_ARRAY_INOUT_ARRAY, nullptr);

        npy_intp ind[3] = {0,0,0};
        data->setData((double*) PyArray_GetPtr(pyArray, ind));
        npy_intp* arr_shape = PyArray_SHAPE(pyArray);
        data->setNZ(arr_shape[0]);
        data->setNY(arr_shape[1]);
        data->setNX(arr_shape[2]);

        return data;
    }

    %pythoncode %{
    def __str__(self):
        s = str(self.__class__.__name__) + "(X={}, Y={}, Z={}, NX={}, NY={}, NZ={}, DX={}, DY={}, DZ={})".format(self
                .getX(), self.getY(), self.getZ(), self.getNX(), self.getNY(), self.getNZ(), self.getDX(), self.getDY(),
                self.getDZ())
        return s
    %}

};
