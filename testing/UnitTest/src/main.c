/*
 * UnitTest.c
 *
 * Created: 7/30/2024 3:32:58 PM
 */ 


#include "sam.h"

#include "unity.h"
#include "uart_console_support.h"
#include "GPIO_tests.h"

#define BOARD_NAME                "SAMD21_XPLAINED_PRO"

#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- Unit Testing with Unity and SERCOM3 UART --\r\n" \
"-- "BOARD_NAME" --\r\n" \
"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL


void setUp(void) {
	// set stuff up here
}

void tearDown(void) {
	// clean stuff up here
}

void UnityHelperDeadLoop(void)
{
	while(1) {};
}

int main(void)
{
	/*Configure UART console.*/
	configure_console();
	
	/* Output header information */
	uart_writestr(STRING_HEADER);
	uart_writestr("Running GPIO driver tests"STRING_EOL);
	UNITY_BEGIN();
	RUN_TEST(t_pinToMask);

	RUN_TEST(t_setPinsAsOutput);
	RUN_TEST(t_setPinsAsInput);
	
	RUN_TEST(t_setPortsAsOutput);
	RUN_TEST(t_setPortsAsInput);
	
	RUN_TEST(t_randomPinDirections);

	RUN_TEST(t_resetPort);
	return UNITY_END();
}
