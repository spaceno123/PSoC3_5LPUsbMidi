/*
	Program	usbmidimain.c
	Date	2012/05/01 .. 2012/05/01	(PSoC1)
	Copyright (C) 2012 by AKIYA
	--- up date ---
*/
#include "usbmidimain.h"
#include "usbmididrv.h"
#include "usbmidi.h"

/* --- midi cable number --- */
#define USBMIDI_IN_CN	(0)
#define USBMIDI_OUT_CN	(0)

/* --- send uart call back function --- */
static void send_midi(unsigned char ubData)
{
	while (put_uart_tx_buf(ubData) != 0) ;	// blocking !

	return;
}

/* --- stream to/from packed work --- */
static SSTREAMMIDI sStrMidi = {0,0,0,0};
static SPACKETMIDI sPacMidi = {send_midi,0,0,0};

/* --- in ep service --- */
static void usbmidi_in_ep_service(void)
{
	static unsigned char phase = 0;
	static SUSBMIDI sUsbMidi;
	short wData;

	switch (phase) {
	case 0:
		wData = get_uart_rx_buf();
		if (wData >= 0) {
			sUsbMidi.ulData = StreamToPacket(&sStrMidi, wData);
			if (sUsbMidi.ulData) {
				SetUsbMidiCn(sUsbMidi.sPacket.CN_CIN, USBMIDI_IN_CN);
				if (TransmitUsbMidiIn(sUsbMidi.ulData) != 0) {
					phase = 1;
				}
			}
		}
		break;
	case 1:
		if (TransmitUsbMidiIn(sUsbMidi.ulData) == 0) {
			phase = 0;
		}
		break;
	}

	return;
}

/* --- out ep service --- */
static void usbmidi_out_ep_service(void)
{
	unsigned long *buf;
	unsigned char count = ReceiveUsbMidiOut(&buf);

	while (count--) {
		SUSBMIDI sUsbMidi;

		sUsbMidi.ulData = *buf++;
		if (GetUsbMidiCn(sUsbMidi.sPacket.CN_CIN) == USBMIDI_OUT_CN) {
			PacketToStream(&sPacMidi, sUsbMidi.ulData);
		}
	}

	return;
}

/* --- usb midi main routine ---- */
void usbmidimain(void)
{
	usbmididrv_ini();	// wait usb enumerate

	while (1) {
		usbmidi_in_ep_service();
		usbmidi_out_ep_service();
	}
}
