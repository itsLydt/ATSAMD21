/*
 * GPIO_tests.c
 *
 * Created: 7/30/2024 2:19:40 PM
 */ 

#include "gpio_tests.h"

void test_myPassingTest(void){
	TEST_ASSERT_EQUAL(0, 0);
}

void test_myFailingTest(void){
	TEST_ASSERT_EQUAL(0, 100);
}
