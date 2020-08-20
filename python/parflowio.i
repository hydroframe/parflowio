/* parflowio.i */
%module parflowio

%{
#define SWIG_FILE_WITH_INIT
#include "parflow/pfdata.hpp"
%}

%include "std_string.i"
%include "std_list.i"
%include "std_map.i"
%include "std_set.i"
%include "std_vector.i"

%include "numpy.i"

%init %{
    import_array();
%}

%apply (double* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {
    (double* data, int nz, int ny, int nx)
}

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
		pyArrayIn = (PyArrayObject *) PyArray_FromAny(pyObjIn, PyArray_DescrFromType(NPY_DOUBLE), 3, 3,
												NPY_ARRAY_IN_ARRAY, NULL);
    	npy_intp ind[3] = {0,0,0};
    	$self->setData((double *) PyArray_GetPtr(pyArrayIn, ind));
    	npy_intp * shape = PyArray_SHAPE(pyArrayIn);
    	$self->setNZ(shape[0]);
    	$self->setNY(shape[1]);
    	$self->setNX(shape[2]);

    }

    %pythoncode %{
    def __str__(self):
        s = str(self.__class__.__name__) + "(X={}, Y={}, Z={}, NX={}, NY={}, NZ={}, DX={}, DY={}, DZ={})".format(self
        		.getX(), self.getY(), self.getZ(), self.getNX(), self.getNY(), self.getNZ(), self.getDX(), self.getDY(),
        		self.getDZ())
        return s
	%}

};
