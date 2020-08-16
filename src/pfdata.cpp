#include "parflow/pfdata.hpp"
#include <stdio.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>



#define WRITEINT(V,f) {uint32_t temp = htonl(V); \
                         fwrite(&temp, 4, 1, f);}
#define READINT(V,f,err) {uint32_t buf; \
                         err = fread(&buf, 4, 1, f);\
                         uint32_t temp =  ntohl(buf);\
                         V = (int)temp;}
#define WRITEDOUBLE(V,f) {uint64_t t1 = *(uint64_t*)&V;\
                         t1 =  pfhtonll(t1); \
                         fwrite(&t1, 8, 1, f);}
#define READDOUBLE(V,f,err) {uint64_t buf; \
                         err = fread(&buf, 8, 1, f);\
                         uint64_t temp =  pfntohll(buf);\
                         V = *(double*)&temp;}
uint64_t pfntohll(uint64_t value);
uint64_t pfhtonll(uint64_t value);
int calcOffset(int extent, int block_count, int block_idx);
int calcExtent(int extent, int block_count, int block_idx);

PFData::PFData() {
}
PFData::PFData(std::string filename) {
    m_filename = filename;
}
int PFData::loadHeader() {

    m_fp = fopen( m_filename.c_str(), "rb");
    if(m_fp == NULL){
        perror("Error opening pfbfile");
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

double PFData::getX() const {
    return m_X;
}

void PFData::setX(double X) {
    m_X = X;
}

double PFData::getY() const {
    return m_Y;
}

void PFData::setY(double Y) {
    m_Y = Y;
}

double PFData::getZ() const {
    return m_Z;
}

void PFData::setZ(double Z) {
    m_Z = Z;
}

int PFData::getNX() const {
    return m_nx;
}

void PFData::setNX(int Nx) {
    m_nx = Nx;
}

int PFData::getNY() const {
    return m_ny;
}

void PFData::setNY(int Ny) {
    m_ny = Ny;
}

int PFData::getNZ() const {
    return m_nz;
}

void PFData::setNZ(int Nz) {
    m_nz = Nz;
}

double PFData::getDX() const {
    return m_dX;
}

void PFData::setDX(double DX) {
    m_dX = DX;
}

double PFData::getDY() const {
    return m_dY;
}

void PFData::setDY(double DY) {
    m_dY = DY;
}

double PFData::getDZ() const {
    return m_dZ;
}

void PFData::setDZ(double DZ) {
    m_dZ = DZ;
}

int PFData::getNumSubgrids() const {
    return m_numSubgrids;
}

void PFData::setNumSubgrids(int NumSubgrids) {
    m_numSubgrids = NumSubgrids;
}

double* PFData::getSubgridData(int grid) {
    return NULL;
}

int PFData::getCoordinateDatum(int x, int y, int z, double *value) {
    return 0;
}

double PFData::operator()(int x, int y, int z) {
    return m_data[z*m_ny*m_nx+y*m_nx+x];
}

double* PFData::getData() {
    return m_data;
}

int PFData::loadData() {
    int nsg;
    //subgrid variables
    int x,y,z,nx,ny,nz,rx,ry,rz;
    if(m_fp  == NULL){
        return 1;
    }
    m_data = (double*)malloc(sizeof(double)*m_nx*m_ny*m_nz);
    if(m_data == NULL){
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
        if(nsg == m_numSubgrids-1){
            m_p = m_nx/nx;
            m_q = m_ny/ny;
            m_r = m_nz/nz;
        }

        // read values for subgrid
        // qq is the location of the subgrid
        int qq = z*m_nx*m_ny + y*m_nx + x;
        int k,i,j;
        //int index = qq;//+k*nx*ny+i*nx;
        for (k=0; k<nz; k++){
            for(i=0;i<ny;i++){
                // read full "pencil"
                int index = qq+k*m_nx*m_ny+i*m_nx;
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
                    tmp = pfntohll(tmp);
                    m_data[index+j] = *(double*)(&tmp);
                }
            }
        }
    }
    return 0;
}

void PFData::close() {
  if(m_fp == NULL){
      return;
  }
  fclose(m_fp);
  m_fp = NULL;
  return;
}

int PFData::writeFile(const std::string filename) {

    FILE *fp = fopen(filename.c_str(), "wb");
    if(fp == nullptr){
        perror("Error Opening File");
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
    double writebuf[max_x_extent];
    // now write the subgrids one at a time
    // this iterates over the subgrids in order
    int nsg=0;
    for(int nsg_z=0;nsg_z<m_r;nsg_z++){
        for(int nsg_y=0;nsg_y<m_q;nsg_y++) {
            for (int nsg_x = 0;nsg_x<m_p;nsg_x++) {
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
                //WRITEINT(0, fp);
                //WRITEINT(0, fp);
                //WRITEINT(0, fp);
                WRITEINT(nsg_x, fp);
                WRITEINT(nsg_y, fp);
                WRITEINT(nsg_z, fp);

                int  ix,iy,iz;
                for(iz=calcOffset(m_nz,m_r,nsg_z); iz < calcOffset(m_nz,m_r,nsg_z+1);iz++){
                    for(iy=calcOffset(m_ny,m_q,nsg_y); iy < calcOffset(m_ny,m_q,nsg_y+1);iy++){

                        uint64_t* buf = (uint64_t*)&(m_data[iz*m_nx*m_ny+iy*m_nx+calcOffset(m_nx,m_p,nsg_x)]);
                        int j;
                        for(j=0;j<x_extent;j++){
                            uint64_t tmp = buf[j];
                            tmp = pfntohll(tmp);
                            writebuf[j] = *(double*)(&tmp);
                        }
                        int written = fwrite(writebuf,sizeof(double),x_extent,fp);
                        if(written != x_extent){
                            std::cerr << "Error printing subgrid data" << std::endl;
                            return 1;
                        }
                    }

                }
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

int PFData::writeFile() {

    return 0;
}

int PFData::distFile(int P, int Q, int R, const std::string outFile) {
    loadHeader();
    loadData();
    m_p = P;
    m_q = Q;
    m_r = R;
    return writeFile(outFile);
}

std::string PFData::getFilename() const{
    return m_filename;
}


uint64_t pfntohll(uint64_t value) {
    if (htonl(1) != 1){
        const uint32_t high_part = ntohl((uint32_t)(value >> 32));
        const uint32_t low_part = ntohl((uint32_t)(value));
        uint64_t retval = (uint64_t)low_part << 32;
        retval = retval | high_part;
        return retval;
    }
    return value;
}
uint64_t pfhtonll(uint64_t value) {
    if (htonl(1) != 1){
        const uint32_t high_part = htonl((uint32_t)(value >> 32));
        const uint32_t low_part = htonl((uint32_t)(value));
        uint64_t retval = (uint64_t)low_part << 32;
        retval = retval | high_part;
        return retval;
    }
    return value;
}