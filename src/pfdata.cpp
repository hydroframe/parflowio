#include "parflow/pfdata.hpp"
#include "pfutil.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>



#define WRITEINT(V,f) {uint32_t temp = bswap32(V); \
                         fwrite(&temp, 4, 1, f);}
#define READINT(V,f,err) {uint32_t buf; \
                         err = fread(&buf, 4, 1, f);\
                         uint32_t temp =  bswap32(buf);\
                         V = (int)temp;}
#define WRITEDOUBLE(V,f) {uint64_t t1 = *(uint64_t*)&V;\
                         t1 =  bswap64(t1); \
                         fwrite(&t1, 8, 1, f);}
#define READDOUBLE(V,f,err) {uint64_t buf; \
                         err = fread(&buf, 8, 1, f);\
                         uint64_t temp =  bswap64(buf);\
                         V = *(double*)&temp;}


PFData::PFData(std::string filename)
    : m_filename{filename} {}

PFData::PFData(double *data, int nz, int ny, int nx)
    : m_data{data}, m_nz{nz}, m_ny{ny}, m_nx{nx} {}

PFData::~PFData(){
    if(m_fp){
        std::fclose(m_fp);
    }

    if(m_dataOwner){
        std::free(m_data);
    }
}

int PFData::loadHeader() {

    m_fp = fopen( m_filename.c_str(), "rb");
    if(m_fp == nullptr){
        std::string err{"Error opening file: \"" + m_filename + "\""};
        perror(err.c_str());
        return 1;
    }

    /* read in header information */
    int errcheck;
    READDOUBLE(m_X,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_Y,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_Z,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_nx,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_ny,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_nz,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_dX,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_dY,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_dZ,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_numSubgrids,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}

    return 0;
}

int PFData::loadPQR(){
    //Read nx, ny, nz from subgrid header.
    auto readValues = [](std::FILE* f, int& nx, int& ny, int& nz){
        int err = std::fread(&nx, 4, 1, f);
        if(err != 1){
            perror("Error reading nx in subgrid header");
            return err;
        }

        err = std::fread(&ny, 4, 1, f);
        if(err != 1){
            perror("Error reading ny in subgrid header");
            return err;
        }

        err = std::fread(&nz, 4, 1, f);
        if(err != 1){
            perror("Error reading nz in subgrid header");
            return err;
        }

        nx = bswap32(nx);
        ny = bswap32(ny);
        nz = bswap32(nz);

        return 0;
    };


    std::fpos_t pos{};
    //Save old position
    std::fgetpos(m_fp, &pos);
    std::fseek(m_fp, 64+12, SEEK_SET); //Skip file header and first 3 ints of the subgrid header.

    //Reset
    m_r = 0;
    m_q = 0;
    m_p = 0;

    int yDim{};
    int xDim{};


    //Each iter:
    //  if m_q == 0, m_p++
    //  Every time xDim == m_nx, add 1 to m_q
    //  Every time yDim == m_ny, add 1 to m_r

    for(int i = 0; i < m_numSubgrids; ++i){
        int nx, ny, nz;
        if(int err = readValues(m_fp, nx, ny, nz)){
            std::fsetpos(m_fp, &pos);
            return err;
        }

        xDim += nx;

        if(m_q == 0){
            m_p++;
        }

        if(xDim == m_nx){
            yDim += ny;     //Only increase y count once per row
            m_q++;
            xDim = 0;
        }

        if(yDim == m_ny){
            m_r++;
            yDim = 0;
        }

        if(i != m_numSubgrids-1){//Avoid skipping past end of file
            //Skip the last 3 ints of the subgrid header, the data, and the first 3 ints of the next subgrid header
            std::fseek(m_fp, 12+8*nx*ny*nz+12, SEEK_CUR);
        }

    }

    //Restore old position
    std::fsetpos(m_fp, &pos);
    return 0;
}


long PFData::getSubgridOffset(int gridZ, int gridY, int gridX) const{
    //Number of elements
    long offset = getSubgridOffsetElements(gridZ, gridY, gridX);

    //--Account for headers and data size

    offset *= 8;    //Size of double (specified to always be 8 for .pfb)

    offset += 64;   //Header at beginning of file

    //Number of headers to SKIP.
    const int numHeaders = (m_p * m_q * gridZ) + (m_p * gridY) + gridX;
    offset += 36 * numHeaders;  //36 bytes per subgrid header;

    return offset;
}

long PFData::getSubgridOffsetElements(int gridZ, int gridY, int gridX) const{
    //NOTE: recall that remainder blocks come first, followed by normal blocks.
    //Grid dimensions of targeted block
    const long currBlockSizeZ = getSubgridSizeZ(gridZ);   //Size of the targeted Z block
    const long currBlockSizeY = getSubgridSizeY(gridY);   //Size of the targeted Y block;

    //--Calculate Z
    //Remainder blocks
    long elements = (m_nx * m_ny * (getNormalBlockSizeZ()+1))* std::min(gridZ, getNormalBlockStartGridZ());

    //Normal blocks
    if(gridZ >= getNormalBlockStartGridZ()){
        elements += (m_nx * m_ny * getNormalBlockSizeZ()) * (gridZ - getNormalBlockStartGridZ());
    }

    //--Calculate Y
    //Remainder blocks
    elements += (m_nx * (getNormalBlockSizeY()+1) * currBlockSizeZ) * std::min(gridY, getNormalBlockStartGridY());

    //Normal blocks
    if(gridY >= getNormalBlockStartGridY()){
        elements += (m_nx * getNormalBlockSizeY() * currBlockSizeZ) * (gridY - getNormalBlockStartGridY());
    }

    //--Calculate X
    //Remainder blocks
    elements += ((getNormalBlockSizeX()+1) * currBlockSizeY * currBlockSizeZ) * std::min(gridX, getNormalBlockStartGridX());

    //Normal blocks
    if(gridX >= getNormalBlockStartGridX()){
        elements += (getNormalBlockSizeX() * currBlockSizeY * currBlockSizeZ) * (gridX - getNormalBlockStartGridX());
    }

    return elements;
}

long PFData::getPointOffset(int z, int y, int x) const{
    const int gridZ = getSubgridIndexZ(z);
    const int gridY = getSubgridIndexY(y);
    const int gridX = getSubgridIndexX(x);

    const int remZ = z - getSubgridStartZ(gridZ);
    const int remY = y - getSubgridStartY(gridY);
    const int remX = x - getSubgridStartX(gridX);

    const int sizeZ = getSubgridSizeZ(gridZ);
    const int sizeY = getSubgridSizeY(gridY);
    const int sizeX = getSubgridSizeX(gridX);

    const long subgridOffset = getSubgridOffset(gridZ, gridY, gridX) + 36;  //Skip header
    const long pointOffset = 8 * ((remZ * sizeY * sizeX) + (remY * sizeX) + remX);
    return subgridOffset + pointOffset;
}

int PFData::fileReadSubgridAtGridIndexInternal(double* buffer, std::FILE* fp, int gridZ, int gridY, int gridX) const{
    const long long offset = getSubgridOffset(gridZ, gridY, gridX) + 36; //Skip header
    std::fseek(fp, offset, SEEK_SET);

    static_assert(sizeof(double) == 8, "Double must be 8 bytes");

    //Number of elements to read
    const long long count = getSubgridSizeX(gridZ) * getSubgridSizeY(gridY) * getSubgridSizeZ(gridX);

    std::size_t numRead = std::fread(buffer, 8, count, fp);
    if(numRead != static_cast<std::size_t>(count)){
        return errno;
    }

    //Perform endian conversion
    for(std::size_t i = 0; i < count; ++i){
        uint64_t tmp = *reinterpret_cast<uint64_t*>(&buffer[i]);
        tmp = bswap64(tmp);
        buffer[i] = *reinterpret_cast<double*>(&tmp);
    }

    return 0;
}

double PFData::fileReadPoint(int z, int y, int x){
    std::fpos_t pos{};
    //Save old position
    std::fgetpos(m_fp, &pos);

    const long offset = getPointOffset(z, y, x);
    if(std::fseek(m_fp, offset, SEEK_SET)){
        std::perror("Error seeking to file");
    }

    double data = 0;
    int err = 0;
    READDOUBLE(data, m_fp, err);
    if(1 != err){
        std::perror("Error reading double");
    }

    //Restore old position
    std::fsetpos(m_fp, &pos);

    return data;
}

std::vector<double> PFData::fileReadSubgridAtPointIndex(int z, int y, int x){
    const int gridZ = getSubgridIndexX(z);
    const int gridY = getSubgridIndexX(y);
    const int gridX = getSubgridIndexX(x);

    return fileReadSubgridAtGridIndex(gridZ, gridY, gridX);
}

std::vector<double> PFData::fileReadSubgridAtGridIndex(int gridZ, int gridY, int gridX){
    const long count = getSubgridSizeZ(gridZ) * getSubgridSizeY(gridY) * getSubgridSizeX(gridX);

    //Fill with empty data
    std::vector<double> result(count);

    std::fpos_t pos{};
    //Save old position
    std::fgetpos(m_fp, &pos);

    int ret = fileReadSubgridAtGridIndexInternal(result.data(), m_fp, gridZ, gridY, gridX);

    //Restore old position
    std::fsetpos(m_fp, &pos);

    if(ret){
        std::cerr << "Error while reading subgrid at subgrid index(ZYX): {" << gridZ << ", " << gridY << ", " << gridX << "}, error code " << ret << ": " << std::strerror(ret) << "\n";
        result.clear();
    }

    return result;
}

int PFData::getSubgridIndexZ(int idx) const{
    const int start = getNormalBlockStartZ();
    const int size = getNormalBlockSizeZ();
    if(idx < start){   //In remainder subgrid
        return idx/(size+1);
    }

    //In normal block subgrids
    //Number of remainder blocks + number of normal blocks
    return (m_nz % m_r) + (idx - start)/size;
}

int PFData::getSubgridIndexY(int idx) const{
    const int start = getNormalBlockStartY();
    const int size = getNormalBlockSizeY();
    if(idx < start){   //In remainder subgrid
        return idx/(size+1);
    }

    //In normal block subgrids
    //Number of remainder blocks + number of normal blocks
    return (m_ny % m_q) + (idx - start)/size;
}

int PFData::getSubgridIndexX(int idx) const{
    const int start = getNormalBlockStartX();
    const int size = getNormalBlockSizeX();
    if(idx < start){   //In remainder subgrid
        return idx/(size+1);
    }

    //In normal block subgrids
    //Number of remainder blocks + number of normal blocks
    return (m_nx % m_p) + (idx - start)/size;
}


int PFData::getSubgridSizeZ(int gridIdx) const{
    const int size = getNormalBlockSizeZ();
    const int numRem = m_nz % m_r;
    return gridIdx >= numRem ? size : size+1;
}

int PFData::getSubgridSizeY(int gridIdx) const{
    const int size = getNormalBlockSizeY();
    const int numRem = m_ny % m_q;
    return gridIdx >= numRem ? size : size+1;
}

int PFData::getSubgridSizeX(int gridIdx) const{
    const int size = getNormalBlockSizeX();
    const int numRem = m_nx % m_p;
    return gridIdx >= numRem ? size : size+1;
}


int PFData::getSubgridStartZ(int gridIdx) const{
    const int size = getNormalBlockSizeZ();
    const int start = m_nx % m_r;

    //Remainder blocks
    int offset = (size+1) * std::min(gridIdx, start);

    //Normal blocks
    if(gridIdx >= start){
        offset += size * (gridIdx - start);
    }

    return offset;
}

int PFData::getSubgridStartY(int gridIdx) const{
    const int size = getNormalBlockSizeY();
    const int start = m_ny % m_q;

    //Remainder blocks
    int offset = (size+1) * std::min(gridIdx, start);

    //Normal blocks
    if(gridIdx >= start){
        offset += size * (gridIdx - start);
    }

    return offset;
}

int PFData::getSubgridStartX(int gridIdx) const{
    const int size = getNormalBlockSizeX();
    const int start = m_nx % m_p;

    //Remainder blocks
    int offset = (size+1) * std::min(gridIdx, start);

    //Normal blocks
    if(gridIdx >= start){
        offset += size * (gridIdx - start);
    }

    return offset;
}


int PFData::getNormalBlockSizeZ() const{
    return m_nz/m_r;
}

int PFData::getNormalBlockSizeY() const{
    return m_ny/m_p;
}

int PFData::getNormalBlockSizeX() const{
    return m_nx/m_p;
}


int PFData::getNormalBlockStartZ() const{
    return (m_nz % m_r) * (getNormalBlockSizeZ()+1);
}

int PFData::getNormalBlockStartY() const{
    return (m_ny % m_q) * (getNormalBlockSizeY()+1);
}

int PFData::getNormalBlockStartX() const{
    return (m_nx % m_p) * (getNormalBlockSizeX()+1);
}


int PFData::getNormalBlockStartGridZ() const{
    return m_nz % m_r;
}

int PFData::getNormalBlockStartGridY() const{
    return m_ny % m_q;
}

int PFData::getNormalBlockStartGridX() const{
    return m_nx % m_p;
}


double PFData::getZ() const {
    return m_Z;
}

void PFData::setZ(double Z) {
    m_Z = Z;
}

double PFData::getY() const {
    return m_Y;
}

void PFData::setY(double Y) {
    m_Y = Y;
}

double PFData::getX() const {
    return m_X;
}

void PFData::setX(double X) {
    m_X = X;
}

int PFData::getNZ() const {
    return m_nz;
}

void PFData::setNZ(int Nz) {
    m_nz = Nz;
}

int PFData::getNY() const {
    return m_ny;
}

void PFData::setNY(int Ny) {
    m_ny = Ny;
}

int PFData::getNX() const {
    return m_nx;
}

void PFData::setNX(int Nx) {
    m_nx = Nx;
}

double PFData::getDZ() const {
    return m_dZ;
}

void PFData::setDZ(double DZ) {
    m_dZ = DZ;
}

double PFData::getDY() const {
    return m_dY;
}

void PFData::setDY(double DY) {
    m_dY = DY;
}

double PFData::getDX() const {
    return m_dX;
}

void PFData::setDX(double DX) {
    m_dX = DX;
}

int PFData::getNumSubgrids() const {
    return m_numSubgrids;
}

void PFData::setNumSubgrids(int NumSubgrids) {
    m_numSubgrids = NumSubgrids;
}

double* PFData::getSubgridData(int grid) {
    return nullptr;
}

double PFData::operator()(int z, int y, int x) {
    return m_data[static_cast<long long>(z)*m_ny*m_nx+y*m_nx+x];
}

std::string PFData::getIndexOrder() const {
    return m_indexOrder;
}

void PFData::setIndexOrder(std::string indexOrder) {
    // Input string should only be 3 chars long - "zyx" or "xyz"
    indexOrder = indexOrder.substr(0, 3);

    // Convert string to lowercase and compare to valid indexOrder strings
    for (int i = 0; i < (int)indexOrder.length(); i++) {
        indexOrder[i] = std::tolower(indexOrder[i]);

        if ((indexOrder[i] != "zyx"[i]) && (indexOrder[i] != "xyz"[i])) {
            return;
        }
    }

    // At this point, indexOrder is valid
    m_indexOrder = indexOrder;
}

double* PFData::getData() {
    return m_data;
}

const double* PFData::getData() const{
    return m_data;
}

int PFData::loadData() {
    int nsg;
    //subgrid variables
    int x,y,z,nx,ny,nz,rx,ry,rz;
    if(m_fp  == nullptr){
        return 1;
    }

    if(m_data && m_dataOwner){
        std::free(m_data);
    }

    m_data = (double*)std::malloc(sizeof(double)*m_nx*m_ny*m_nz);
    m_dataOwner = true;

    if(m_data == nullptr){
        return 2;
    }

    for (nsg = 0;nsg<m_numSubgrids; nsg++){
        // read subgrid header
        int errcheck;
        READINT(x,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(y,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(z,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(nx,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(ny,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(nz,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(rx,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(ry,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(rz,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}

        // \/ \/ Not how this works, replaced with loadPQR();
        //if(nsg == m_numSubgrids-1){
        //    m_p = m_nx/nx;
        //    m_q = m_ny/ny;
        //    m_r = m_nz/nz;
        //}

        // read values for subgrid
        // qq is the location of the subgrid
        long long qq = z*m_nx*m_ny + y*m_nx + x;
        long long k,i,j;
        //int index = qq;//+k*nx*ny+i*nx;
        for (k=0; k<nz; k++){
            for(i=0;i<ny;i++){
                // read full "pencil"
                long long index = qq+k*m_nx*m_ny+i*m_nx;
                uint64_t* buf = (uint64_t*)&(m_data[index]);
                int read_count = fread(buf,8,nx,m_fp);
                if(read_count != nx){
                    perror("Error Reading Data, File Ended Unexpectedly");
                    return 1;
                }
                // handle byte order
                //uint64_t* buf = (uint64_t*)&(m_data[index]);
                for(j=0;j<nx;j++){
                    uint64_t tmp = buf[j];
                    tmp = bswap64(tmp);
                    m_data[index+j] = *(double*)(&tmp);
                }
            }
        }
    }
    return 0;
}

/**
 * This function makes the assumption that the clipping is only in 2D and all z
 * values will be contained
**/
int PFData::loadClipOfData(int clip_x, int clip_y, int extent_x, int extent_y) {

    int nsg;
    //subgrid variables
    int x,y,z,nx,ny,nz,rx,ry,rz;
    if(m_fp  == nullptr){
        return 1;
    }

    if(m_data && m_dataOwner){
        std::free(m_data);
    }

    // allocating based on size of slice.
    m_data = (double*)std::malloc(sizeof(double)*extent_x*extent_y*m_nz);
    m_dataOwner = true;

    if(m_data == nullptr){
        return 2;
    }

    for (nsg = 0;nsg<m_numSubgrids; nsg++){
        // read subgrid header
        int errcheck;
        READINT(x,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(y,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(z,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(nx,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(ny,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(nz,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(rx,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(ry,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(rz,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}

        // is this subgrid part of our clip?
        int x_overlap = fminl(clip_x+extent_x, x+nx) - fmaxl(clip_x,x); 
        int y_overlap = fminl(clip_y+extent_y, y+ny) - fmaxl(clip_y,y); 
        if(x_overlap > 0 && y_overlap >0){
          // some of the data is in here -- will read the whole subgrid, but
          // only save the overlap part.
          uint64_t* buf =(uint64_t*) malloc(sizeof(uint64_t)*nx);

          // read values for subgrid
          // qq is the location of the subgrid
          // z will always be 0
          // qq points to the location in the destination data where the first value should
          // be written
          long long qq = z*extent_x*extent_y + (fmaxl(y,clip_y)-clip_y)*extent_x + (fmaxl(x,clip_x)-clip_x);
          long long k,i,j;
          //int index = qq;//+k*nx*ny+i*nx;
          for (k=0; k<nz; k++){
              for(i=0;i<ny;i++){
                  // read full "pencil"
                  int read_count = fread(buf,8,nx,m_fp);
                  if(read_count != nx){
                      perror("Error Reading Data, File Ended Unexpectedly");
                      return 1;
                  }
                  // handle byte order and copy over only what we need
                  long long index = qq+k*extent_x*extent_y+i*extent_x;
                  // if this pencil is within our y-range
                  if(y+i >= clip_y && y+i < clip_y+extent_y){
                    long long index = qq+k*extent_x*extent_y+i*extent_x;
                    int pos =0;
                    // this loop needs to go over the portion of the pencil that is
                    // within our clip. Sometimes our clip will be entirely contained
                    // by this subgrid and sometimes it won't
                    // j needs to go from 0 to nx
                    for(j=0;j<nx;j++){
                        if((x+j) >= clip_x && (x+j) < clip_x+extent_x){
                          index = (z+k)*extent_y*extent_x +((y+i)-clip_y)*extent_x + (x+j)-clip_x;
                          uint64_t tmp = buf[j];
                          tmp = bswap64(tmp);
                          m_data[index] = *(double*)(&tmp);
                          pos++;
                       }
                    }
                 }
              }
          }
          free(buf);
        }else{
          // scan to the next subgrid
          std::fseek(m_fp, 8*nx*ny*nz, SEEK_CUR);
        }
    }

    m_X = clip_x;
    m_Y = clip_y;
    m_nx = extent_x;
    m_ny = extent_y;
    m_numSubgrids = 1;

    return 0;
}


int PFData::emplaceSubgridFromFile(std::FILE* fp, int gridZ, int gridY, int gridX){
    //Position file
    const long offset = getSubgridOffset(gridZ, gridY, gridX) + 36;
    std::fseek(fp, offset, SEEK_SET);

    const int sizeZ = getSubgridSizeZ(gridZ);
    const int sizeY = getSubgridSizeY(gridY);
    const int sizeX = getSubgridSizeX(gridX);

    const int startZ = getSubgridStartZ(gridZ);
    const int startY = getSubgridStartY(gridY);
    const int startX = getSubgridStartX(gridX);

    //The index into m_data where the first element of the grid belongs.
    const long long startOfGrid = startZ*m_nx*m_ny + startY * m_nx + startX;

    for(int z = 0; z < sizeZ; ++z){
        for(int y = 0; y < sizeY; ++y){
            const long long index = startOfGrid + z * m_nx * m_ny + y * m_nx;
            double* const dataPtr = &(m_data[index]);

            const std::size_t numRead = std::fread(dataPtr, 8, sizeX, fp);
            if(numRead != static_cast<std::size_t>(sizeX)){
                return errno;
            }

            //Perform endian byte swap
            for(std::size_t i = 0; i < sizeX; ++i){
                uint64_t tmp = *reinterpret_cast<uint64_t*>(&dataPtr[i]);
                tmp = bswap64(tmp);
                dataPtr[i] = *reinterpret_cast<double*>(&tmp);
            }
        }
    }

    return 0;
}

int PFData::loadDataThreaded(const int numThreads){
    if(numThreads < 1){
        std::cerr << "Number of threads must be at least 1\n";
        return EINVAL;
    }

    //Note: [begin, end)
    auto threadFunc = [this](int subgridBegin, int subgridEnd, std::FILE* fp, int& err){
        err = 0;

        for(int i = subgridBegin; i < subgridEnd; ++i){
            const std::array<int, 3> idx = unflattenGridIndex(i);
            err = emplaceSubgridFromFile(fp, idx[0], idx[1], idx[2]);
            if(err){
                break;
            }
        }
    };

    if(m_data && m_dataOwner){
        std::free(m_data);
    }

    m_data = reinterpret_cast<double*>(std::malloc(sizeof(double) * m_nx * m_ny * m_nz));
    m_dataOwner = true;

    std::vector<std::thread> pool(numThreads);
    std::vector<int> retCodes(numThreads);
    std::vector<std::FILE*> fps(numThreads);

    //Open separate file pointers
    for(int i = 0; i < numThreads; ++i){
        fps.at(i) = std::fopen(m_filename.c_str(), "rb");
        if(!fps.at(i)){
            std::perror("Unable to open file for reading");
            return errno;
        }
    }

    //Base number of grids
    const int gridsPerThread = m_numSubgrids / numThreads;
    //Number of threads that get an extra grid
    const int numRemainderThreads = m_numSubgrids % numThreads;

    for(int i = 0; i < numThreads; ++i){
        const int subgridBegin = gridsPerThread * i + std::min(i, numRemainderThreads);
        int subgridEnd = subgridBegin + gridsPerThread;
        if(i < numRemainderThreads){     //Distribute remainder grids
            subgridEnd++;
        }

        pool.at(i) = std::thread(threadFunc, subgridBegin, subgridEnd, fps.at(i), std::ref(retCodes.at(i)));
    }

    for(int i = 0; i < numThreads; ++i){
        pool.at(i).join();
        std::fclose(fps.at(i));
    }

    //Separate loop to ensure we join all threads and close all fps
    for(int i = 0; i < numThreads; ++i){
        const int err = retCodes.at(i);
        if(err){
            std::cerr << "loadDataThreaded: error in thread number " << i << ", error code " << err << ":" << strerror(err) << "\n";
            return err;
        }
    }

    return 0;
}

void PFData::close() {
    if(m_fp){
        std::fclose(m_fp);
        m_fp = nullptr;
    }
}

int PFData::writeFile(const std::string filename) {
    std::vector<long> _offsets((m_p*m_q*m_r) + 1);
    return writeFile(filename, _offsets);
}

int PFData::writeFile(const std::string filename, std::vector<long> &byte_offsets) {
    // m_indexOrder must be set to "zyx" in order to write file
    // Notify user and exit function if not
    if (m_indexOrder != "zyx") {
        perror("PFData indexOrder attribute must be set to \"zyx\" before calling writeFile(). "
                "Please confirm that your arrays are in the right order, and call setIndexOrder() "
                "on your PFData object to set this attribute.");
        return 1;
    }

    std::FILE* fp = std::fopen(filename.c_str(), "wb");
    if(fp == nullptr){
        std::string err{"Error opening file: \"" + filename + "\""};
        perror(err.c_str());
        return 1;
    }

    // calculate the number of subgrids.
    m_numSubgrids = m_p * m_q * m_r;
    WRITEDOUBLE(m_X,fp);
    WRITEDOUBLE(m_Y,fp);
    WRITEDOUBLE(m_Z,fp);
    WRITEINT(m_nx,fp);
    WRITEINT(m_ny,fp);
    WRITEINT(m_nz,fp);
    WRITEDOUBLE(m_dX,fp);
    WRITEDOUBLE(m_dY,fp);
    WRITEDOUBLE(m_dZ,fp);
    WRITEINT(m_numSubgrids,fp);
    int max_x_extent =calcExtent(m_nx,m_p,0);
    std::vector<double> writeBuf(max_x_extent);
    // now write the subgrids one at a time
    // this iterates over the subgrids in order
    int nsg=0;
    byte_offsets[0] = 0;
    long long sg_count = 1;
    for(int nsg_z=0;nsg_z<m_r;nsg_z++){
        for(int nsg_y=0;nsg_y<m_q;nsg_y++) {
            for (int nsg_x = 0;nsg_x<m_p;nsg_x++) {
                //Write byte offset of chunk from header
                // This subgrid starts at x,y,z
                // The number of items in the x-direction of each block is m_nx/m_p + [1|0]
                // The 1 is added for the last m_nx%m_p blocks
                int x = m_X + calcOffset(m_nx,m_p,nsg_x);
                int y = m_Y + calcOffset(m_ny,m_q,nsg_y);
                int z = m_Z + calcOffset(m_nz,m_r,nsg_z);
                // x,y,z of lower lefthand corner
                WRITEINT(x, fp);
                WRITEINT(y, fp);
                WRITEINT(z, fp);
                // nx,ny,nz extents of each direction
                int x_extent =calcExtent(m_nx,m_p,nsg_x);
                WRITEINT(x_extent, fp);
                WRITEINT(calcExtent(m_ny,m_q,nsg_y), fp);
                WRITEINT(calcExtent(m_nz,m_r,nsg_z), fp);
                // subgrid  location in 3D grid
                WRITEINT(1, fp);
                WRITEINT(1, fp);
                WRITEINT(1, fp);

                long long ix,iy,iz;
                for(iz=calcOffset(m_nz,m_r,nsg_z); iz < calcOffset(m_nz,m_r,nsg_z+1);iz++){
                    for(iy=calcOffset(m_ny,m_q,nsg_y); iy < calcOffset(m_ny,m_q,nsg_y+1);iy++){

                        uint64_t* buf = (uint64_t*)&(m_data[iz*m_nx*m_ny+iy*m_nx+calcOffset(m_nx,m_p,nsg_x)]);
                        long long j;
                        for(j=0;j<x_extent;j++){
                            uint64_t tmp = buf[j];
                            tmp = bswap64(tmp);
                            writeBuf[j] = *(double*)(&tmp);
                        }
                        int written = fwrite(writeBuf.data(),sizeof(double),x_extent,fp);
                        if(written != x_extent){
                            std::fclose(fp);
                            std::cerr << "Error writing subgrid data to file " << filename << "\n";
                            perror("");
                            return 1;
                        }
                    }
                }
                byte_offsets[sg_count] = std::ftell(fp);
                sg_count++;
            }
        }
        nsg++;
    }
    fclose(fp);
    return 0;
}

int calcExtent(int extent, int block_count, int block_idx) {
    int lx = extent % block_count;
    int bx = extent / block_count;
    int block_extent = bx;
    if (block_idx < lx) {
        block_extent++;
    }
    return block_extent;
}

int calcOffset(int extent, int block_count, int block_idx) {
    int lx = extent % block_count;
    int bx = extent / block_count;
    int offset =  block_idx * bx;
    if (block_idx < lx) {
        offset += block_idx;
    }else{
        offset += lx;
    }
    return offset;
}

int PFData::distFile(int P, int Q, int R, const std::string outFile) {
    loadHeader();
    loadData();
    m_p = P;
    m_q = Q;
    m_r = R;
    // create array to hold byte offset for blocks
    std::vector<long> offsets((P*Q*R)+1);
    //create the filestream for the .dist file
    std::fstream distFile(outFile + ".dist", std::ios::trunc | std::ios::out) ;   //Clear file if it exists
    if(!distFile){
        perror("Error creating distfile");
        return 1;
    }

    //write the blocked pfb file to disk, collecting the block offsets
    int rtnVal = writeFile(outFile, offsets);

    //write the block offsets to the .dist file
    for (long long i = 0; i<=static_cast<long long>(P)*Q*R; i++){
        distFile << offsets[i] << "\n";
    }

    return rtnVal;
}

PFData::differenceType PFData::compare(const PFData& otherObj, std::array<int, 3>* diffIndex) const{
    //Check relevant header data
    if(otherObj.getZ()  != getZ())  return differenceType::z;
    if(otherObj.getY()  != getY())  return differenceType::y;
    if(otherObj.getX()  != getX())  return differenceType::x;

    if(otherObj.getDZ() != getDZ()) return differenceType::dZ;
    if(otherObj.getDY() != getDY()) return differenceType::dY;
    if(otherObj.getDX() != getDX()) return differenceType::dX;

    if(otherObj.getNZ() != getNZ()) return differenceType::nZ;
    if(otherObj.getNY() != getNY()) return differenceType::nY;
    if(otherObj.getNX() != getNX()) return differenceType::nX;


    //Check for differences in the data array
    //@@TODO: Do we want to handle invalid data?
    assert(otherObj.getData() && getData());
    assert(getNZ() > 0 && getNY() > 0 && getNX() > 0);

    const double* dataOther = otherObj.getData();
    const double* dataSelf = getData();
    const long long dataSize = static_cast<long long>(getNZ()) * getNY() * getNX();

    for(long long i = 0; i < dataSize; ++i){
        if(dataOther[i] != dataSelf[i]){
            if(diffIndex){  //Only write if not null
                *diffIndex = unflattenIndex(i);
            }

            return differenceType::data;
        }
    }

    return differenceType::none;
}

std::array<int, 3> PFData::unflattenIndex(int index) const{
    if(index >= getNZ() * getNY() * getNX() || index < 0){  //Invalid index, @@TODO assert instead?
        return {-1, -1, -1};
    }

    const int z = index / (getNX() * getNY());
    index -= z * getNX() * getNY();

    const int y = index / getNX();
    index -= y * getNX();

    const int x = index;    //index remainder is x value

    //Sanity check
    assert(z < getNZ() && z >= 0);
    assert(y < getNY() && y >= 0);
    assert(x < getNX() && x >= 0);

    return {z, y, x};
}

std::array<int, 3> PFData::unflattenGridIndex(int index) const{
    if(index >= m_numSubgrids || index < 0){
        return {-1, -1, -1};
    }

    const int gridZ = index / (getP() * getQ());
    index -= gridZ * getP() * getQ();

    const int gridY = index / getP();
    index -= gridY * getP();

    const int gridX = index;
    return {gridZ, gridY, gridX};
}

std::string PFData::getFilename() const{
    return m_filename;
}

void PFData::setData(double *data) {
    m_data = data;
}

int PFData::getP() const {
    return m_p;
}

int PFData::getQ() const {
    return m_q;
}

int PFData::getR() const {
    return m_r;
}

void PFData::setP(int P) {
    m_p = P;
}

void PFData::setQ(int Q) {
    m_q = Q;
}

void PFData::setR(int R) {
    m_r = R;
}

void PFData::setIsDataOwner(bool isOwner){
    m_dataOwner = isOwner;
}
