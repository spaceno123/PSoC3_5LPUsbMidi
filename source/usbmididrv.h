/*
	USB MIDI Driver Header File
*/
#ifndef USBMIDIDRV_H
#define	USBMIDIDRV_H

void usbmididrv_ini(void);
short TransmitUsbMidiIn(unsigned long ulData);
unsigned char ReceiveUsbMidiOut(unsigned long **buf);
short get_uart_rx_buf(void);
short put_uart_tx_buf(unsigned char ubData);

#endif	/* USBMIDIDRV_H */

