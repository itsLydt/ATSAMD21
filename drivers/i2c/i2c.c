#include "i2c.h"

void I2C_ClkControl(uint8_t sercom_num, bool en_busClk, int8_t coreClkGenerator, int8_t slowClkGenerator){
	uint32_t bus_mask = (0x01 << (2 + sercom_num)); // SERCOM0: bit2, SERCOM1: bit3, etc
	if(en_busClk){
		PM->APBCMASK.reg |= bus_mask;
	}
	else {
		PM->APBCMASK.reg &= ~bus_mask;
	}

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

void I2C_ConfigureHostTimeouts(Sercom* sercom, struct TimeoutConfigHost_t* timeoutInfo){
	sercom->I2CM.CTRLA.bit.LOWTOUTEN = timeoutInfo->enableLOWTOUTEN;
	sercom->I2CM.CTRLA.bit.INACTOUT = timeoutInfo->timeoutMode;
	sercom->I2CM.CTRLA.bit.SEXTTOEN = timeoutInfo->enableSEXTTOEN;
	sercom->I2CM.CTRLA.bit.MEXTTOEN = timeoutInfo->enableMEXTTOEN;
}

void I2C_ConfigureClientTimeouts(Sercom* sercom, struct TimeoutConfigClient_t* timeoutInfo){
	sercom->I2CS.CTRLA.bit.LOWTOUTEN = timeoutInfo->enableLOWTOUTEN;
	sercom->I2CS.CTRLA.bit.SEXTTOEN = timeoutInfo->enableSEXTTOEN;
}

void I2C_InitHost(Sercom* sercom, bool stretch_mode, uint8_t bus_speed, uint8_t sda_hold, bool enable_sm, uint8_t baud, uint8_t baudlow){
	I2C_SetEnabled(sercom, false);
	
	// 1. set the mode
	sercom->I2CM.CTRLA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val;
	
	struct TimeoutConfigHost_t timeoutInfo = { 1, 0, 0, 0}; // enable LOWTOUTEN
	I2C_ConfigureHostTimeouts(sercom, &timeoutInfo);
	
	sercom->I2CM.CTRLA.bit.SCLSM = bus_speed == 2? 1 : stretch_mode; // high-speed mode requires stretch mode 1
	sercom->I2CM.CTRLA.bit.SPEED = bus_speed;
	sercom->I2CM.CTRLA.bit.SDAHOLD = sda_hold;
	sercom->I2CM.CTRLB.bit.SMEN = enable_sm;

	if(bus_speed < 2){
		// standard mode, fast mode, or fast mode+
		sercom->I2CM.BAUD.bit.BAUD = baud;
		sercom->I2CM.BAUD.bit.BAUDLOW = baudlow;
		// TODO: fast mode + requires high:low ratio of 1:2
	}
	else {
		// high speed mode
		sercom->I2CM.BAUD.bit.HSBAUD = baud;
		sercom->I2CM.BAUD.bit.HSBAUDLOW = baudlow;
	}
	I2C_SetEnabled(sercom, true);
	// set bus state to IDLE
	sercom->I2CM.STATUS.bit.BUSSTATE = 1; 
	// wait for bus state to be synchronized
	while(sercom->I2CM.SYNCBUSY.bit.SYSOP);
}

void I2C_InitClient(Sercom* sercom, bool stretch_mode, uint8_t bus_speed, uint8_t sda_hold, uint8_t addr_mode, bool auto_addr_ack, bool enable_sm){
	I2C_SetEnabled(sercom, false);
	
	// 1. set the mode
	sercom->I2CS.CTRLA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_SLAVE_Val;
	struct TimeoutConfigClient_t timeoutInfo = { 0, 0 };
	I2C_ConfigureClientTimeouts(sercom, &timeoutInfo);
	
	sercom->I2CS.CTRLA.bit.SCLSM = stretch_mode;
	// 2. Optional: SDA hold time
	// 3. Optional: Smart mode
	// 4. Optional: SCL low timeout
	// 5. Address information (Address mode, address, and address mask)
	I2C_SetEnabled(sercom, true);
}


void I2C_Reset(Sercom* sercom){
	sercom->I2CM.CTRLA.bit.SWRST = 1; // resets all registers of the sercom peripheral and disables it
	while(sercom->SPI.SYNCBUSY.bit.SWRST);
}

void I2C_SetEnabled(Sercom* sercom, bool setEnabled){
	sercom->I2CM.CTRLA.bit.ENABLE = setEnabled? 1 : 0;
	while(sercom->SPI.SYNCBUSY.bit.ENABLE);
}
