/* parflowio.i */
%module parflowio

%include "std_string.i"
%include "std_list.i"
%include "std_map.i"
%include "std_set.i"
%include "std_vector.i"

%{
	#define SWIG_FILE_WITH_INIT
	#include "parflow/pfdata.hpp"
%} 

%include "numpy.i"
%include "parflow/pfdata.hpp"
%include <typemaps.i>

%init %{
	import_array();
%}

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

    %pythoncode %{
    def __str__(self):
        s = str(self.__class__.__name__) +
        		"(X={}, Y={}, Z={}, NX={}, NY={}, NZ={}, DX={}, DY={}, DZ={})".format(self.getX(),
																				self.getY(), self.getZ(),
																				self.getNX(), self.getNY(),
																				self.getNZ(), self.getDX(),
																				self.getDY(), self.getDZ())
        return s
	%}

};
