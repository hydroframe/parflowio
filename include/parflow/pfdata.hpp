#ifndef PARFLOWIO_PFDATA_HPP
#define PARFLOWIO_PFDATA_HPP
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

/**
 * class: PFData
 * The PFData class refers to the contents of ParflowBinary File. This class provides several methods to read
 * and write those files as well as export the data.
 */
class PFData {
private:
    std::string m_filename;
    std::FILE* m_fp = nullptr;

    // The following information is available only after the file is opened
    // main header information
    double m_X = 0.0;
    double m_Y = 0.0;
    double m_Z = 0.0;

    int m_nx = 0;
    int m_ny = 0;
    int m_nz = 0;

    double m_dX = 1.0;
    double m_dY = 1.0;
    double m_dZ = 1.0;

    int m_numSubgrids{};
    int m_p = 1;
    int m_q = 1;
    int m_r = 1;

    //Tracks if we own m_data, and need to free it.
    bool m_dataOwner = false;

    double* m_data = nullptr;

	/**
	 * writeFile
	 * @param string filename
	 * @return int 
	 */
    int writeFile(const std::string filename, std::vector<long> &byte_offsets);

    /** Given a target subgrid, returns the absolute offset from the start of the file to the beginning of the target subgrid header.
     * \pre             loadHeader() and loadPQR()
     * \param gridX     The X index of the target subgrid
     * \param gridY     The Y index of the target subgrid
     * \param gridZ     The Z index of the target subgrid
     * \return          The absolute offset from the beginning of the file, to the start of the target subgrid header.
     */
    long getSubgridOffset(int gridX, int gridY, int gridZ) const;

    /** Given a target subgrid, returns the index of the first element of that grid in the pfb file. Effectively returns the number of elements between the start of the file(0) and the subgrid in the file.
     * \pre             loadHeader() and loadPQR()
     * \param gridX     The X index of the target subgrid
     * \param gridY     The Y index of the target subgrid
     * \param gridZ     The Z index of the target subgrid
     * \return          The number of elements before the target subgrid.
     */
    long getSubgridOffsetElements(int gridX, int gridY, int gridZ) const;


    /** Given a target point, returns the absolute offset from the start of the file to the specified point.
     * \pre             loadHeader() and loadPQR()
     * \param x         The x index of the target point
     * \param y         The y index of the target point
     * \param z         The z index of the target point
     * \return          The absolute offset from the start of the file to the point.
     */
    long getPointOffset(int x, int y, int z) const;

    /** Read in the subgrid at the specified subgrid index.
     * \pre             loadHeader() and loadPQR()
     * \param           buffer  Pointer to an array of size: getSubgridSizeX(gridX) * getSubgridSizeY(gridY) * getSubgridSizeZ(gridZ), 1d
     * \param   fp      File pointer to use. No requirement on current position, as it will seek to the appropriate location. If no error has occurred, fp will point after the last element read.
     * \param   gridX   The X index of the subgrid to read.
     * \param   gridY   The Y index of the subgrid to read
     * \param   gridZ   The Z index of the subgrid to read.
     * \return          0 if success, non-zero if error.
     */
    int fileReadSubgridAtGridIndexInternal(double* buffer, std::FILE* fp, int gridX, int gridY, int gridZ) const;

    /** Reads in the subgrid from the file pointer, emplacing it into the m_data array.
     * \pre             loadHeader() and loadPQR()
     * \param   fp      The file pointer to use.
     * \param   gridX   X index of the target subgrid
     * \param   gridY   Y index of the target subgrid
     * \param   gridZ   Z index of the target subgrid
     * \return          0 if success, non-zero on error.
     */
    int emplaceSubgridFromFile(std::FILE* fp, int gridX, int gridY, int gridZ);

public:

    /**
     * PFData
     * Default constructor, useful when storing data that may be written later
     */
    PFData() = default;

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
    PFData(double * data, int nx, int ny, int nz);

    //Closes the file descriptor, if open. If we own the backing data memory, it is freed.
    ~PFData();

    /** Read a single point from the file, without loading it all into memory.
     * \pre             loadHeader() and loadPQR()
     * \param   x       X index of the point
     * \param   y       Y index of the point
     * \param   z       Z index of the point
     * \return          Value of the data at the specified point.
     */
    double fileReadPoint(int x, int y, int z);

    /** Read in the subgrid containing the specified point from the file.
     * \pre         loadHeader() and loadPQR()
     * \param   x   X index of the point inside the desired subgrid.
     * \param   y   Y index of the point inside the desired subgrid.
     * \param   z   Z index of the point inside the desired subgrid.
     * \return      The subgrid containing the specified point.
     */
    std::vector<double> fileReadSubgridAtPointIndex(int x, int y, int z);

    /** Read in the subgrid at the specified subgrid index. Note that this is different than the point indicies.
     * \pre             loadHeader() and loadPQR()
     * \param   gridX   The X index of the subgrid. 
     * \param   gridY   The Y index of the subgrid. 
     * \param   gridZ   The Z index of the subgrid. 
     * \return          Value of the subgrid at the specified index.
     */
    std::vector<double> fileReadSubgridAtGridIndex(int gridX, int gridY, int gridZ);


    /** Returns the X subgrid index of the point at the specified X index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The X index of the point.
     * \return      The X index of the subgrid containing the point.
     */
    int getSubgridIndexX(int idx) const;

    /** Returns the Y subgrid index of the point at the specified Y index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Y index of the point.
     * \return      The Y index of the subgrid containing the point.
     */
    int getSubgridIndexY(int idx) const;

    /** Returns the Z subgrid index of the point at the specified Z index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Z index of the point.
     * \return      The Z index of the subgrid containing the point.
     */
    int getSubgridIndexZ(int idx) const;


    /** Returns the size in the X direction of the subgrid at the specified X index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The X index of the target subgrid.
     * \return      The X size of the subgrid at the specified index.
     */
    int getSubgridSizeX(int idx) const;

    /** Returns the size in the Y direction of the subgrid at the specified Y index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Y index of the target subgrid.
     * \return      The Y size of the subgrid at the specified index.
     */
    int getSubgridSizeY(int idx) const;

    /** Returns the size in the Z direction of the subgrid at the specified Z index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Z index of the target subgrid.
     * \return      The Z size of the subgrid at the specified index.
     */
    int getSubgridSizeZ(int idx) const;


    /**Returns the starting X index of the specified subgrid.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The X subgrid index of the specified subgrid.
     * \return      The first X index of the subgrid.
     */
    int getSubgridStartX(int gridIdx) const;

    /**\See getSubgridStartX
     */
    int getSubgridStartY(int gridIdx) const;

    /**\See getSubgridStartX
     */
    int getSubgridStartZ(int gridIdx) const;


    /** Returns the size in the X direction of a normal block.
     * \pre     loadHeader() and loadPQR()
     * \return  X size of a normal block.
     */
    int getNormalBlockSizeX() const;

    /**\See getNormalBlockSizeX
     */
    int getNormalBlockSizeY() const;

    /**\See getNormalBlockSizeX
     */
    int getNormalBlockSizeZ() const;


    /** Returns the index where normal blocks begin(inclusive).
     * \pre     loadHeader() and loadPQR()
     * \return  Zero if there are no remainder blocks.
     */
    int getNormalBlockStartX() const;

    /**\See getNormalBlockStartX
     */
    int getNormalBlockStartY() const;

    /**\See getNormalBlockStartX
     */
    int getNormalBlockStartZ() const;

    /** Similar to getNormalBlockStartX, but returns the grid index instead of the point index.
     * \pre     loadHeader() and loadPQR()
     * \return  The grid index where normal blocks begin (inclusive)
     */
    int getNormalBlockStartGridX() const;

    /**\See getNormalBlockStartGridX
     */
    int getNormalBlockStartGridY() const;

    /** \See getNormalBlockStartGridX
     */
    int getNormalBlockStartGridZ() const;


    /**
     * loadHeader
     * @retval 0 on success, non 0 on failure (sets errno)
     * This function reads the header of the pfb file, but does not read the data.
     */
    int loadHeader();


    /** This function loads the subgrid headers in order to calculate PQR. The only way to do this is by reading all of the subgrids and counting them, so this function incurs an performance penalty proportional to seeking and reading each subgrid header. 
     * \pre     loadHeader() must have been previously called.
     * \return  0 on success. Other values indicate an error.
     */
    int loadPQR();

    std::string getFilename() const;

    /**
     * loadData
     * @retval 0 on success, non-0 on failure
     * This function reads all of the data from the pfb file into memory.
     */
     int loadData();

     /**
      * Performs the same functionality as loadData(), but loads the file in parallel, using the supplied number of threads.
      * \pre                loadPQR(). @@TODO maybe we want a way to enforce this in the future.
      * \param  numThreads  The number of threads to use, must be at least one.
      * \return             0 if success, non-zero if error.
      */
     int loadDataThreaded(int numThreads);

	 /**
	  * writeFile
	  * @param string filenamee
	  * @return int
	  */
     int writeFile(std::string filename);

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
    enum class differenceType {none=0, x, y, z, dX, dY, dZ, nX, nY, nZ, data};

    /** Compares `this` and another PFData object. Comparison is based on `X`, `Y`, `Z`, `DX`, `DY`, `DZ`, and the data.
     * Note: The behavior is undefined if `this` or `otherObj` is not fully initialized. The return value corresponds to the first difference found.
     * \param[in]   otherObj        Other object to compare with.
     * \param[out]  diffIndex       Unused if it is `nullptr` and/or the return values are not `data` Otherwise contains the location where the first difference occurs.
     * \retval      none            The objects are the same
     * \retval      x               The `X` values are different
     * \retval      y               The `Y` values are different
     * \retval      z               The `Z` values are different
     * \retval      dX              The `DX` values are different
     * \retval      dY              The `DY` values are different
     * \retval      dZ              The `DZ` values are different
     * \retval      nX              The `NX` values are different (data dimensions)
     * \retval      nY              The `NY` values are different
     * \retval      nZ              The `NZ` values are different
     * \retval      data            The data dimensions are the same, but the values are different. If `diffIndex` is non-null, it will contain the `XYZ` data location  where the (first) difference occurred.
    */
    differenceType compare(const PFData& otherObj, std::array<int, 3>* diffIndex) const;

    /** Given a flattened index into `data`, unflatten it into its `XYZ` components.
     * Note: The behavior is undefined if the dimension data is not fully initialized.
     * \param[int]  index           The flattened index of the `data`
     * \retval      {X,Y,Z}         Corresponding `XYZ` of the valid `index`
     * \retval      {-1,-1,-1}      Invalid index @@TODO want?
     */
    std::array<int, 3> unflattenIndex(int index) const;

    /** Given a flattened subgrid index (value in range [0, getNumSubgrids()), unflatten it.
     * \pre         loadHeader() and loadPQR()
     * \param       index           Subgrid index, [0, getNumSubgrids())
     * \retval      {X,Y,Z}         The XYZ indicies of the subgrid.
     * \retval      {-1, -1, -1}    Invalid index
     * @@TEST
     */
    std::array<int, 3> unflattenGridIndex(int index) const;

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

    /**Sets if the class owns the backing data or not. Mostly provided for compatibility with SWIG.
     * \param   isOwner     True if the class should free the data upon destruction, false otherwise.
     */
    void setIsDataOwner(bool isOwner);


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
