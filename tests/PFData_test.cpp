//
// Created by Catherine Olschanowsky on 7/14/20.
//
#include "gtest/gtest.h"
#include "parflow/pfdata.hpp"
#include <fstream>
#include <string>
#include <cstdlib>

class PFData_test : public ::testing::Test {

protected:
    char directory[1024];

virtual void SetUp() {
}

virtual void TearDown() {
}

};

TEST_F(PFData_test, loadHeader){
  PFData test;
  int retval = test.loadHeader();
  EXPECT_NE(0,retval);
}

TEST_F(PFData_test, data1){
    // this file does not exist, therefore, load should fail
    PFData test("badname");
    int retval = test.loadHeader();
    EXPECT_NE(0,retval);
}

TEST_F(PFData_test, openClose){
    PFData test;
    test.close();
}

TEST_F(PFData_test, data2){
    // this file needs to exist, and should load
    PFData test("tests/inputs/press.init.pfb");
    int retval = test.loadHeader();
    EXPECT_EQ(0,retval);
    EXPECT_EQ(50,test.getNZ());
    EXPECT_EQ(41,test.getNY());
    EXPECT_EQ(41,test.getNX());
    EXPECT_NEAR(0,test.getZ(),.00001);
    EXPECT_NEAR(0,test.getY(),.00001);
    EXPECT_NEAR(0,test.getX(),.00001);
    EXPECT_EQ(16,test.getNumSubgrids());
    test.close();
}
TEST_F(PFData_test, loadData) {
    // this file needs to exist, and should load
    PFData test("tests/inputs/press.init.pfb");
    int retval = test.loadHeader();
    ASSERT_EQ(0,retval);
    retval = test.loadData();
    ASSERT_EQ(0,retval);
    double* data = test.getData();
    EXPECT_NE(nullptr, data);
    EXPECT_NEAR(98.003604098773,test(0,0,0),1E-12);
    EXPECT_NEAR(97.36460429313328,test(0,0,40),1E-12);
    EXPECT_NEAR(98.0043134691891,test(0,1,0),1E-12);
    EXPECT_NEAR(98.00901307022781,test(0,0,1),1E-12);
    EXPECT_NEAR(92.61370155558751,test(2,1,21),1E-12);
    EXPECT_NEAR(7.98008728357588,test(45,1,0),1E-12);
    EXPECT_NEAR(97.30205516102234,test(0,1,22),1E-12);
    test.close();
}

/*TEST_F(PFData_test, loadDataThreaded){
    PFData base("tests/inputs/press.init.pfb");
    base.loadHeader();
    base.loadData();

    //1 thread
    PFData test1("tests/inputs/press.init.pfb");
    test1.loadHeader();
    test1.loadPQR();
    test1.loadDataThreaded(1);
    EXPECT_EQ(base.compare(test1, nullptr), PFData::differenceType::none);

    //8 threads
    PFData test8("tests/inputs/press.init.pfb");
    test8.loadHeader();
    test8.loadPQR();
    test8.loadDataThreaded(8);
    EXPECT_EQ(base.compare(test8, nullptr), PFData::differenceType::none);

    //40 threads (more than the number of subgrids)
    PFData test40("tests/inputs/press.init.pfb");
    test40.loadHeader();
    test40.loadPQR();
    test40.loadDataThreaded(40);
    EXPECT_EQ(base.compare(test40, nullptr), PFData::differenceType::none);
}*/

TEST_F(PFData_test, fileReadPoint1){
    PFData test("tests/inputs/press.init.pfb");
    int retval = test.loadHeader();
    EXPECT_EQ(0,retval);

    retval = test.loadPQR();
    EXPECT_EQ(0,retval);
    EXPECT_EQ(1, test.getR());
    EXPECT_EQ(4, test.getQ());
    EXPECT_EQ(4, test.getP());

    PFData base("tests/inputs/press.init.pfb");
    base.loadHeader();
    base.loadData();
    for(int z = 0; z < test.getNZ(); ++z){
        for(int y = 0; y < test.getNY(); ++y){
            for(int x = 0; x < test.getNX(); ++x){
                EXPECT_EQ(base(z, y, x), test.fileReadPoint(z, y, x));
            }
        }
    }

    EXPECT_NEAR(98.003604098773,    test.fileReadPoint(0,0,0),1E-12);
    EXPECT_NEAR(97.36460429313328,  test.fileReadPoint(0,0,40),1E-12);
    EXPECT_NEAR(98.0043134691891,   test.fileReadPoint(0,1,0),1E-12);
    EXPECT_NEAR(98.00901307022781,  test.fileReadPoint(0,0,1),1E-12);
    EXPECT_NEAR(92.61370155558751,  test.fileReadPoint(2,1,21),1E-12);
    EXPECT_NEAR(7.98008728357588,   test.fileReadPoint(45,1,0),1E-12);
    EXPECT_NEAR(97.30205516102234,  test.fileReadPoint(0,1,22),1E-12);

    EXPECT_EQ(0, test.getSubgridIndexX(0));
    EXPECT_EQ(3, test.getSubgridIndexX(40));

    test.close();
}

TEST_F(PFData_test, helperFunctions){
    PFData test("tests/inputs/press.init.pfb");
    int retval = test.loadHeader();
    EXPECT_EQ(0,retval);

    retval = test.loadPQR();
    EXPECT_EQ(0,retval);

    //getNormalBlockStartGrid
    EXPECT_EQ(test.getNormalBlockStartGridZ(),  0);
    EXPECT_EQ(test.getNormalBlockStartGridY(),  1);
    EXPECT_EQ(test.getNormalBlockStartGridX(),  1);

    //getNormalBlockStart
    EXPECT_EQ(test.getNormalBlockStartZ(),      0);
    EXPECT_EQ(test.getNormalBlockStartY(),      11);
    EXPECT_EQ(test.getNormalBlockStartX(),      11);

    //getNormalBlockSize
    EXPECT_EQ(test.getNormalBlockSizeZ(),       50);
    EXPECT_EQ(test.getNormalBlockSizeY(),       10);
    EXPECT_EQ(test.getNormalBlockSizeX(),       10);

    EXPECT_EQ(test.getNormalBlockSizeZ(),       50);
    EXPECT_EQ(test.getNormalBlockSizeY(),       10);
    EXPECT_EQ(test.getNormalBlockSizeX(),       10);

    //getSubgridStart
    EXPECT_EQ(test.getSubgridStartZ(0),         0);

    EXPECT_EQ(test.getSubgridStartY(0),         0);
    EXPECT_EQ(test.getSubgridStartY(1),         11);
    EXPECT_EQ(test.getSubgridStartY(2),         21);
    EXPECT_EQ(test.getSubgridStartY(3),         31);

    EXPECT_EQ(test.getSubgridStartX(0),         0);
    EXPECT_EQ(test.getSubgridStartX(1),         11);
    EXPECT_EQ(test.getSubgridStartX(2),         21);
    EXPECT_EQ(test.getSubgridStartX(3),         31);

    //getSubgridSize
    EXPECT_EQ(test.getSubgridSizeZ(0),          50);

    EXPECT_EQ(test.getSubgridSizeY(0),          11);
    EXPECT_EQ(test.getSubgridSizeY(1),          10);
    EXPECT_EQ(test.getSubgridSizeY(2),          10);
    EXPECT_EQ(test.getSubgridSizeY(3),          10);

    EXPECT_EQ(test.getSubgridSizeX(0),          11);
    EXPECT_EQ(test.getSubgridSizeX(1),          10);
    EXPECT_EQ(test.getSubgridSizeX(2),          10);
    EXPECT_EQ(test.getSubgridSizeX(3),          10);

    //getSubgridIndex
    EXPECT_EQ(test.getSubgridIndexZ(0),         0);
    EXPECT_EQ(test.getSubgridIndexZ(49),        0);

    EXPECT_EQ(test.getSubgridIndexY(0),         0);
    EXPECT_EQ(test.getSubgridIndexY(10),        0);
    EXPECT_EQ(test.getSubgridIndexY(11),        1);
    EXPECT_EQ(test.getSubgridIndexY(20),        1);
    EXPECT_EQ(test.getSubgridIndexY(21),        2);
    EXPECT_EQ(test.getSubgridIndexY(30),        2);
    EXPECT_EQ(test.getSubgridIndexY(31),        3);
    EXPECT_EQ(test.getSubgridIndexY(40),        3);

    EXPECT_EQ(test.getSubgridIndexX(0),         0);
    EXPECT_EQ(test.getSubgridIndexX(10),        0);
    EXPECT_EQ(test.getSubgridIndexX(11),        1);
    EXPECT_EQ(test.getSubgridIndexX(20),        1);
    EXPECT_EQ(test.getSubgridIndexX(21),        2);
    EXPECT_EQ(test.getSubgridIndexX(30),        2);
    EXPECT_EQ(test.getSubgridIndexX(31),        3);
    EXPECT_EQ(test.getSubgridIndexX(40),        3);
}

TEST_F(PFData_test, loadPQR){
    PFData test("tests/inputs/press.init.pfb");
    int retval = test.loadHeader();
    EXPECT_EQ(0,retval);

    retval = test.loadPQR();
    EXPECT_EQ(0,retval);

    EXPECT_EQ(1, test.getR());
    EXPECT_EQ(4, test.getQ());
    EXPECT_EQ(4, test.getP());

    EXPECT_EQ(16, test.getNumSubgrids());

    retval = test.loadData();
    ASSERT_EQ(0, retval);
    double* data = test.getData();
    EXPECT_NE(nullptr, data);
    EXPECT_NEAR(98.003604098773,test(0,0,0),1E-12);
    EXPECT_NEAR(97.36460429313328,test(0,0,40),1E-12);
    EXPECT_NEAR(98.0043134691891,test(0,1,0),1E-12);
    EXPECT_NEAR(98.00901307022781,test(0,0,1),1E-12);
    EXPECT_NEAR(92.61370155558751,test(2,1,21),1E-12);
    EXPECT_NEAR(7.98008728357588,test(45,1,0),1E-12);
    EXPECT_NEAR(97.30205516102234,test(0,1,22),1E-12);
}

TEST_F(PFData_test, compareFuncSame){
    PFData test1("tests/inputs/press.init.pfb");
    test1.loadHeader();
    test1.loadData();
    PFData test2("tests/inputs/press.init.pfb");
    test2.loadHeader();
    test2.loadData();

    //none
    auto res = test1.compare(test2, nullptr);
    EXPECT_EQ(res, PFData::differenceType::none);


    //{Z,Y,X}
    test1.setZ(test1.getZ()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::z);
    test1.setZ(test1.getZ()-1.0);

    test1.setY(test1.getY()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::y);
    test1.setY(test1.getY()-1.0);

    test1.setX(test1.getX()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::x);
    test1.setX(test1.getX()-1.0);

    //D{Z,Y,X}
    test1.setDZ(test1.getDZ()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::dZ);
    test1.setDZ(test1.getDZ()-1.0);

    test1.setDY(test1.getDY()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::dY);
    test1.setDY(test1.getDY()-1.0);

    test1.setDX(test1.getDX()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::dX);
    test1.setDX(test1.getDX()-1.0);

    //N{Z,Y,X}
    test1.setNZ(test1.getNZ()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::nZ);
    test1.setNZ(test1.getNZ()-1.0);

    test1.setNY(test1.getNY()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::nY);
    test1.setNY(test1.getNY()-1.0);

    test1.setNX(test1.getNX()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::nX);
    test1.setNX(test1.getNX()-1.0);

    //data
    const int mutIdx = (test1.getNZ() * test1.getNY() * test1.getNX() - 1)/3;
    const auto diffZYX = test1.unflattenIndex(mutIdx);
    test1.getData()[mutIdx]++;

    std::array<int, 3> diffLoc{};
    EXPECT_EQ(test1.compare(test2, &diffLoc), PFData::differenceType::data);
    EXPECT_EQ(diffLoc, diffZYX);    //Ensure reported location is correct

    test1.getData()[mutIdx]--;

    test1.close();
    test2.close();
}

TEST_F(PFData_test, unflattenIndex){
    PFData test("tests/inputs/press.init.pfb");
    test.loadHeader();
    test.loadData();

    //Small function to reflatten index for easy checking
    auto flatten = [&](const std::array<int, 3>& zyx){
        return  zyx[0]*test.getNX()*test.getNY() + zyx[1]*test.getNX() + zyx[2];
    };

    const int maxIdx = test.getNZ() * test.getNY() * test.getNX() - 1;

    {
        const int expected = maxIdx;
        const int calculated = flatten(test.unflattenIndex(expected));
        EXPECT_EQ(expected, calculated);
    }
    {
        const int expected = maxIdx/2;
        const int calculated = flatten(test.unflattenIndex(expected));
        EXPECT_EQ(expected, calculated);
    }
    {
        const int expected = maxIdx/3;
        const int calculated = flatten(test.unflattenIndex(expected));
        EXPECT_EQ(expected, calculated);
    }
    {
        const int expected = maxIdx/4;
        const int calculated = flatten(test.unflattenIndex(expected));
        EXPECT_EQ(expected, calculated);
    }
    {
        const int expected = maxIdx/5;
        const int calculated = flatten(test.unflattenIndex(expected));
        EXPECT_EQ(expected, calculated);
    }

    const std::array<int, 3> invalid = {-1, -1, -1};
    EXPECT_EQ(invalid, test.unflattenIndex(maxIdx+2));
    EXPECT_EQ(invalid, test.unflattenIndex(maxIdx+1));
    EXPECT_EQ(invalid, test.unflattenIndex(-1));

    test.close();
}

TEST_F(PFData_test, unflattenGridIndex){
    PFData test("tests/inputs/press.init.pfb");
    test.loadHeader();
    test.loadPQR();


    std::array<int, 3> expected{0, 0, 0};
    EXPECT_EQ(expected, test.unflattenGridIndex(0));

    expected = {0, 0, 1};
    EXPECT_EQ(expected, test.unflattenGridIndex(1));

    expected = {0, 1, 0};
    EXPECT_EQ(expected, test.unflattenGridIndex(4));

    expected = {0, 1, 1};
    EXPECT_EQ(expected, test.unflattenGridIndex(5));

    expected = {0, 2, 2};
    EXPECT_EQ(expected, test.unflattenGridIndex(10));

    expected = {0, 3, 3};
    EXPECT_EQ(expected, test.unflattenGridIndex(15));

    const std::array<int, 3> invalid{-1, -1, -1};
    EXPECT_EQ(invalid, test.unflattenGridIndex(-1));
    EXPECT_EQ(invalid, test.unflattenGridIndex(16));
    EXPECT_EQ(invalid, test.unflattenGridIndex(17));
}

TEST_F(PFData_test, loadDataAbs) {
    // this file needs to exist, and should load
    char filename[2048];
    getcwd(filename,2048);
    strcat(filename,"/tests/inputs/press.init.pfb");
    PFData test(filename);
    int retval = test.loadHeader();
    ASSERT_EQ(0,retval);
    retval = test.loadData();
    ASSERT_EQ(0,retval);
    double* data = test.getData();
    EXPECT_NE(nullptr, data);
    EXPECT_NEAR(98.003604098773,test(0,0,0),1E-12);
    EXPECT_NEAR(97.36460429313328,test(0,0,40),1E-12);
    EXPECT_NEAR(98.0043134691891,test(0,1,0),1E-12);
    EXPECT_NEAR(98.00901307022781,test(0,0,1),1E-12);
    EXPECT_NEAR(92.61370155558751,test(2,1,21),1E-12);
    EXPECT_NEAR(7.98008728357588,test(45,1,0),1E-12);
    EXPECT_NEAR(97.30205516102234,test(0,1,22),1E-12);

    EXPECT_NEAR(95.173603867758615, test(0, 40, 40), 1E-12);
    EXPECT_NEAR(98.006254316614971, test(0, 39, 39), 1E-12);
    test.close();
}
TEST_F(PFData_test, loadClipTest1){

    // this file needs to exist, and should load
    char filename[2048];
    getcwd(filename,2048);
    strcat(filename,"/tests/inputs/press.init.pfb");
    PFData test(filename);
    int retval = test.loadHeader();
    ASSERT_EQ(0,retval);
    retval = test.loadClipOfData(0,0,3,3);
    ASSERT_EQ(0,retval);
    //double* data = test.getData();
    //EXPECT_NE(nullptr, data);
    //EXPECT_EQ(50, test.getNZ());
    //EXPECT_EQ(3, test.getNY());
    //EXPECT_EQ(3, test.getNX());
    //EXPECT_EQ(0, test.getZ());
    //EXPECT_EQ(0, test.getY());
    //EXPECT_EQ(0, test.getX());
    //EXPECT_NEAR(98.003604098773,test(0,0,0),1E-12);
    //EXPECT_NEAR(98.0043134691891,test(0,1,0),1E-12);
    //EXPECT_NEAR(98.00901307022781,test(0,0,1),1E-12);
    //EXPECT_NEAR(7.98008728357588,test(45,1,0),1E-12);
    //test.close();
}
TEST_F(PFData_test, loadClipTest2){

    // this file needs to exist, and should load
    char filename[2048];
    getcwd(filename,2048);
    strcat(filename,"/tests/inputs/press.init.pfb");
    PFData test(filename);
    int retval = test.loadHeader();
    ASSERT_EQ(0,retval);
    retval = test.loadClipOfData(39,39,2,2);
    ASSERT_EQ(0,retval);
    double* data = test.getData();
    EXPECT_NE(nullptr, data);
    EXPECT_EQ(50, test.getNZ());
    EXPECT_EQ(2, test.getNY());
    EXPECT_EQ(2, test.getNX());
    EXPECT_EQ(0, test.getZ());
    EXPECT_EQ(39, test.getY());
    EXPECT_EQ(39, test.getX());
    EXPECT_NEAR(95.173603867758615, test(0, 1, 1), 1E-12);
    EXPECT_NEAR(98.006254316614971, test(0, 0, 0), 1E-12);
    test.close();
}
TEST_F(PFData_test, loadClipTestTemp){

    // this file needs to exist, and should load
    char filename[2048];
    getcwd(filename,2048);
    strcat(filename,"/tests/inputs/NLDAS.Press.000001_to_000024_orig.pfb");
    PFData test(filename);
    int retval = test.loadHeader();
    ASSERT_EQ(0,retval);
    retval = test.loadClipOfData(2806,1060,10,16);
    ASSERT_EQ(0,retval);
    EXPECT_EQ(20, test.getDZ());
    EXPECT_EQ(1000, test.getDY());
    EXPECT_EQ(1000, test.getDX());
    EXPECT_EQ(24, test.getNZ());
    EXPECT_EQ(16, test.getNY());
    EXPECT_EQ(10, test.getNX());
    EXPECT_EQ(0, test.getZ());
    EXPECT_EQ(1060, test.getY());
    EXPECT_EQ(2806, test.getX());
    retval = test.writeFile("tests/inputs/NLDAS.Press.000001_to_000024_clip.pfb");
    double* data = test.getData();
    EXPECT_NE(nullptr, data);
    //EXPECT_NEAR(95.173603867758615, test(0, 1, 1), 1E-12);
    //EXPECT_NEAR(98.006254316614971, test(0, 0, 0), 1E-12);
    test.close();
}
TEST_F(PFData_test, loadClipTest3){

    // this file needs to exist, and should load
    char filename[2048];
    getcwd(filename,2048);
    strcat(filename,"/tests/inputs/press.init.pfb");
    PFData test(filename);
    int retval = test.loadHeader();
    ASSERT_EQ(0,retval);
    retval = test.loadClipOfData(2,2,39,39);
    ASSERT_EQ(0,retval);
    double* data = test.getData();
    EXPECT_NE(nullptr, data);
    EXPECT_NEAR(95.173603867758615, test(0, 38, 38), 1E-12);
    EXPECT_NEAR(98.006254316614971, test(0, 37, 37), 1E-12);
    test.close();
}

TEST_F(PFData_test, dist_press){
    char buf1[1024];
    char buf2[1024];

    PFData p1("tests/inputs/press.init.pfb");
    p1.loadHeader();
    p1.loadPQR();
    EXPECT_EQ(1, p1.getR());
    EXPECT_EQ(4, p1.getQ());
    EXPECT_EQ(4, p1.getP());
    EXPECT_EQ(16, p1.getNumSubgrids());


    PFData test("tests/inputs/press.init.pfb");
    test.distFile(2,2,1,"tests/press.init.pfb");

    EXPECT_EQ(4, test.getNumSubgrids());
    //Also test loadPQR
    PFData test2("tests/press.init.pfb");
    test2.loadHeader();
    test2.loadPQR();
    EXPECT_EQ(1, test2.getR());
    EXPECT_EQ(2, test2.getQ());
    EXPECT_EQ(2, test2.getP());
    EXPECT_EQ(4, test2.getNumSubgrids());

    FILE* f1 = fopen("tests/inputs/press.init.pfb","rb");
    FILE* f2 = fopen("tests/press.init.pfb","rb");
    ASSERT_NE(f1,nullptr);
    ASSERT_NE(f2,nullptr);
    int retval1 = fread(buf1,1,64,f1);
    int retval2 = fread(buf2,1,64,f2);
    EXPECT_EQ(retval1,retval2);
    int result = memcmp(buf1,buf2,62);
    EXPECT_EQ(0,result);
    result = memcmp(buf1,buf2,64);
    EXPECT_NE(0,result);
    fclose(f1);
    fclose(f2);
    ASSERT_EQ(0,remove("tests/press.init.pfb"));
}


TEST_F(PFData_test, dist_lw_nldas){
    char buf1[1024];
    char buf2[1024];

    PFData test("tests/inputs/NLDAS.APCP.000001_to_000024.pfb");
    test.distFile(2,2,1,"tests/NLDAS.APCP.000001_to_000024.pfb");

    //Also test loadPQR
    PFData test2("tests/NLDAS.APCP.000001_to_000024.pfb");
    test2.loadHeader();
    test2.loadPQR();
    ASSERT_EQ(1, test2.getR());
    ASSERT_EQ(2, test2.getQ());
    ASSERT_EQ(2, test2.getP());

    FILE* f1 = fopen("tests/inputs/NLDAS.APCP.000001_to_000024.pfb","rb");
    FILE* f2 = fopen("tests/NLDAS.APCP.000001_to_000024.pfb","rb");
    ASSERT_NE(f1,nullptr);
    ASSERT_NE(f2,nullptr);
    int retval1 = fread(buf1,1,64,f1);
    int retval2 = fread(buf2,1,64,f2);
    EXPECT_EQ(retval1,retval2);
    int result = memcmp(buf1,buf2,62);
    EXPECT_EQ(0,result);
    result = memcmp(buf1,buf2,64);
    EXPECT_NE(0,result);
    fclose(f1);
    fclose(f2);


    ASSERT_EQ(0,remove("tests/NLDAS.APCP.000001_to_000024.pfb.dist"));
    ASSERT_EQ(0,remove("tests/NLDAS.APCP.000001_to_000024.pfb"));

}

TEST_F(PFData_test, idxCalcs){
   int nz = 1;
   int ny =8;
   int nx =8;
   int r=1,q=3,p=3;

   EXPECT_EQ(1,calcExtent(nz,r,0));
   EXPECT_EQ(3,calcExtent(ny,q,0));
   EXPECT_EQ(3,calcExtent(nx,p,0));
   EXPECT_EQ(3,calcExtent(ny,q,1));
   EXPECT_EQ(3,calcExtent(nx,p,1));
   EXPECT_EQ(2,calcExtent(ny,q,2));
   EXPECT_EQ(2,calcExtent(nx,p,2));

   EXPECT_EQ(0,calcOffset(nz,r,0));
   EXPECT_EQ(0,calcOffset(ny,q,0));
   EXPECT_EQ(0,calcOffset(nx,p,0));
   EXPECT_EQ(3,calcOffset(ny,q,1));
   EXPECT_EQ(3,calcOffset(nx,p,1));
   EXPECT_EQ(6,calcOffset(ny,q,2));
   EXPECT_EQ(6,calcOffset(nx,p,2));

}

TEST_F(PFData_test, emptyFile){
	std::ofstream MyFile("emptyFile");
	MyFile.close();
	PFData test("emptyFile");
	int retval = test.loadHeader();
	EXPECT_NE(0, retval);
	std::remove("emptyFile");
}

TEST_F(PFData_test, fileFromData){
    int retval = -1;
    double data[24];
    for (int i =0; i<24; i++){
        data[i] = (double) rand() / 1000;
    }
    PFData test(data, 1, 4, 6);
    int r = test.getR();
    EXPECT_EQ(1, r);
    int q = test.getQ();
    EXPECT_EQ(1, q);
    int p = test.getP();
    EXPECT_EQ(1, p);
    retval = test.writeFile("tests/pfb_file_from_data.pfb");

    PFData test_read("tests/pfb_file_from_data.pfb");
    retval = test_read.loadHeader();
    EXPECT_EQ(0, retval);
    retval = test_read.loadData();
    EXPECT_EQ(0, retval);
    r = test_read.getR();
    EXPECT_EQ(1, r);
    q = test_read.getQ();
    EXPECT_EQ(1, q);
    p = test_read.getP();
    EXPECT_EQ(1, p);
    EXPECT_EQ(1, test_read.getDZ());
    EXPECT_EQ(1, test_read.getDY());
    EXPECT_EQ(1, test_read.getDX());
    EXPECT_EQ(1, test_read.getNZ());
    EXPECT_EQ(4, test_read.getNY());
    EXPECT_EQ(6, test_read.getNX());
    EXPECT_EQ(0, test_read.getZ());
    EXPECT_EQ(0, test_read.getY());
    EXPECT_EQ(0, test_read.getX());
    test_read.close();
    test.close();
    ASSERT_EQ(0,remove("tests/pfb_file_from_data.pfb"));
}

TEST_F(PFData_test, setData){
    PFData test = PFData();
    double data[24];
    for (int i =0; i<24; i++){
        data[i] = (double) rand() / 1000;
    }
    test.setR(1);
    test.setQ(1);
    test.setP(1);
    test.setDZ(1.0);
    test.setDY(1.0);
    test.setDX(1.0);
    test.setNZ(1);
    test.setNY(4);
    test.setNX(6);
    test.setData(data);
    test.writeFile("tests/test_write_file_out.pfb");
    PFData test_read = PFData("tests/test_write_file_out.pfb");
    test_read.loadHeader();
    test_read.loadData();
    EXPECT_EQ(1, test_read.getR());
    EXPECT_EQ(1, test_read.getQ());
    EXPECT_EQ(1, test_read.getP());
    EXPECT_EQ(1, test_read.getDZ());
    EXPECT_EQ(1, test_read.getDY());
    EXPECT_EQ(1, test_read.getDX());
    EXPECT_EQ(1, test_read.getNZ());
    EXPECT_EQ(4, test_read.getNY());
    EXPECT_EQ(6, test_read.getNX());
    EXPECT_EQ(0, test_read.getZ());
    EXPECT_EQ(0, test_read.getY());
    EXPECT_EQ(0, test_read.getX());

    test_read.close();
    test.close();
    ASSERT_EQ(0,remove("tests/test_write_file_out.pfb"));
}

TEST_F(PFData_test, setIndexOrder) {
    PFData test = PFData();
    double data[24];
    for (int i =0; i<24; i++) {
        data[i] = (double) rand() / 1000;
    }
    test.setData(data);

    // Should equal "zyz"
    EXPECT_EQ(test.getIndexOrder(), "zyx");

    // Should equal "xyz"
    test.setIndexOrder("xyz");
    EXPECT_EQ(test.getIndexOrder(), "xyz");

    // Should equal "xyz"
    test.setIndexOrder("xYz");
    EXPECT_EQ(test.getIndexOrder(), "xyz");

    // Should equal "xyz"
    test.setIndexOrder("xYZ");
    EXPECT_EQ(test.getIndexOrder(), "xyz");

    // Should equal "xyz"
    test.setIndexOrder("XYZ");
    EXPECT_EQ(test.getIndexOrder(), "xyz");

    // Should equal "xyz"
    test.setIndexOrder("XYZZZZ");
    EXPECT_EQ(test.getIndexOrder(), "xyz");

    // Should not work, should still equal "xyz"
    test.setIndexOrder("abc");
    EXPECT_EQ(test.getIndexOrder(), "xyz");

    // Should not be able to write to file when indexOrder == "xyz"
    ASSERT_EQ(test.writeFile("tests/test_write_index_order.pfb"), 1);

    // Should equal "zyx"
    test.setIndexOrder("ZYX");
    EXPECT_EQ(test.getIndexOrder(), "zyx");

    // Should equal "zyx"
    test.setIndexOrder("zYx");
    EXPECT_EQ(test.getIndexOrder(), "zyx");

    // Should be able to write to file
    ASSERT_EQ(test.writeFile("tests/test_write_index_order.pfb"), 0);

    // Read file, indexOrder should equal "zyx"
    PFData test_read = PFData("tests/test_write_index_order.pfb");
    test_read.loadHeader();
    test_read.loadData();
    EXPECT_EQ("zyx", test_read.getIndexOrder());

    test.close();
    test_read.close();
    ASSERT_EQ(0, remove("tests/test_write_index_order.pfb"));
}


