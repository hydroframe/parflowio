#ifndef PARFLOWIO_PFDATA_HPP
#define PARFLOWIO_PFDATA_HPP
#include <cstddef>
#include <stdio.h>
#include <vector>

/**
 * class: PFData
 * The PFData class refers to the contents of ParflowBinary File. This class provides several methods to read
 * and write those files as well as export the data.
 */
class PFData {
private:
    const char* m_filename;
    FILE* m_fp;

    // The following information is available only after the file is opened
    // main header information
    double m_X,m_Y,m_Z;
    int m_nx,m_ny,m_nz;
    double m_dX,m_dY,m_dZ;
    int m_numSubgrids;
    int m_p,m_q,m_r;
    double* m_data;

public:

    /**
     * PFData
     * @param const char* filename, a relative filename (this file may or may not exist)
     * The validity of this file is not checked at  this point. Only during the first
     * interaction.
     */
    PFData(const char*);
    /**
     * loadHeader
     * @retval 0 on success, non 0 on failure (sets errno)
     * This function reads the header of the pfb file, but does not read the data.
     */
    int loadHeader();
    const char * getFilename();

    /**
     * loadData
     * @retval 0 on success, non-0 on failure
     * This function reads all of the data from the pfb file into memory.
     */
     int loadData();
     int writeFile(const char* filename);
     int writeFile();
     int distFile(int P, int Q, int R, const char* outFile);

    /**
     * get[X,Y,Z]
     * [X,Y,Z] is the lower left corner of the Computational Grid.
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns double
     */
    double getX() const;
    double getY() const;
    double getZ() const;


    /**
     * set[X,Y,Z]
     * @param double [X,Y,Z]
     * [X,Y,Z] is the lower left corner of the Computational Grid.
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */
    void setX(double X);
    void setY(double Y);
    void setZ(double Z);



    /**
     * getN[X,Y,Z]
     * [NX,NY,NZ] describe the dimensions of the computational domain.
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns int
     */
    int getNX() const;
    int getNY() const;
    int getNZ() const;

    /**
     * setN[X,Y,Z]
     * @param double N[X,Y,Z]
     * [NX,NY,NZ] describes the dimensions of the computational grid.
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */
    void setNX(int NX);
    void setNY(int NY);
    void setNZ(int NZ);

    /**
     * getD[X,Y,Z]
     * [DX,DY,DZ] describe the TODO: the what?
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns int
     */
    double getDX() const;
    double getDY() const;
    double getDZ() const;

    /**
     * setD[X,Y,Z]
     * @param double D[X,Y,Z]
     * [DX,DY,DZ] describes the TODO: the what?
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */
    void setDX(double DX);
    void setDY(double DY);
    void setDZ(double DZ);

    int getNumSubgrids() const;

    void setNumSubgrids(int mNumSubgrids);

    /**
     * getCoordinateDatum
     * @param int x
     * @param int y
     * @param int z
     * @param double* value
     * @returns 0 on success, non 0 on failure
     * These coordinates are an offset from the lower left hand corner.
     */
    int getCoordinateDatum(int x,int y,int z, double* value);
    double operator()(int,int,int);
    double* getSubgridData(int grid);

    /**
     * getData
     * @return std::vector<double>*
     * Get a pointer to the raw data as a one dimensional array.
     */
    double* getData();
    void  setData(double*);

    void close();


};
int calcOffset(int extent, int block_count, int block_idx);
int calcExtent(int extent, int block_count, int block_idx);


#endif //PARFLOWIO_PFDATA_HPP
