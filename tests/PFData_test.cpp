//
// Created by Catherine Olschanowsky on 7/14/20.
//
#include "gtest/gtest.h"
#include "parflow/pfdata.hpp"
#include <iostream>
#include <fstream>
#include <string>
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
    test.close();
}

TEST_F(PFData_test, emptyFile){
	std::ofstream MyFile("emptyFile");
	MyFile.close();
	PFData test("emptyFile");
	int retval = test.loadHeader();
	EXPECT_NE(0, retval);
	std::remove("emptyFile");
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
