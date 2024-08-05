/*
 * GPIO_tests.h
 *
 * Created: 7/30/2024 2:16:13 PM
 */ 


#ifndef GPIO_TESTS_H_
#define GPIO_TESTS_H_

#include <stdlib.h>
#include <time.h>

#include "unity.h"

void t_pinToMask(void);

void test_myPassingTest(void);
void test_myFailingTest(void);

void t_setPinsAsInput(void);
void t_setPinsAsOutput(void);

void t_setPortsAsInput(void);
void t_setPortsAsOutput(void);

void t_randomPinDirections(void);

void t_resetPort(void);

#endif /* GPIO_TESTS_H_ */