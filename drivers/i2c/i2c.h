#ifndef I2C_H_
#define I2C_H_

#include "sam.h"

/* enables/configures the clocks used by the I2C peripheral
sercom_num: 0-5; indicates which sercom instance is to be used. Note that not all sercom pins can be used as I2C pins
setBusClkEnabled: enables or disables the SERCOMx bus clock (CLK_SERCOMx_APB)
coreClkGenerator: 0-7 or -1: indicates which clock generator will supply the clock signal for the core clock (GCLK_SERCOMx_CORE)
			set to -1 to disable this clock
slowClkGenerator: 0-7 or -1: indicates which clock generator will supply the clock signal for the slow clock (GCLK_SERCOM_SLOW)
			set to -1 to disable this clock
 */
void I2C_ClkControl(uint8_t sercom_num, bool setBusClkEnabled, int8_t coreClkGenerator, int8_t slowClkGenerator);
void I2C_Init();

void I2C_Reset(Sercom* sercom);
void I2C_SetEnabled(Sercom* sercom, bool setEnabled);

#endif