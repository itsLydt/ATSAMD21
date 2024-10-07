#ifndef I2C_H_
#define I2C_H_

#include "sam.h"

#include <stddef.h>
#include <stdbool.h>

/* enables/configures the clocks used by the I2C peripheral
sercom_num: 0-5; indicates which sercom instance is to be used. Note that not all sercom pins can be used as I2C pins
en_busClk: enables or disables the SERCOMx bus clock (CLK_SERCOMx_APB)
coreClkGen: 0-7 or -1: indicates which clock generator will supply the clock signal for the core clock (GCLK_SERCOMx_CORE)
			set to -1 to disable this clock
slowClkGen: 0-7 or -1: indicates which clock generator will supply the clock signal for the slow clock (GCLK_SERCOM_SLOW)
			set to -1 to disable this clock
 */
void I2C_ClkControl(uint8_t sercom_num, bool en_busClk, int8_t coreClkGen, int8_t slowClkGen);

/* Timeout configuration:
LOWTOUTEN: Enable SCL Low Time-Out
	if enabled, the host/client will release its clock hold, if enabled, 
	and complete the current transaction if SCL is held low for 25ms-35ms
INACTOUT: Inactive Time-Out (host only)
	if enabled, bus state will be returned to idle if the bus is inactive for the specified timeout period.
	required for SMBus
SEXTTOEN: Enable Client SCL Low Extend Time-Out
	if enabled, the host/client will release its clock hold, if enabled, 
	and complete the current transaction if SCL is cumulatively held low for greater than 25ms from the
	initial START to a STOP
MEXTTOEN: Host SCL Low Extend Time-Out (host only)
	if enabled, the host will release its clock hold if enabled, and complete the
	current transaction if SCL is cumulatively held low for greater than 10ms from
	START-to-ACK, ACK-to-ACK, or ACK-to-STOP
*/
struct TimeoutConfigHost_t {
	uint8_t enableMEXTTOEN:1;
	uint8_t	enableSEXTTOEN:1;
	uint8_t timeoutMode:2;
	uint8_t enableLOWTOUTEN:1;
	uint8_t :3;
};

struct TimeoutConfigClient_t {
	uint8_t :1;
	uint8_t	enableSEXTTOEN:1;
	uint8_t :2;
	uint8_t enableLOWTOUTEN:1;
	uint8_t :3;
};

enum I2C_BusMode {
	STANDARD_OR_FAST,	// standard mode or fast mode (up to 100 kHz, up to 400 kHz)
	FAST_PLUS,			// fast-mode+	(up to 1MHz): note that this mode requires a baud setting such that T_high : T_low = 1:2
	HIGH_SPEED			// high-speed mode	(up to 3.4MHz) : note that this mode requires a baud setting such that T_high : T_low = 1:2
};

/*
stretch_mode:	0 - stretch prior to ACK
				1 - stretch after ACK - high-speed mode requires this mode
bus_speed:		see I2C_BusMode
enable_sm:		enables smart mode, which automatically causes the device to ack/nack according to the setting in CTRLB.ACKACT 
				whenever the DATA register is read
baud:			used to time the high period of the serial clock (T_high), or both high and low if baudlow is zero. 
baudlow:		used to time the low period of the serial clock (T_low). 
*/
void I2C_InitHost(Sercom* sercom, _Bool stretch_mode, enum I2C_BusMode bus_speed, uint8_t sda_hold, _Bool enable_sm, uint8_t baud, uint8_t baudlow);


void I2C_InitClient(Sercom* sercom, _Bool stretch_mode, enum I2C_BusMode bus_speed, uint8_t sda_hold, uint8_t addr_mode, _Bool auto_addr_ack, _Bool enable_sm);

/* Calculate the baud settings required to achieve the specified SCL frequency given the serial clock core clock frequency, if possible 
	gclk_freq: frequency of the serial engine clock generator (GCLK_SERCOMx_CORE), in kHz
	target_sclk_freq: target SCLK frequency/I2C bit rate, in kHz
	baud: pointer to variable where baud setting will be stored
	baudlow pointer to variable where baudlow setting will be stored
*/
_Bool I2C_TryCalcBaud(uint16_t gclk_freq, uint16_t target_sclk_freq, uint8_t* baud, uint8_t* baudlow);
void I2C_Reset(Sercom* sercom);
void I2C_SetEnabled(Sercom* sercom, _Bool setEnabled);

size_t I2CHost_SendData(Sercom* i2c, uint8_t addr, uint8_t* txBuffer, size_t len);

#endif