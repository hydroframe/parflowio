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

