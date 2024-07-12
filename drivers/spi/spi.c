/*
 * spi.c
 *
 * Created: 7/2/2024 12:04:20 PM
 *  Author: itsLydt
 */ 

#include "spi.h"
#define SPI_BUFFER_LEN UINT8_MAX
struct SPI_BUFFERS {
	bool isBusy;
	uint8_t tx_index;
	uint8_t rx_index;
	uint8_t dataLen;
	char txBuffer[SPI_BUFFER_LEN];
	char rxBuffer[SPI_BUFFER_LEN];
} SPI_DATA;

/*
SERCOM number: 0-5
setClocksEnabled: true to enable, false to disable
clkGenerator: select clock generator, 0-8
Note that clock generator must be configured separately
*/
void SPI_ClkControl(uint8_t sercom_num, bool setBusClockEnabled, uint8_t serialClkGenerator){
	// enable/disable APBC bus clock and peripheral clock for specified SERCOM instance
	
	uint32_t bus_mask = (0x01 << (2 + sercom_num)); // SERCOM0: bit2, SERCOM1: bit3, etc
	uint32_t clk_ctrl = GCLK_CLKCTRL_GEN(serialClkGenerator) | GCLK_CLKCTRL_ID(0x14 + sercom_num); // SERCOM0: ID=14, SERCOM1: ID:15, etc

	if(setBusClockEnabled){
		PM->APBCMASK.reg |= bus_mask;
	}
	else {
		PM->APBCMASK.reg &= ~bus_mask;
	}
	
	if(serialClkGenerator != -1)
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | clk_ctrl;
	else 
		GCLK->CLKCTRL.reg = clk_ctrl;
}

void SPI_commonInit(Sercom* sercom, bool isHost, uint8_t transfer_mode){
	SPI_DATA.isBusy = false;
	SPI_DATA.dataLen = 0;
	SPI_DATA.tx_index = 0;
	SPI_DATA.rx_index = 0;
	
	sercom->SPI.CTRLA.bit.MODE = isHost? SERCOM_SPI_CTRLA_MODE_SPI_MASTER_Val : SERCOM_SPI_CTRLA_MODE_SPI_SLAVE_Val;
	sercom->SPI.CTRLA.bit.DOPO = 0;  //PAD[0] Data out, PAD[1] SCK, PAD[2] SS
	sercom->SPI.CTRLA.bit.DIPO = 3;  //PAD[3] Data in
	sercom->SPI.CTRLA.bit.DORD = 0; // MSB first

	sercom->SPI.CTRLA.bit.CPHA = transfer_mode % 2;
	sercom->SPI.CTRLA.bit.CPOL = transfer_mode >= 2? 1 : 0;

	sercom->SPI.CTRLB.bit.CHSIZE = 0; //chars are 8 bits
}

// baud: value calculated via f_ref/(2*f_baud) - 1, where f_ref is speed of serial clock (GCLK_SERCOM0_CORE) and f_baud is desired baud rate
// AKA f_baud = f_ref /(2*(baud+1))
// f_baud must be <= 0.5*f_ref
void SPI_InitHost(Sercom* sercom, uint8_t transfer_mode, uint8_t baud, bool hardwareSS){
	SPI_SetEnabled(sercom, false);			// disable SERCOMx so configuration is possible

	SPI_commonInit(sercom, true, transfer_mode);

	sercom->SPI.BAUD.reg = SERCOM_SPI_BAUD_BAUD(baud);
	sercom->SPI.CTRLB.bit.MSSEN = hardwareSS? 1 : 0;
	
	sercom->SPI.CTRLB.bit.RXEN = 1;			// enable receiver
	SPI_SetEnabled(sercom, true);			// enable peripheral
}

// if address mode is -1, address matching will be turned off
void SPI_InitClient(Sercom* sercom, uint8_t transfer_mode, int8_t addressMode, uint8_t address, uint8_t addressMask){
	SPI_SetEnabled(sercom, false);			// disable SERCOMx so configuration is possible

	SPI_commonInit(sercom, false, transfer_mode);

	if(addressMode == -1){
		sercom->SPI.CTRLA.bit.FORM = 0; // normal SPI frame
	}
	else {
		sercom->SPI.CTRLA.bit.FORM = 2; // SPI frame with address
		sercom->SPI.CTRLB.bit.AMODE = addressMode;
		sercom->SPI.ADDR.reg = SERCOM_SPI_ADDR_ADDRMASK(addressMask) | SERCOM_SPI_ADDR_ADDR(address);
	}
	sercom->SPI.CTRLB.bit.SSDE = 1;
	sercom->SPI.CTRLB.bit.PLOADEN = 1;

	sercom->SPI.CTRLB.bit.RXEN = 1;			// enable receiver
	SPI_SetEnabled(sercom, true);			// enable peripheral
	
}


void SPI_Reset(Sercom* sercom){
	sercom->SPI.CTRLA.bit.SWRST = 1; // resets all registers of the sercom peripheral and disables it
	SPI_DATA.isBusy = false;
	SPI_DATA.dataLen = 0;
	SPI_DATA.tx_index = 0;
	SPI_DATA.rx_index = 0;
		
	while(sercom->SPI.SYNCBUSY.bit.SWRST);
}

void SPI_SetEnabled(Sercom* sercom, bool setEnabled){
	sercom->SPI.CTRLA.bit.ENABLE = setEnabled? 1 : 0;
	while(sercom->SPI.SYNCBUSY.bit.ENABLE);
}

/* recall that for SPI, each byte of data sent will result in one byte being received */
void SPI_SendData(Sercom* spi, uint8_t* txBuffer, uint8_t* rxBuffer, size_t len){
	for(int i = 0; i < len; i++){
		//wait for tx buffer to empty
		while(spi->SPI.INTFLAG.bit.DRE == 0); //TODO: WDT
		//load DR with next byte
		if(txBuffer) {
			spi->SPI.DATA.reg = txBuffer[i];
		}
		else {
			//write a zero
			spi->SPI.DATA.reg = 0;	// TODO: configurable dummy byte?
		}
			
		//wait for transmit complete
		while(spi->SPI.INTFLAG.bit.TXC == 0);

		if(rxBuffer){ // don't read if rxBuffer is null
			// wait for byte received
			while(spi->SPI.INTFLAG.bit.RXC == 0);
			//read data
			rxBuffer[i] = spi->SPI.DATA.reg;
		}
	}
}

/* because of this, to read data as host, we must send data. Client can simply read, but there is no harm in sending data as well. */
void SPI_ReceiveData(Sercom* spi, uint8_t* rxBuffer, size_t len){
	SPI_SendData(spi, NULL, rxBuffer, len); // no txbuffer because we are not sending "real" data
}

/* Nonblocking calls for transmit and receive */
void SPI_BeginSendData(Sercom* spi, uint8_t* txBuffer, uint8_t* rxBuffer, size_t len){
	while(SPI_DATA.isBusy); //wait until previous transaction finished
	
	SPI_DATA.isBusy = true;
	memcpy(SPI_DATA.txBuffer, txBuffer, len); // copy data into txBuffer
	SPI_DATA.tx_index = 0;
	SPI_DATA.rx_index = 0;
	SPI_DATA.dataLen = len;
	
	// enable interrupt routines to handle data transfer
	SPI_SetDREIntEnabled(spi, true);
	SPI_SetRXCIntEnabled(spi, true);
}
void SPI_BeginReceiveData(Sercom* spi, uint8_t* rxBuffer, size_t len);

void SPI_SetTXCIntEnabled(Sercom* spi, bool setEnabled){
	if(setEnabled)
		spi->SPI.INTENSET.bit.TXC = 1;
	else
		spi->SPI.INTENCLR.bit.TXC = 1;
}
void SPI_SetRXCIntEnabled(Sercom* spi, bool setEnabled){
	if(setEnabled)
	spi->SPI.INTENSET.bit.RXC = 1;
	else
	spi->SPI.INTENCLR.bit.RXC = 1;
}

void SPI_SetDREIntEnabled(Sercom* spi, bool setEnabled){
	if(setEnabled)
		spi->SPI.INTENSET.bit.DRE = 1;
	else
		spi->SPI.INTENCLR.bit.DRE = 1;
}
void SPI_SetSSLIntEnabled(Sercom* spi, bool setEnabled){
	if(setEnabled)
		spi->SPI.INTENSET.bit.SSL = 1;
	else
		spi->SPI.INTENCLR.bit.SSL = 1;
}
void SPI_SetErrIntEnabled(Sercom* spi, bool setEnabled){
	if(setEnabled)
		spi->SPI.INTENSET.bit.ERROR = 1;
	else
		spi->SPI.INTENCLR.bit.ERROR = 1;
}

void SPI_InterruptHandler(Sercom* spi){

	// determine which interrupt occurred
	if(spi->SPI.INTFLAG.bit.DRE){ // ready to send next byte
		if(SPI_DATA.tx_index >= SPI_DATA.dataLen){
			SPI_SetDREIntEnabled(spi, false);
			return;
		}
		spi->SPI.DATA.reg = SPI_DATA.txBuffer[SPI_DATA.tx_index++]; // writing to DATA clears TXC flag
		return;
	}
	
	if(spi->SPI.INTFLAG.bit.RXC){
		if(SPI_DATA.rx_index >= SPI_DATA.dataLen){
			// sent all data
			SPI_SetRXCIntEnabled(spi, false);
			SPI_DATA.isBusy = false;
			return;
		}
		SPI_DATA.rxBuffer[SPI_DATA.rx_index++] = spi->SPI.DATA.reg; //reading DATA clears RXC flag
		return;
	}
}