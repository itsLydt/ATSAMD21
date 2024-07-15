/*
 * spi.h
 *
 * Created: 7/2/2024 12:04:07 PM
 *  Author: itsLydt
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "sam.h"

/* enables/configures the clocks used by the SPI peripheral
sercom_num: 0-5; indicates which sercom instance is to be used
setBusClkEnabled: enables or disables the SERCOMx bus clock (CLK_SERCOMx_APB)
coreClkGenerator: 0-7 or -1: indicates which clock generator will supply the clock signal for the core clock (GCLK_SERCOMx_CORE)
			set to -1 to disable this clock
 */
void SPI_ClkControl(uint8_t sercom_num, bool setBusClkEnabled, int8_t clkGenerator);

void SPI_InitHost(Sercom* sercom, uint8_t transfer_mode, uint8_t baud, bool hardwareSS);
void SPI_InitClient(Sercom* sercom, uint8_t transfer_mode, int8_t addressMode, uint8_t address, uint8_t addressMask);

void SPI_Reset(Sercom* sercom);
void SPI_SetEnabled(Sercom* sercom, bool setEnabled);


/* Blocking calls for transmit and receive */
void SPI_SendData(Sercom* spi, uint8_t* txBuffer, uint8_t* rxBuffer, size_t len);  //TODO: size_t max is larger than SPI_DATA buffer
void SPI_ReceiveData(Sercom* spi, uint8_t* rxBuffer, size_t len);

/* Nonblocking calls for transmit and receive */
bool SPI_IsBusy();
bool SPI_BeginSendData(Sercom* spi, uint8_t* txBuffer, size_t len);
bool SPI_BeginReceiveData(Sercom* spi, size_t len);
size_t SPI_FinishReceiveData(Sercom* spi, uint8_t* rxBuffer);

void SPI_SetTXCIntEnabled(Sercom* spi, bool setEnabled);
void SPI_SetRXCIntEnabled(Sercom* spi, bool setEnabled);
void SPI_SetDREIntEnabled(Sercom* spi, bool setEnabled);
void SPI_SetSSLIntEnabled(Sercom* spi, bool setEnabled);
void SPI_SetErrIntEnabled(Sercom* spi, bool setEnabled);

void SPI_InterruptHandler(Sercom* spi);

//void SPI_LockConfiguration();

#endif /* SPI_H_ */