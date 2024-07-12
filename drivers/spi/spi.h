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

#include "sam.h"

void SPI_ClkControl(uint8_t sercom_num, bool setBusClkEnabled, uint8_t clkGenerator);

void SPI_InitMaster(Sercom* sercom, uint8_t transfer_mode, uint8_t baud, bool hardwareSS);
void SPI_InitSlave(Sercom* sercom, uint8_t transfer_mode, int8_t addressMode, uint8_t address, uint8_t addressMask);

void SPI_Reset(Sercom* sercom);
void SPI_SetEnabled(Sercom* sercom, bool setEnabled);


/* Blocking calls for transmit and receive */
void SPI_SendData(Sercom* spi, uint8_t* txBuffer, uint8_t* rxBuffer, size_t len);
uint32_t SPI_ReceiveData(Sercom* spi, uint8_t* rxBuffer, size_t toRead);

void SPI_ConfigureInterupt();
void SPI_EnableInterrupt();
void SPI_DisableInterrupt();

//void SPI_LockConfiguration();

#endif /* SPI_H_ */