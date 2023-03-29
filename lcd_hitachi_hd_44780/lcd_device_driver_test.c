#include <kunit/test.h>
#include "lcd_device_driver.h"

static void test_force_4bit_mode(struct kunit *test)
{
  force_4bit_mode();
}

char Send_Nibble(char nibble, char nibble_type)
{
  MSG_OK("Entrei no mock");
	return 0;
}


static struct kunit_case lcd_test_cases[] = {
    KUNIT_CASE(test_force_4bit_mode),
    /* Add more test cases as needed */
    {}
};

static struct kunit_suite lcd_test_suite = {
    .name = "lcd-driver-test",
    .test_cases = lcd_test_cases,
};

kunit_test_suite(lcd_test_suite);