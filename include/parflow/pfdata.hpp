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
    double m_Z = 0.0;
    double m_Y = 0.0;
    double m_X = 0.0;

    int m_nz = 0;
    int m_ny = 0;
    int m_nx = 0;

    double m_dZ = 1.0;
    double m_dY = 1.0;
    double m_dX = 1.0;

    int m_numSubgrids{};
    int m_r = 1;
    int m_q = 1;
    int m_p = 1;

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
     * \param gridZ     The Z index of the target subgrid
     * \param gridY     The Y index of the target subgrid
     * \param gridX     The X index of the target subgrid
     * \return          The absolute offset from the beginning of the file, to the start of the target subgrid header.
     */
    long getSubgridOffset(int gridZ, int gridY, int gridX) const;

    /** Given a target subgrid, returns the index of the first element of that grid in the pfb file. Effectively returns the number of elements between the start of the file(0) and the subgrid in the file.
     * \pre             loadHeader() and loadPQR()
     * \param gridZ     The Z index of the target subgrid
     * \param gridY     The Y index of the target subgrid
     * \param gridX     The X index of the target subgrid
     * \return          The number of elements before the target subgrid.
     */
    long getSubgridOffsetElements(int gridZ, int gridY, int gridX) const;


    /** Given a target point, returns the absolute offset from the start of the file to the specified point.
     * \pre             loadHeader() and loadPQR()
     * \param z         The z index of the target point
     * \param y         The y index of the target point
     * \param x         The x index of the target point
     * \return          The absolute offset from the start of the file to the point.
     */
    long getPointOffset(int z, int y, int x) const;

    /** Read in the subgrid at the specified subgrid index.
     * \pre             loadHeader() and loadPQR()
     * \param           buffer  Pointer to an array of size: getSubgridSizeX(gridX) * getSubgridSizeY(gridY) * getSubgridSizeZ(gridZ), 1d
     * \param   fp      File pointer to use. No requirement on current position, as it will seek to the appropriate location. If no error has occurred, fp will point after the last element read.
     * \param   gridZ   The Z index of the subgrid to read.
     * \param   gridY   The Y index of the subgrid to read
     * \param   gridX   The X index of the subgrid to read.
     * \return          0 if success, non-zero if error.
     */
    int fileReadSubgridAtGridIndexInternal(double* buffer, std::FILE* fp, int gridZ, int gridY, int gridX) const;

    /** Reads in the subgrid from the file pointer, emplacing it into the m_data array.
     * \pre             loadHeader() and loadPQR()
     * \param   fp      The file pointer to use.
     * \param   gridZ   Z index of the target subgrid
     * \param   gridY   Y index of the target subgrid
     * \param   gridX   X index of the target subgrid
     * \return          0 if success, non-zero on error.
     */
    int emplaceSubgridFromFile(std::FILE* fp, int gridZ, int gridY, int gridX);

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
     * @param nz number of elements in z direction
     * @param ny number of elements in y dimension
     * @param nx number of elements in x dimension
     * This constructor is useful if you have an existing data array you want to make into a PFB file.
     */
    PFData(double * data, int nz, int ny, int nx);

    //Closes the file descriptor, if open. If we own the backing data memory, it is freed.
    ~PFData();

    /** Read a single point from the file, without loading it all into memory.
     * \pre             loadHeader() and loadPQR()
     * \param   z       Z index of the point
     * \param   y       Y index of the point
     * \param   x       X index of the point
     * \return          Value of the data at the specified point.
     */
    double fileReadPoint(int z, int y, int x);

    /** Read in the subgrid containing the specified point from the file.
     * \pre         loadHeader() and loadPQR()
     * \param   z   Z index of the point inside the desired subgrid.
     * \param   y   Y index of the point inside the desired subgrid.
     * \param   x   X index of the point inside the desired subgrid.
     * \return      The subgrid containing the specified point.
     */
    std::vector<double> fileReadSubgridAtPointIndex(int z, int y, int x);

    /** Read in the subgrid at the specified subgrid index. Note that this is different than the point indicies.
     * \pre             loadHeader() and loadPQR()
     * \param   gridZ   The Z index of the subgrid.
     * \param   gridY   The Y index of the subgrid.
     * \param   gridX   The X index of the subgrid.
     * \return          Value of the subgrid at the specified index.
     */
    std::vector<double> fileReadSubgridAtGridIndex(int gridZ, int gridY, int gridX);


    /** Returns the Z subgrid index of the point at the specified Z index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Z index of the point.
     * \return      The Z index of the subgrid containing the point.
     */
    int getSubgridIndexZ(int idx) const;

    /** Returns the Y subgrid index of the point at the specified Y index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Y index of the point.
     * \return      The Y index of the subgrid containing the point.
     */
    int getSubgridIndexY(int idx) const;

    /** Returns the X subgrid index of the point at the specified X index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The X index of the point.
     * \return      The X index of the subgrid containing the point.
     */
    int getSubgridIndexX(int idx) const;


    /** Returns the size in the Z direction of the subgrid at the specified Z index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Z index of the target subgrid.
     * \return      The Z size of the subgrid at the specified index.
     */
    int getSubgridSizeZ(int idx) const;

    /** Returns the size in the Y direction of the subgrid at the specified Y index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Y index of the target subgrid.
     * \return      The Y size of the subgrid at the specified index.
     */
    int getSubgridSizeY(int idx) const;

    /** Returns the size in the X direction of the subgrid at the specified X index.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The X index of the target subgrid.
     * \return      The X size of the subgrid at the specified index.
     */
    int getSubgridSizeX(int idx) const;


    /**Returns the starting Z index of the specified subgrid.
     * \pre         loadHeader() and loadPQR()
     * \param idx   The Z subgrid index of the specified subgrid.
     * \return      The first Z index of the subgrid.
     */
    int getSubgridStartZ(int gridIdx) const;

    /**\See getSubgridStartZ
     */
    int getSubgridStartY(int gridIdx) const;

    /**\See getSubgridStartZ
     */
    int getSubgridStartX(int gridIdx) const;


    /** Returns the size in the Z direction of a normal block.
     * \pre     loadHeader() and loadPQR()
     * \return  Z size of a normal block.
     */
    int getNormalBlockSizeZ() const;

    /**\See getNormalBlockSizeZ
     */
    int getNormalBlockSizeY() const;

    /**\See getNormalBlockSizeZ
     */
    int getNormalBlockSizeX() const;


    /** Returns the index where normal blocks begin(inclusive).
     * \pre     loadHeader() and loadPQR()
     * \return  Zero if there are no remainder blocks.
     */
    int getNormalBlockStartZ() const;

    /**\See getNormalBlockStartZ
     */
    int getNormalBlockStartY() const;

    /**\See getNormalBlockStartZ
     */
    int getNormalBlockStartX() const;

    /** Similar to getNormalBlockStartZ, but returns the grid index instead of the point index.
     * \pre     loadHeader() and loadPQR()
     * \return  The grid index where normal blocks begin (inclusive)
     */
    int getNormalBlockStartGridZ() const;

    /**\See getNormalBlockStartGridZ
     */
    int getNormalBlockStartGridY() const;

    /** \See getNormalBlockStartGridZ
     */
    int getNormalBlockStartGridX() const;


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
     * \retval      nZ              The `NZ` values are different
     * \retval      nY              The `NY` values are different
     * \retval      nX              The `NX` values are different (data dimensions)
     * \retval      data            The data dimensions are the same, but the values are different. If `diffIndex` is non-null, it will contain the `ZYX` data location where the (first) difference occurred, where diffIndex[0] is the Z index.
    */
    differenceType compare(const PFData& otherObj, std::array<int, 3>* diffIndex) const;

    /** Given a flattened index into `data`, unflatten it into its `ZYX` components.
     * Note: The behavior is undefined if the dimension data is not fully initialized.
     * \param[int]  index           The flattened index of the `data`
     * \retval      {Z,Y,X}         Corresponding `ZYX` of the valid `index`, such that the first entry ([0]) is the Z index.
     * \retval      {-1,-1,-1}      Invalid index @@TODO want?
     */
    std::array<int, 3> unflattenIndex(int index) const;

    /** Given a flattened subgrid index (value in range [0, getNumSubgrids()), unflatten it.
     * \pre         loadHeader() and loadPQR()
     * \param       index           Subgrid index, [0, getNumSubgrids())
     * \retval      {Z,Y,X}         The ZYX indicies of the subgrid.
     * \retval      {-1, -1, -1}    Invalid index
     * @@TEST
     */
    std::array<int, 3> unflattenGridIndex(int index) const;

	/**
	 * @param double get[Z,Y,X] [Z] is the lower left corner of the Computational Grid.
	 * This function is useful either when reading an existing file or when confirming the configuration
	 * of a file that is being created where the computational grid has already been set.
	 * @return double
	 */
    double getZ() const;

	/** @param double  get[Z,Y,X] [Y] is the lower left corner of the Computational Grid.
	 * This function is useful either when reading an existing file or when confirming the configuration
	 * of a file that is being created where the computational grid has already been set.
	 * @return double
	 */
    double getY() const;

	/** @param double get[Z,Y,X] [X] is the lower left corner of the Computational Grid.
	 * This function is useful either when reading an existing file or when
	 * confirming the configuration of a file that is being created where the
	 * computational grid has already been set.
	 * @return double
	 */
    double getX() const;

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
     * set[Z,Y,X]
     * @param double [Z,Y,X]
     * [Z,Y,X] is the lower left corner of the Computational Grid.
     * This function is useful when creating a new pfb file. It is important to note that you can call this
     * function on an existing file, but it will invalidate the file and break all subsequent uses of the class
     * unless you call load_file() again to reset the value back to the one used in the file.
     */

	/** @param double set[Z,Y,X] [Z] is the lower left corner of the Computational
	 * Grid. This function is useful when creating a new pfb file. It is
	 * important to note that you can call this function on an existing file,
	 * but it will invalidate the file and break all subsequent uses of the
	 * class unless you call load_file() again to reset the value back to the
	 * one used in the file.
	 */
    void setZ(double Z);

	/** @param double set[Z,Y,X] [Y] is the lower left corner of the Computational
	 * Grid. This function is useful when creating a new pfb file. It is
	 * important to note that you can call this function on an existing file,
	 * but it will invalidate the file and break all subsequent uses of the
	 * class unless you call load_file() again to reset the value back to the
	 * one used in the file.
	 */
    void setY(double Y);

	/** @param double set[Z,Y,X] [X] is the lower left corner of the Computational
	 * Grid. This function is useful when creating a new pfb file. It is
	 * important to note that you can call this function on an existing file,
	 * but it will invalidate the file and break all subsequent uses of the
	 * class unless you call load_file() again to reset the value back to the
	 * one used in the file.
	 */
    void setX(double X);

	/** @param get[NZ,NY,NX] [NZ] describes the dimensions of the
	 * computational domain. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return int
	 */
    int getNZ() const;

	/** @param get[NZ,NY,NX] [NY] describes the dimensions of the
	 * computational domain. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return int
	 */
    int getNY() const;

	/** @param get[NZ,NY,NX] [NX] describe the dimensions of the
	 * computational domain. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return int
	 */
    int getNX() const;

	/** @param double set[NZ,NY,NX] [NZ] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setNZ(int NZ);

	/** @param double set[NZ,NY,NX] [NY] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setNY(int NY);

	/** @param double set[NZ,NY,NX] [NX] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setNX(int NX);

	/** param get[DZ,DY,DX] [DZ] describes the dimensions of the
	 * computational grid. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return double
	 */
    double getDZ() const;

	/** @param get[DZ,DY,DX] [DY] describes the dimensions of the
	 * computational grid. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return double
	 */
    double getDY() const;

	/** @param get[DZ,DY,DX] [DX] describes the dimensions of the
	 * computational grid. This function is useful either when reading an
	 * existing file or when confirming the configuration of a file that is
	 * being created where the computational grid has already been set.
	 * @return double
	 */
    double getDX() const;

	/** @param  double[DZ,DY,DX] [DZ] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setDZ(double DZ);

	/** @param double [DZ,DY,DX] [DY] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setDY(double DY);

	/** @param double [DZ,DY,DX] [DX] describes the dimensions of the
	 * computational grid. This function is useful when creating a new pfb file.
	 * It is important to note that you can call this function on an existing
	 * file, but it will invalidate the file and break all subsequent uses of
	 * the class unless you call load_file() again to reset the value back to
	 * the one used in the file.
	 */
    void setDX(double DX);

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
	 * operator
	 * @param z
	 * @param y
	 * @param x
	 * @return double
	 */
    double operator()(int z, int y, int x);

	/**
	 * getSubgridData
	 * @param int grid
	 */
    double* getSubgridData(int grid);

    /**
     * getData
     * @return double*
     * Get a pointer to the raw data as a one dimensional array.
     */
    double* getData();

    /**
	 * see getData()
	 */
    const double* getData() const;

	/**
	 * setData
	 * @param data flattened ZYX array(X is most contiguous) to use as the data array.
	 */
    void setData(double* data);

	/**
	 * close file. Destructor should automatically handle this in almost all cases.
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
