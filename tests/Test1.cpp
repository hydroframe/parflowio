//
// Created by Catherine Olschanowsky on 7/14/20.
//
#include "gtest/gtest.h"
extern "C" {
#include "parflowio/pfdata.hpp"
}

class Test1 : public ::testing::Test {
  int testval;
protected:
virtual void SetUp()
{
    testval = 1;
}

virtual void TearDown() {
}

};

TEST(Test1, Exp1){
    hello();
    int testval = 1;
  EXPECT_EQ(testval,1);
}

