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
    EXPECT_EQ(41,test.getNX());
    EXPECT_EQ(41,test.getNY());
    EXPECT_EQ(50,test.getNZ());
    EXPECT_NEAR(0,test.getX(),.00001);
    EXPECT_NEAR(0,test.getY(),.00001);
    EXPECT_NEAR(0,test.getZ(),.00001);
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
    EXPECT_NEAR(97.36460429313328,test(40,0,0),1E-12);
    EXPECT_NEAR(98.0043134691891,test(0,1,0),1E-12);
    EXPECT_NEAR(98.00901307022781,test(1,0,0),1E-12);
    EXPECT_NEAR(92.61370155558751,test(21,1,2),1E-12);
    EXPECT_NEAR(7.98008728357588,test(0,1,45),1E-12);
    EXPECT_NEAR(97.30205516102234,test(22,1,0),1E-12);
    test.close();
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


    //{X,Y,Z}
    test1.setX(test1.getX()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::x);
    test1.setX(test1.getX()-1.0);

    test1.setY(test1.getY()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::y);
    test1.setY(test1.getY()-1.0);

    test1.setZ(test1.getZ()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::z);
    test1.setZ(test1.getZ()-1.0);

    //D{X,Y,Z}
    test1.setDX(test1.getDX()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::dX);
    test1.setDX(test1.getDX()-1.0);

    test1.setDY(test1.getDY()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::dY);
    test1.setDY(test1.getDY()-1.0);

    test1.setDZ(test1.getDZ()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::dZ);
    test1.setDZ(test1.getDZ()-1.0);


    //N{X,Y,Z}
    test1.setNX(test1.getNX()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::nX);
    test1.setNX(test1.getNX()-1.0);

    test1.setNY(test1.getNY()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::nY);
    test1.setNY(test1.getNY()-1.0);

    test1.setNZ(test1.getNZ()+1.0);
    EXPECT_EQ(test1.compare(test2, nullptr), PFData::differenceType::nZ);
    test1.setNZ(test1.getNZ()-1.0);

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
    auto flatten = [&](const std::array<int, 3>& xyz){
        return  xyz[2]*test.getNX()*test.getNY() + xyz[1]*test.getNX() + xyz[0];
    };

    const int maxIdx = test.getNX() * test.getNY() * test.getNZ() - 1;

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
    EXPECT_NEAR(97.36460429313328,test(40,0,0),1E-12);
    EXPECT_NEAR(98.0043134691891,test(0,1,0),1E-12);
    EXPECT_NEAR(98.00901307022781,test(1,0,0),1E-12);
    EXPECT_NEAR(92.61370155558751,test(21,1,2),1E-12);
    EXPECT_NEAR(7.98008728357588,test(0,1,45),1E-12);
    EXPECT_NEAR(97.30205516102234,test(22,1,0),1E-12);
    test.close();
}

//TEST_F(PFData_test, readWrite){
//    char buf1[1024];
//    char buf2[1024];
//
//    PFData test("tests/inputs/press.init.pfb");
//    int retval = test.loadHeader();
//    ASSERT_EQ(0,retval);
//    retval = test.loadData();
//    ASSERT_EQ(0,retval);
//    retval = test.writeFile("tests/press.init.pfb.tmp");
//    ASSERT_EQ(0,retval);
//
//    FILE* f1 = fopen("tests/inputs/press.init.pfb","rb");
//    FILE* f2 = fopen("tests/press.init.pfb.tmp","rb");
//    ASSERT_NE(f1,nullptr);
//    ASSERT_NE(f2,nullptr);
//    int retval1 = fread(buf1,1,1024,f1);
//    int retval2 = fread(buf2,1,1024,f2);
//    int diff = 0;
//    int count = 0;
//    EXPECT_EQ(retval1,retval2);
//    while(retval1 == retval2 && retval1 == 1024){
//        if(memcmp(buf1,buf2,1024) != 0){
//            diff = 1;
//            fprintf(stderr,"Files differ at  read %d\n",count);
//            int tstCount=1;
//            while(memcmp(buf1,buf2,tstCount)==0){tstCount++;}
//            fprintf(stderr,"Files differ at  byte %d\n",tstCount);
//            fprintf(stderr,"val0: %lf val1: %lf",(double)buf1[tstCount-1],(double)buf2[tstCount-1]);
//
//            break;
//        }
//        retval1 = fread(buf1,1,1024,f1);
//        retval2 = fread(buf2,1,1024,f2);
//        count++;
//    }
//    ASSERT_EQ(0,diff);
//    ASSERT_EQ(retval1,retval2);
//    if(memcmp(buf1,buf2,retval1) != 0){
//        diff = 1;
//    }
//    ASSERT_EQ(0,diff);
//    fclose(f1);
//    fclose(f2);
//    ASSERT_EQ(0,remove("tests/press.init.pfb.tmp"));
//
//}
TEST_F(PFData_test, dist_press){
    char buf1[1024];
    char buf2[1024];

    PFData test("tests/inputs/press.init.pfb");
    test.distFile(2,2,1,"tests/press.init.pfb");

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
   int nx =8;
   int ny =8;
   int nz = 1;
   int p=3,q=3,r=1;

    EXPECT_EQ(3,calcExtent(nx,p,0));
    EXPECT_EQ(3,calcExtent(ny,q,0));
    EXPECT_EQ(1,calcExtent(nz,r,0));
    EXPECT_EQ(3,calcExtent(nx,p,1));
    EXPECT_EQ(3,calcExtent(ny,q,1));
    EXPECT_EQ(2,calcExtent(nx,p,2));
    EXPECT_EQ(2,calcExtent(ny,q,2));

   EXPECT_EQ(0,calcOffset(nx,p,0));
   EXPECT_EQ(0,calcOffset(ny,q,0));
   EXPECT_EQ(0,calcOffset(nz,r,0));
   EXPECT_EQ(3,calcOffset(nx,p,1));
   EXPECT_EQ(3,calcOffset(ny,q,1));
   EXPECT_EQ(6,calcOffset(nx,p,2));
   EXPECT_EQ(6,calcOffset(ny,q,2));

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
    PFData test(data, 6, 4, 1);
    int p = test.getP();
    EXPECT_EQ(1, p);
    int q = test.getQ();
    EXPECT_EQ(1, q);
    int r = test.getR();
    EXPECT_EQ(1, r);
    retval = test.writeFile("tests/pfb_file_from_data.pfb");

    PFData test_read("tests/pfb_file_from_data.pfb");
    retval = test_read.loadHeader();
    EXPECT_EQ(0, retval);
    retval = test_read.loadData();
    EXPECT_EQ(0, retval);
    p = test_read.getP();
    EXPECT_EQ(1, p);
    q = test_read.getQ();
    EXPECT_EQ(1, q);
    r = test_read.getR();
    EXPECT_EQ(1, r);
    EXPECT_EQ(1, test_read.getDX());
    EXPECT_EQ(1, test_read.getDY());
    EXPECT_EQ(1, test_read.getDZ());
    EXPECT_EQ(1, test_read.getNZ());
    EXPECT_EQ(4, test_read.getNY());
    EXPECT_EQ(6, test_read.getNX());
    EXPECT_EQ(0, test_read.getX());
    EXPECT_EQ(0, test_read.getY());
    EXPECT_EQ(0, test_read.getZ());
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
    test.setP(1);
    test.setQ(1);
    test.setR(1);
    test.setDX(1.0);
    test.setDY(1.0);
    test.setDZ(1.0);
    test.setNX(6);
    test.setNY(4);
    test.setNZ(1);
    test.setData(data);
    test.writeFile("tests/test_write_file_out.pfb");
    PFData test_read = PFData("tests/test_write_file_out.pfb");
    test_read.loadHeader();
    test_read.loadData();
    EXPECT_EQ(1, test_read.getP());
    EXPECT_EQ(1, test_read.getQ());
    EXPECT_EQ(1, test_read.getR());
    EXPECT_EQ(1, test_read.getDX());
    EXPECT_EQ(1, test_read.getDY());
    EXPECT_EQ(1, test_read.getDZ());
    EXPECT_EQ(1, test_read.getNZ());
    EXPECT_EQ(4, test_read.getNY());
    EXPECT_EQ(6, test_read.getNX());
    EXPECT_EQ(0, test_read.getX());
    EXPECT_EQ(0, test_read.getY());
    EXPECT_EQ(0, test_read.getZ());

    test_read.close();
    test.close();
    ASSERT_EQ(0,remove("tests/test_write_file_out.pfb"));
}

//TEST_F(PFData_test, readFile){
	//std::ofstream readFile("newFile");
	//std::string  data="";
	//std::string test="this will show up in the file.\n";
	//
	//readFile << test;
	//readFile.close();
	//std::ifstream infile;
    //infile.open("newFile");
	////infile.getline(data,100);
	//while(!infile.eof()){
			//infile >> data;}
	////std::cout<<data<<std::endl;
	//EXPECT_EQ(0,data.compare(test));
//}
