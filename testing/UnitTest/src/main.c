/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>

#include "unity.h"
#include "GPIO_tests.h"

#define STRING_EOL    "\r"
#define STRING_HEADER "-- Unit Testing with Unity and SERCOM3 UART --\r\n" \
"-- "BOARD_NAME" --\r\n" \
"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

static struct usart_module cdc_uart_module;

/**
 *  Configure UART console.
 */
static void configure_console(void)
{
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	usart_conf.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	usart_conf.baudrate    = 38400;

	stdio_serial_init(&cdc_uart_module, EDBG_CDC_MODULE, &usart_conf);
	usart_enable(&cdc_uart_module);
}

void UnityHelperDeadLoop(void)
{
	while(1) {};
}

void setUp(void) {
	// set stuff up here
}

void tearDown(void) {
	// clean stuff up here
}

int main (void)
{
	system_init();

	/*Configure UART console.*/
	configure_console();
	
	/* Output header information */
	puts(STRING_HEADER);
	
	puts("Running GPIO driver tests"STRING_EOL);
	UNITY_BEGIN();
	RUN_TEST(test_myFailingTest);
	RUN_TEST(test_myPassingTest);
	return UNITY_END();
	
}
