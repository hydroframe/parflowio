//
// Created by Catherine Olschanowsky on 7/14/20.
//
#include "gtest/gtest.h"
#include "parflow/pfdata.hpp"

class PFData_test : public ::testing::Test {
protected:
virtual void SetUp()
{
}

virtual void TearDown() {
}

};

TEST(Test1, loadFile1){
  PFData test;
  int retval = test.loadFile();
  EXPECT_EQ(0,retval);
}

TEST(Test1, data1){
    // this file does not exist, therefore, load should fail
    PFData test("badname");
    int retval = test.loadFile();
    EXPECT_NE(0,retval);
}

TEST(Test1, data2){
    // this file needs to exist, and should load
    PFData test("tests/var/validfilename.pdb");
    int retval = test.loadFile();
    EXPECT_EQ(0,retval);
}

TEST(Test1, header1){
    // Let's make sure that we can load the header information from the file
    PFData test("tests/var/validfilename.pdb");
    int retval = test.loadFile();
    int NX = test.getNX();
    int NY= test.getNY();
    int NZ = test.getNZ();
    EXPECT_EQ(41,NX);
    EXPECT_EQ(41,NY);
    EXPECT_EQ(5,NZ);
}

