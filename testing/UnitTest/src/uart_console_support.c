/*
 * uart_support.c
 *
 * Created: 7/29/2024 11:41:13 AM
 */ 

 #include <string.h>
 
#include "sam.h"
#include "uart_console_support.h"
 
#define PAD0 22	//PA22
#define PAD1 23	//PA23
#define PAD2 24 //PA24
#define PAD3 25	//PA25

void _GPIO_EnablePMUX(PortGroup* port, uint8_t pin, _Bool enable){
	port->PINCFG[pin].bit.PMUXEN = enable? 1 : 0;
}

void _GPIO_SetPeripheralFunction(PortGroup* port, uint8_t pin, uint8_t alt_function){
	uint32_t wrconfig = PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(alt_function);
	uint8_t shift = 0;
	
	if(pin > 15){
		wrconfig |= PORT_WRCONFIG_HWSEL;
		shift = 16;
	}
	uint32_t pin_mask = 1 << (pin - shift);
	port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_PINMASK(pin_mask);

	_GPIO_EnablePMUX(port, pin, 1);
}

/**
 * \internal Calculate 64 bit division, ref can be found in
 * http://en.wikipedia.org/wiki/Division_algorithm#Long_division
 */
static uint64_t long_division(uint64_t n, uint64_t d)
{
	int32_t i;
	uint64_t q = 0, r = 0, bit_shift;
	for (i = 63; i >= 0; i--) {
		bit_shift = (uint64_t)1 << i;

		r = r << 1;

		if (n & bit_shift) {
			r |= 0x01;
		}

		if (r >= d) {
			r = r - d;
			q |= bit_shift;
		}
	}

	return q;
}

#define SHIFT 32
#define BAUD_INT_MAX   8192
#define BAUD_FP_MAX     8

/**
 * \internal Calculate asynchronous baudrate value (UART)
*/
void _sercom_get_async_baud_val(
		const uint32_t baudrate,
		const uint32_t peripheral_clock,
		uint16_t *const baudval,
		uint8_t mode,
		uint8_t sample_num)
{
	*baudval = 0;

	/* Temporary variables  */
	uint64_t ratio = 0;
	uint64_t scale = 0;
	uint64_t baud_calculated = 0;
	uint8_t baud_fp;
	uint32_t baud_int = 0;
	uint64_t temp1;

	/* Check if the baudrate is outside of valid range */
	if ((baudrate * sample_num) > peripheral_clock) {
		/* Return with error code */
		return;
	}

	if(mode == 0) {
		/* Calculate the BAUD value */
		temp1 = ((sample_num * (uint64_t)baudrate) << SHIFT);
		ratio = long_division(temp1, peripheral_clock);
		scale = ((uint64_t)1 << SHIFT) - ratio;
		baud_calculated = (65536 * scale) >> SHIFT;
	} else if(mode == 1) {
		temp1 = ((uint64_t)baudrate * sample_num);
		baud_int = long_division( peripheral_clock, temp1);
		if(baud_int > BAUD_INT_MAX) {
				return;
		}
		temp1 = long_division( 8 * (uint64_t)peripheral_clock, temp1);
		baud_fp = temp1 - 8 * baud_int;
		baud_calculated = baud_int | (baud_fp << 13);
	}

	*baudval = baud_calculated;
	return;
}

 void configure_console(void){
	 // enable the clocks
	 uint8_t sercom_num = 3;
	 uint32_t bus_mask = (0x01 << (2 + sercom_num)); // SERCOM0: bit2, SERCOM1: bit3, etc
	 uint32_t clk_ctrl = GCLK_CLKCTRL_ID(0x14 + sercom_num); // SERCOM0: ID=14, SERCOM1: ID:15, etc

	 PM->APBCMASK.reg |= bus_mask;
	 GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0) | clk_ctrl;

	uint16_t baud;
	_sercom_get_async_baud_val(38400, 1000000, &baud, 0, 16); // should calculate 25270?
	 	 
	SERCOM3->USART.BAUD.reg = baud;
	 
	 while(SERCOM3->USART.SYNCBUSY.reg);
	 
	//SERCOM3->USART.CTRLB.bit.CHSIZE = 0; //chars are 8 bits
	//SERCOM3->USART.CTRLB.bit.SBMODE = 0; // use 1 stop bit
	SERCOM3->USART.CTRLB.bit.RXEN = 1;
	SERCOM3->USART.CTRLB.bit.TXEN = 1;
	 
	while(SERCOM3->USART.SYNCBUSY.reg);

	 

	 SERCOM3->USART.CTRLA.bit.DORD = 1;		// LSB first
	 //SERCOM3->USART.CTRLA.bit.CPOL = 0;
	 //SERCOM3->USART.CTRLA.bit.CMODE = 0;	// asynchronous mode
	 
	 //SERCOM3->USART.CTRLA.bit.FORM = 0;
	 
	 //SERCOM3->USART.CTRLA.bit.SAMPA = 0;
	 SERCOM3->USART.CTRLA.bit.RXPO = 1;		// RX = SERCOM pad 1
	 SERCOM3->USART.CTRLA.bit.TXPO = 0;		// TX = SERCOM pad 0, XCK = pad 1 (not used because async mode)
	 
	 //SERCOM3->USART.CTRLA.bit.SAMPR = 0;
	 //SERCOM3->USART.CTRLA.bit.IBON = 0;
	 
	 SERCOM3->USART.CTRLA.bit.RUNSTDBY = 1;
	 SERCOM3->USART.CTRLA.bit.MODE = 1;		// internal clock	 
	 while(SERCOM3->USART.SYNCBUSY.reg);

	 // configure pins
	 _GPIO_SetPeripheralFunction(&PORT->Group[0], PAD0, 2);
	 _GPIO_SetPeripheralFunction(&PORT->Group[0], PAD1, 2);
	 
	 SERCOM3->USART.CTRLA.bit.ENABLE = 1;
	while(SERCOM3->USART.SYNCBUSY.reg);
 }
 
 
 
 void uart_writechar(char a){
	 while(!(SERCOM3->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
	 SERCOM3->USART.DATA.reg = a;
	 while(!(SERCOM3->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC));
 }
/*
void uart_writestr(const char* string)
{
	const char* pch = string;

	if (pch != NULL)
	{
		while (*pch)
		{
			uart_writechar(pch);
			pch++;
		}
	}
}
*/
 void uart_writestr(const char* string)
 {
	 size_t len = strlen(string);

	 for(int i = 0; i < len; i++){
		 uart_writechar(string[i]);
	 }
 }
