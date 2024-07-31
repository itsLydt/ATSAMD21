#include "i2c.h"

void I2C_ClkControl(uint8_t sercom_num, _Bool en_busClk, int8_t coreClkGenerator, int8_t slowClkGenerator){
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

void I2C_InitHost(Sercom* sercom, _Bool stretch_mode, uint8_t bus_speed, uint8_t sda_hold, _Bool enable_sm, uint8_t baud, uint8_t baudlow){
	I2C_SetEnabled(sercom, 0);
	
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
	I2C_SetEnabled(sercom, 1);
	// set bus state to IDLE
	sercom->I2CM.STATUS.bit.BUSSTATE = 1; 
	// wait for bus state to be synchronized
	while(sercom->I2CM.SYNCBUSY.bit.SYSOP);
}

void I2C_InitClient(Sercom* sercom, _Bool stretch_mode, uint8_t bus_speed, uint8_t sda_hold, uint8_t addr_mode, _Bool auto_addr_ack, _Bool enable_sm){
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
	I2C_SetEnabled(sercom, 1);
}

_Bool I2C_TryCalcBaud(uint16_t gclk_freq, uint16_t target_sclk_freq, uint8_t* baud, uint8_t* baudlow){	
	/* calculations assume worst possible allowable rise time according to I2C specification
	SM: 1000 ns
	FM: 300 ns
	FM+: 120 ns
	HS: 80ns
	*/	
	
	/* ratio of core clock to target bit rate */
	uint16_t clk_ratio = gclk_freq / target_sclk_freq;
		
	uint16_t min_clk_ratio;
	uint8_t rise_mod;
	uint16_t b;
	if(target_sclk_freq <= 100){
		// standard mode
		rise_mod = gclk_freq / 1000; // f_CLK (kHz) * 1000 (ns) / 1 x 10^6 -> f_CLK / 1000
		min_clk_ratio = 10 + 2 + rise_mod;
		b = clk_ratio - 10 - rise_mod; // >= 2
	}
	else if(target_sclk_freq <= 400){
		// fast mode
		rise_mod = (gclk_freq * 3) / 10000; // f_CLK (kHz) * 300 (ns) / 1 x 10^6 ->  3*f_CLK/10000
		min_clk_ratio = 10 + 2 + rise_mod;
		b = clk_ratio - 10 - rise_mod; // >= 2
	}
	else if(target_sclk_freq <= 1000){
		// fast-mode+
		rise_mod = (gclk_freq * 3) / 25000;	// f_CLK (kHz) * 120 (ns) / 1 x 10^6 ->  3*f_CLK/25000
		min_clk_ratio = 10 + 3 + rise_mod;
		b = clk_ratio - 10 - rise_mod; // >= 3
	}
	else if( target_sclk_freq <= 3400){
		// high speed mode
		rise_mod = 0;
		min_clk_ratio = 2 + 3;
		b = clk_ratio - 2; // >= 3
	}
	else {
		// error: 3.4MHz is the maximum supported speed
		*baud = 0;
		*baudlow = 0;
		return false;
	}

	if(clk_ratio < min_clk_ratio){
		// error: sercom core clk must be at least (ratio) times faster than the desired bit rate
		*baud = 0;
		*baudlow = 0;
		return false;
	}
	
	if(target_sclk_freq <= 400){
		// SM or FM: T_low and T_high can be even
		*baud = *baudlow = b/2;
	}
	else {
		// FM+ or HS: T_low must be twice as long as T_high
		*baud = b / 3;
		*baudlow = *baud * 2;
	}

	return true;
}

void I2C_Reset(Sercom* sercom){
	sercom->I2CM.CTRLA.bit.SWRST = 1; // resets all registers of the sercom peripheral and disables it
	while(sercom->SPI.SYNCBUSY.bit.SWRST);
}

void I2C_SetEnabled(Sercom* sercom, _Bool setEnabled){
	sercom->I2CM.CTRLA.bit.ENABLE = setEnabled? 1 : 0;
	while(sercom->SPI.SYNCBUSY.bit.ENABLE);
}

void I2CHost_GenerateStop(Sercom* i2c){
	i2c->I2CM.CTRLB.bit.CMD = 0x3;
	// wait for stop condition to occur
	//while(i2c->I2CM.SYNCBUSY.bit.SYSOP);
}

size_t I2CHost_SendData(Sercom* i2c, uint8_t addr, uint8_t* txBuffer, size_t len){
	/* 1. Generate start condition, send address */
	// In this family of microcontrollers, start signal is generated automatically when setting client address and direction
	i2c->I2CM.ADDR.reg = (addr << 1); // bit 0 is direction of transfer (0 - write, 1 - read);
	/* 2. Wait for ACK/NACK of address */
	
	/* Three possibilities:
	1. arbitration lost or bus error
		INTFLAG.MB and STATUS.ARBLOST set, STATUS.BUSERR may also be set
		clear the interrupt and leave
	2. address transmitted, no ACK
		INTFLAG.MB and STATUS.RXNACK set
		send stop sequence and leave
	3. address transmitted with ACK
		INTFLAG.MB set, STATUS.RXNACK cleared
		begin transmitting data packets
	*/	
	
	// wait until MB flag set
	while(!i2c->I2CM.INTFLAG.bit.MB);
	
	uint32_t status = i2c->I2CM.STATUS.reg;
	if(status & SERCOM_I2CM_STATUS_ARBLOST || status & SERCOM_I2CM_STATUS_BUSERR){
		// case 1
		return 0;
	}
	
	if(status & SERCOM_I2CM_STATUS_RXNACK){
		// NACK, generate stop condition
		I2CHost_GenerateStop(i2c);
		return 0;
	}	
	
	// transmit bytes
	int b_written;
	for(b_written = 0; b_written < len; b_written++){
		i2c->I2CM.DATA.reg = txBuffer[b_written];
		// wait for transaction to take place
		while(i2c->I2CM.SYNCBUSY.bit.SYSOP);
		// wait until MB flag set
		while(!i2c->I2CM.INTFLAG.bit.MB);
		// check if ack
		if(i2c->I2CM.STATUS.bit.RXNACK){
			//not acknowledged, bail
			break;
		}
	}
	I2CHost_GenerateStop(i2c);
	return b_written;
}