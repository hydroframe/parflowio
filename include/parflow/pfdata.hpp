#ifndef PARFLOWIO_PFDATA_HPP
#define PARFLOWIO_PFDATA_HPP
#include <array>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <string>

/**
 * class: PFData
 * The PFData class refers to the contents of ParflowBinary File. This class provides several methods to read
 * and write those files as well as export the data.
 */
class PFData {
private:
    std::string m_filename;
    FILE* m_fp;

    // The following information is available only after the file is opened
    // main header information
    double m_X,m_Y,m_Z;
    int m_nx,m_ny,m_nz;
    double m_dX,m_dY,m_dZ;
    int m_numSubgrids;
    int m_p,m_q,m_r;
    double* m_data;
	/**
	 * writeFile
	 * @param string filename
	 * @return int 
	 */
    int writeFile(const std::string filename, std::vector<long> &byte_offsets);

public:

    /**
     * PFData
     * Default constructor, useful when storing data that may be written later
     */
    PFData();

    /**
     * PFData
     * @param const char* filename, a relative filename (this file may or may not exist)
     * The validity of this file is not checked at  this point. Only during the first
     * interaction.
     */
    explicit PFData(std::string);

    /**
     * PFData
     * @param data array of doubles,
     * @param nx number of elements in x dimension
     * @param ny number of elements in y dimension
     * @param nz number of elements in z direction
     * This constructor is useful if you have an existing data array you want to make into a PFB file.
     */
    PFData(double * data, int nz, int ny, int nx);

    /**
     * loadHeader
     * @retval 0 on success, non 0 on failure (sets errno)
     * This function reads the header of the pfb file, but does not read the data.
     */
    int loadHeader();
    std::string getFilename() const;

    /**
     * loadData
     * @retval 0 on success, non-0 on failure
     * This function reads all of the data from the pfb file into memory.
     */
     int loadData();
	 /**
	  * writeFile
	  * @param string filenamee
	  * @return int
	  */
     int writeFile(std::string filename);
	 /**
	  * writeFile
	  * @param empty
	  * @reutrn int
	  */

     int writeFile();
	 /**
	  * distFile
	  * @param int P
	  * @param int Q
	  * @param int R
	  * @param string outFile
	  * @return int
	  */
     int distFile(int P, int Q, int R, std::string outFile);

    //Used for the compare function
    enum class differenceType {none=0, z, y, x, dZ, dY, dX, nZ, nY, nX, data};

    /** Compares `this` and another PFData object. Comparison is based on `Z`, `Y`, `X`, `DZ`, `DY`, `DX`, and the data.
     * Note: The behavior is undefined if `this` or `otherObj` is not fully initialized. The return value corresponds to the first difference found.
     * \param[in]   otherObj        Other object to compare with.
     * \param[out]  diffIndex       Unused if it is `nullptr` and/or the return values are not `data` Otherwise contains the location where the first difference occurs.
     * \retval      none            The objects are the same
     * \retval      z               The `Z` values are different
     * \retval      y               The `Y` values are different
     * \retval      x               The `X` values are different
     * \retval      dZ              The `DZ` values are different
     * \retval      dY              The `DY` values are different
     * \retval      dX              The `DX` values are different
     * \retval      nZ              The `NZ` values are different (data dimensions)
     * \retval      nY              The `NY` values are different
     * \retval      nX              The `NX` values are different
     * \retval      data            The data dimensions are the same, but the values are different. If `diffIndex` is non-null, it will contain the `ZYX` data location  where the (first) difference occurred.
    */
    differenceType compare(const PFData& otherObj, std::array<int, 3>* diffIndex) const;

    /** Given a flattened index into `data`, unflatten it into its `ZYX` components.
     * Note: The behavior is undefined the dimension data is not fully initialized.
     * \param[int]  index           The flattened index of the `data`
     * \retval      {Z,Y,X}         Corresponding `ZYX` of the valid `index`
     * \retval      {-1,-1,-1}      Invalid index @@TODO want?
     */
    std::array<int, 3> unflattenIndex(int index) const;

    /**
     * get[X,Y,Z]
     * [X,Y,Z] is the lower left corner of the Computational Grid.
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns double
     */

	/**
	 * @param double get[X,Y,Z] [X] is the lower left corner of the Computational Grid. 
	 * This function is useful either when reading an existing file or when confirming the configuration 
	 * of a file that is being created where the computational grid has already been set.
	 * @return double
	 */
    double getX() const;
	/** @param double  get[X,Y,Z] [Y] is the lower left corner of the Computational Grid. 
	 * This function is useful either when reading an existing file or when confirming the configuration 
	 * of a file that is being created where the computational grid has already been set.
	 * @return double
	 */
    double getY() const;
	/** @param double get[X,Y,Z] [Z] is the lower left corner of the Computational Grid.
	 * This function is useful either when reading an existing file or when
	 * confirming the configuration of a file that is being created where the
	 * computational grid has already been set.
	 * @return double
	 */
    double getZ() const;

    /**
    * get[P,Q,R]
     * [P,Q,R] define processor topology for blocking the file
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns int
     */
	int getP() const;
	int getQ() const;
	int getR() const;

    /**
     * set[P,Q,R]
     * @param int [P,Q,R]
     * [P,Q,R] define processor topology for blocking the file
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */
    void setP(int P);
    void setQ(int Q);
    void setR(int R);

    /**
     * set[X,Y,Z]
     * @param double [X,Y,Z]
     * [X,Y,Z] is the lower left corner of the Computational Grid.
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */

	/** @param double [X,Y,Z] [X] is the lower left corner of the Computational
	 * Grid. This function is useful when creating a new pfb file. It is
	 * important to note that you can call this function on an existing file,
	 * but it will invalidate the file and break all subsequent uses of the
	 * class unless you call load_file() again to reset the value back to the
	 * one used in the file.
	 */
    void setX(double X);
	/** @param double [X,Y,Z] [Y] is the lower left corner of the Computational
	 * Grid. This function is useful when creating a new pfb file. It is
	 * important to note that you can call this function on an existing file,
	 * but it will invalidate the file and break all subsequent uses of the
	 * class unless you call load_file() again to reset the value back to the
	 * one used in the file.
	 */
    void setY(double Y);
	/** @param double [X,Y,Z] [Z] is the lower left corner of the Computational
	 * Grid. This function is useful when creating a new pfb file. It is
	 * important to note that you can call this function on an existing file,
	 * but it will invalidate the file and break all subsequent uses of the
	 * class unless you call load_file() again to reset the value back to the
	 * one used in the file.
	 */
    void setZ(double Z);

    /**
     * getN[X,Y,Z]
     * [NX,NY,NZ] describe the dimensions of the computational domain.
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns int
     */

	/** @param getN[X,Y,Z] [NX] describe the dimensions of the
	 * computational domain. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return int
	 */
    int getNX() const;
	/** @param getN[X,Y,Z] [NY] describe the dimensions of the
	 * computational domain. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return int
	 */
    int getNY() const;
	/** @param getN[X,Y,Z] [NZ] describe the dimensions of the
	 * computational domain. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return int
	 */
    int getNZ() const;

    /**
     * setN[X,Y,Z]
     * @param double N[X,Y,Z]
     * [NX,NY,NZ] describes the dimensions of the computational grid.
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */

	/** @param double [NX,NY,NZ] [NX] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setNX(int NX);
	/** @param double [NX,NY,NZ] [NY] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setNY(int NY);
	/** @param double [NX,NY,NZ] [NZ] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setNZ(int NZ);

    /**
     * getD[X,Y,Z]
     * [DX,DY,DZ] describe the TODO: the what?
     * This function is useful either when reading an existing file or when confirming the configuration
     * of a file that is being created where the computational grid has already been set.
     * @returns int
     */

	/** param get[DX,DY,DZ] [DX] describe the TODO: the what? This function
	 * is useful either when reading an existing file or when confirming the
	 * configuration of a file that is being created where the computational
	 * grid has already been set.
	 * @return double
	 */
    double getDX() const;
	/** @param get[DX,DY,DZ] [DY] describe the TODO: the what? This function
	 * is useful either when reading an existing file or when confirming the
	 * configuration of a file that is being created where the computational
	 * grid has already been set.
	 * @return double
	 */
    double getDY() const;
	/** @param get[DX,DY,DZ] [DZ] describe the TODO: the what? This function
	 * is useful either when reading an existing file or when confirming the
	 * configuration of a file that is being created where the computational
	 * grid has already been set.
	 * @return double
	 */
    double getDZ() const;

    /**
     * setD[X,Y,Z]
     * @param double D[X,Y,Z]
     * [DX,DY,DZ] describes the TODO: the what?
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */

	/** @param  double[DX,DY,DZ] [DX] describes the TODO: the what? This
	 * function is useful when creating a new pfb file. It is important to note
	 * that you can call this function on an existing file, but it will
	 * invalidate the file and break all subsequent uses of the class unless you
	 * call load_file() again to reset the value back to the one used in the
	 * file.
	 */
    void setDX(double DX);
	/** @param double [DX,DY,DZ] [DY] describes the TODO: the what? This
	 * function is useful when creating a new pfb file. It is important to note
	 * that you can call this function on an existing file, but it will
	 * invalidate the file and break all subsequent uses of the class unless you
	 * call load_file() again to reset the value back to the one used in the
	 * file.
	 */
    void setDY(double DY);
	/** @param double [DX,DY,DZ] [DZ] describes the TODO: the what? This
	 * function is useful when creating a new pfb file. It is important to note
	 * that you can call this function on an existing file, but it will
	 * invalidate the file and break all subsequent uses of the class unless you
	 * call load_file() again to reset the value back to the one used in the
	 * file.
	 */
    void setDZ(double DZ);

	/**
	 * getNumSubgrids() const
	 * @param empty
	 * @return no return 
	 */

    int getNumSubgrids() const;

	/** setNumSubgrids
	 * @param int mNumSubgrids
	 * @return no return
	 */

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

	/**
	 * operator
	 * @param int
	 * @param int
	 * @param int
	 * @return double
	 */
    double operator()(int,int,int);
	/**
	 * getSubgridData
	 * @param int grid
	 */
    double* getSubgridData(int grid);

    /**
     * getData
     * @return std::vector<double>*
     * Get a pointer to the raw data as a one dimensional array.
     */
    double* getData();

    /**
	 * see getData() 
	 * @param empty
	 */
    const double* getData() const;
	/**
	 * setData
	 * @param double*
	 * @return void
	 */
    void  setData(double*);
	/**
	 * close
	 * @param empty
	 * @return empty
	 */
    void close();


};
/**
 * calcOffset
 * @param int extent
 * @param int block_count
 * @param int block_idx
 */
int calcOffset(int extent, int block_count, int block_idx);
/**
 * calcExtent
 * @param int extent 
 * @param int block_count
 * @param int block_idx
 * @return int
 */
 
int calcExtent(int extent, int block_count, int block_idx);


#endif //PARFLOWIO_PFDATA_HPP
