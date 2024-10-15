/*
	Program	usbmididrv.c
	Date	2012/04/30 .. 2012/05/01	(PSoC1)
	Copyright (C) 2012 by AKIYA
	--- up date ---
*/
#include "usbmididrv.h"
#include "UART.h"
#include "USBFS.h"

/* --- for USB IN/OUT --- */
#define IN_EP			(2)
#define OUT_EP			(1)
#define EP_DATA_SIZE	(64)
#define EP_DATA_COUNT	(EP_DATA_SIZE/4)

static unsigned long in_buf[EP_DATA_COUNT];
static unsigned char in_buf_wc = 0;
static unsigned long out_buf[EP_DATA_COUNT];

/* --- initialize --- */
void usbmididrv_ini(void)
{
	USBFS_Start(0, USBFS_5V_OPERATION); //Start USBFS Operation using device 0 at 5V
	while(!USBFS_bGetConfiguration()); //Wait for Device to enumerate
	USBFS_EnableOutEP(OUT_EP);

	UART_Start();
}

/* --- Transmit To PC --- */
short TransmitUsbMidiIn(unsigned long ulData)
{

	if (in_buf_wc == EP_DATA_COUNT) {
		if (USBFS_GetEPState(IN_EP) != USBFS_IN_BUFFER_EMPTY) {
			return -1;
		}
		USBFS_LoadInEP(IN_EP, (unsigned char *)in_buf, EP_DATA_SIZE);
		in_buf_wc = 0;
	}

	in_buf[in_buf_wc++] = ulData;

	if (USBFS_GetEPState(IN_EP) == USBFS_IN_BUFFER_EMPTY) {
		while (in_buf_wc < EP_DATA_COUNT) {
			in_buf[in_buf_wc++] = 0;
		}
		USBFS_LoadInEP(IN_EP, (unsigned char *)in_buf, EP_DATA_SIZE);
		in_buf_wc = 0;
	}

	return 0;
}

/* --- Receive from PC --- */
unsigned char ReceiveUsbMidiOut(unsigned long **buf)
{
	unsigned char count;

	if (USBFS_GetEPState(OUT_EP) == USBFS_OUT_BUFFER_FULL) {
		count = USBFS_ReadOutEP(OUT_EP, (unsigned char *)out_buf, EP_DATA_SIZE);
		while (USBFS_GetEPState(OUT_EP) == USBFS_OUT_BUFFER_FULL);
		USBFS_EnableOutEP(OUT_EP);
	}
	else {
		count = 0;
	}
	if (buf) {
		*buf = out_buf;
	}
	return count/4;	// /4 for long count
}

/* --- get uart receive data --- */
short get_uart_rx_buf(void)
{
	short ret;
	
	if (UART_GetRxBufferSize()) {
		ret = UART_ReadRxData();
	}
	else {
		ret = -1;
	}

	return ret;
}

/* --- put uart transmit data --- */
short put_uart_tx_buf(unsigned char dat)
{
	short ret;

	if (UART_GetTxBufferSize() < (UART_TXBUFFERSIZE - 1)) {
		UART_WriteTxData(dat);
		ret = 0;
	}
	else {
		ret = -1;
	}

	return ret;
}
