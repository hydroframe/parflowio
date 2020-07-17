#ifndef PARFLOWIO_PFDATA_HPP
#define PARFLOWIO_PFDATA_HPP

class PFData {
    char* filename;
    FILE* fp;

    // The following information is available only after the file is opened
    size_t size;
    int nx,ny,nz; // dimensions
    int rx,ry,rz; // block sizes from header
    double* data;

    PFData();
};

/**
 * get_value
 * @param int x
 * @param int y
 * @param int z
 * @return double value from location in file
 * Looks up a single value from the pfb file.
 */
#define get_value(x,y,z) get_valueiii(x,y,z)
/**
 * get_values
 * @param int x
 * @param int y
 * @return double* a pointer to the block data
 */
#define get_values(x,y) get_valueii(x,y)
struct pfdata* get_valueii(int x, int y);
#define get_values() get_data()
struct pfdata* get_data(int x, int y);


/**
 * get_num_blocks
 * @param a relative path to a pfb file
 * @return the number of blocks saved in this file
 */
int get_num_blocks(const char*);

#endif //PARFLOWIO_PFDATA_HPP
