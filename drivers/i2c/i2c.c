#include "i2c.h"

void I2C_ClkControl(uint8_t sercom_num, bool setBusClkEnabled, int8_t coreClkGenerator, int8_t slowClkGenerator){
	uint32_t bus_mask = (0x01 << (2 + sercom_num)); // SERCOM0: bit2, SERCOM1: bit3, etc
	
	uint32_t core_clk_ctrl =  GCLK_CLKCTRL_ID(0x14 + sercom_num); // SERCOM0: ID=14, SERCOM1: ID:15, etc

	if(coreClkGenerator != -1){
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(coreClkGenerator) | core_clk_ctrl;
	}
	else {
		GCLK->CLKCTRL.reg = core_clk_ctrl;
	}
	
	if(slowClkGenerator != -1){
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(slowClkGenerator) | GCLK_CLKCTRL_ID_SERCOMX_SLOW;
	}
	else {
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOMX_SLOW;
	}
}

void I2C_Reset(Sercom* sercom){
	sercom->I2CM.CTRLA.bit.SWRST = 1; // resets all registers of the sercom peripheral and disables it
	while(sercom->SPI.SYNCBUSY.bit.SWRST);
}

void I2C_SetEnabled(Sercom* sercom, bool setEnabled){
	sercom->I2CM.CTRLA.bit.ENABLE = setEnabled? 1 : 0;
	while(sercom->SPI.SYNCBUSY.bit.ENABLE);
}
