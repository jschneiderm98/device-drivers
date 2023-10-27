#include "CppUTest/TestHarness.h"

extern "C"
{
#include "lcd_device_driver.h"
}

TEST_GROUP(FirstTestGroup)
{
};

TEST(FirstTestGroup, FirstTest)
{
   STRCMP_EQUAL("igual", "igual");
}