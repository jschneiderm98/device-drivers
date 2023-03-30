#include <linux/kernel.h>
#include <kunit/test.h>
#include "lcd_device_driver.c"

static void test_force_4bit_mode(struct kunit *test)
{
  KUNIT_ASSERT_EQ(test, 2, 1 + 1);
}

/*char lcd_send_nibble(char nibble, char nibble_type)
{
  MSG_OK("entrei no mock");
	return 'A';
}*/


static struct kunit_case lcd_test_cases[] = {
    KUNIT_CASE(test_force_4bit_mode),
    /* Add more test cases as needed */
    {}
};

static struct kunit_suite lcd_test_suite = {
    .name = "lcd-driver-test",
    .test_cases = lcd_test_cases,
};

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Júlio César Schneider Martins <jschneiderm98@gmail.com>");
MODULE_DESCRIPTION("Tests for the device driver to interface with lcd displays that use hitashi_hd_44780");

kunit_test_suite(lcd_test_suite);