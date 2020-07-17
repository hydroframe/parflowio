#ifndef PARFLOWIO_PFDATA_HPP
#define PARFLOWIO_PFDATA_HPP
#include <stdio.h>

/**
 * class: PFData
 * The PFData class refers to the contents of ParflowBinary File. This class provides several methods to read
 * and write those files as well as export the data.
 */
class PFData {
private:
    const char* _filename;
    FILE* _fp;

    // The following information is available only after the file is opened
    // main header information
    double _X,_Y,_Z;
    int _nx,_ny,_nz;
    double _dX,_dY,_dZ;
    int _numSubgrids;
    double* data;

public:
    /**
     * PFData
     * Default constructor, useful when storing data that may be written later
     */
    PFData();
    /**
     * PFData
     * @param const char* filename, a relative filename (this file may or may not exist)
     */
    PFData(const char*);
    /**
     * loadFile
     * @retval 0 on success, non 0 on failure (sets errno)
     */
    int loadFile();

};

class PFSubgrid {
    int ix,iy,iz;
    int nx,ny,nz;
    int rx,ry,rz;

    double *data;

    PFSubgrid(FILE* file);
};
#endif //PARFLOWIO_PFDATA_HPP
