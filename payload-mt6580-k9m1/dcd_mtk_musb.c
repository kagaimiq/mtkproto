/* "MUSBMHDRC - Mediatek edition" DCD for TinyUSB
 *  by Kagami Hiiragi, 2021
 */
#include <log.h>
#include "tusb_option.h"

#if TUSB_OPT_DEVICE_ENABLED
#include "common/tusb_fifo.h"
#include "device/dcd.h"

typedef struct {
	/*---- Common USB Registers ----*/
	volatile uint8_t FAddr;
	volatile uint8_t Power;
	volatile uint16_t IntrTx;
	volatile uint16_t IntrRx;
	volatile uint16_t IntrTxE;
	volatile uint16_t IntrRxE;
	volatile uint8_t IntrUSB;
	volatile uint8_t IntrUSBE;
	volatile uint16_t Frame;
	volatile uint8_t Index;
	volatile uint8_t TestMode;
	
	/*---- Indexed Registers ----*/
	volatile uint16_t TxMaxP;
	union {
		struct	{
			volatile uint8_t CSR0L;
			volatile uint8_t CSR0H;
		};
		struct	{
			volatile uint8_t TxCSRL;
			volatile uint8_t TxCSRH;
		};
	};
	volatile uint16_t RxMaxP;
	volatile uint8_t RxCSRL;
	volatile uint8_t RxCSRH;
	union {
		volatile uint16_t Count0;
		volatile uint16_t RxCount;
	};
	union {
		volatile uint8_t Type0;
		volatile uint8_t TxType;
	};
	union {
		volatile uint8_t NAKLimit0;
		volatile uint8_t TxInterval;
	};
	volatile uint8_t RxType;
	volatile uint8_t RxInterval;
	uint8_t Reserved_1E;
	union {
		volatile uint8_t ConfigData;
		volatile uint8_t FIFOSize;
	};
	
	/* ----- FIFOs ----- */
	volatile uint32_t FIFOx[16];
	
	/* ----- Additional Control & Configuration Registers ----- */
	volatile uint8_t DevCtl;
	volatile uint8_t MISC;
	volatile uint8_t TxFIFOsz;
	volatile uint8_t RxFIFOsz;
	volatile uint16_t TxFIFOadd;
	volatile uint16_t RxFIFOadd;
	union {
		volatile uint32_t VControl;
		volatile uint32_t VStatus;
	};
	volatile uint16_t HWVers;
	uint16_t Reserved_6E;
	volatile uint8_t XXX_ulpi_regs[8];
	volatile uint8_t EPInfo;
	volatile uint8_t RAMInfo;
	volatile uint8_t LinkInfo;
	volatile uint8_t VPLen;
	volatile uint8_t HS_EOF1;
	volatile uint8_t FS_EOF1;
	volatile uint8_t LS_EOF1;
	volatile uint8_t SOFT_RST;
	
	/* ----- Medaitek Registers ----- */
	volatile uint16_t RxTog;
	volatile uint16_t RxTogEn;
	volatile uint16_t TxTog;
	volatile uint16_t TxTogEn;
	uint8_t Reserved_88[24];
	volatile uint32_t L1IntS;
	volatile uint32_t L1IntM;
} musb_regs_t;


#define RHPORT_REGS_BASE	0x11100000
#define RHPORT_IRQn		32

#define MUSB_BASE(_port)	((musb_regs_t *)RHPORT_REGS_BASE)

struct ep_io_thing {
	void *ptr;
	uint16_t len;
	uint16_t total;
};

static struct ep_io_thing eptx_io[16], eprx_io[16];

/* -------------- Basics --------------- */
void dcd_init(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	
	regs->Power |= (1<<5); /* High speed! */
	
	/* Connect */
	dcd_disconnect(rhport);
	delay(100);
	dcd_connect(rhport);
}

void dcd_int_enable(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->L1IntM = 0x7;
}

void dcd_int_disable(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->L1IntM = 0x0;
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->FAddr = dev_addr;
	
	/* Do it by yourself
	 *           -- Ha Thach
	 */
	dcd_edpt_xfer(rhport, tu_edpt_addr(0, TUSB_DIR_IN), NULL, 0);
}

void dcd_remote_wakeup(uint8_t rhport) {
	logd("TUSB_DCD", "Remote WakeUp!! %d", rhport);
}

void dcd_connect(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Power |= (1<<6);
}

void dcd_disconnect(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Power &= ~(1<<6);
}

/* -------------- Endpoints --------------- */
static void ep_fifo_put(uint8_t rhport, int ep, void *ptr, uint16_t len) {
	musb_regs_t *regs = MUSB_BASE(rhport);

	while (len > 0) {
		/* WAIT!!! FIXME!! what if ptr is not aligned to word boundary ???! */
		
		if (len >= 4) {
			/* word */
			*(volatile uint32_t*)&regs->FIFOx[ep] = *(uint32_t*)ptr;
			ptr += 4; len -= 4;
		} else if (len >= 2) {
			/* halfword */
			*(volatile uint16_t*)&regs->FIFOx[ep] = *(uint16_t*)ptr;
			ptr += 2; len -= 2;
		} else {
			/* byte */
			*(volatile uint8_t*)&regs->FIFOx[ep] = *(uint8_t*)ptr;
			ptr += 1; len -= 1;
		}
	}
}

static void ep_fifo_get(uint8_t rhport, int ep, void *ptr, uint16_t len) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	
	while (len > 0) {
		/* WAIT!!! FIXME!! what if ptr is not aligned to word boundary ???! */
	
		if (len >= 4) {
			/* word */
			*(uint32_t*)ptr = *(volatile uint32_t*)&regs->FIFOx[ep];
			ptr += 4; len -= 4;
		} else if (len >= 2) {
			/* halfword */
			*(uint16_t*)ptr = *(volatile uint16_t*)&regs->FIFOx[ep];
			ptr += 2; len -= 2;
		} else {
			/* byte */
			*(uint8_t*)ptr = *(volatile uint8_t*)&regs->FIFOx[ep];
			ptr += 1; len -= 1;
		}
	}
}

static void do_edpt_tx(uint8_t rhport, char ep) {
	struct ep_io_thing *epio = &eptx_io[ep];
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = ep;
	
	if (ep == 0) {
		uint16_t ndata = (epio->len > 64)?64:epio->len;
		if (ndata > 0) {
			ep_fifo_put(rhport, ep, epio->ptr + epio->total, ndata);
			epio->len -= ndata;
			epio->total += ndata;
			regs->CSR0L |= (1<<1); /* TxPktRdy */
		}
		
		if (epio->len == 0) {
			regs->CSR0L |= (1<<3); /* DataEnd */
			ep0_fsm = 0;
		}
	} else {
		uint16_t ndata = (epio->len > regs->TxMaxP)?regs->TxMaxP:epio->len;
		if (ndata > 0) {
			ep_fifo_put(rhport, ep, epio->ptr + epio->total, ndata);
			epio->len -= ndata;
			epio->total += ndata;
			regs->TxCSRL |= (1<<0); /* TxPktRdy */
			
			if (epio->len == 0) {
				dcd_event_xfer_complete(rhport, tu_edpt_addr(ep, TUSB_DIR_IN), epio->total, XFER_RESULT_SUCCESS, true);
			}
		}
	}
}

static void do_edpt_rx(uint8_t rhport, char ep) {
	struct ep_io_thing *epio = &eprx_io[ep];
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = ep;
	
	if (ep == 0) {
		/* TODO */
	} else {
		uint16_t ndata = (epio->len > regs->RxCount)?regs->RxCount:epio->len;
		if (ndata > 0) {
			ep_fifo_get(rhport, ep, epio->ptr + epio->total, ndata);
			epio->len -= ndata;
			epio->total += ndata;
			regs->RxCSRL &= ~(1<<0); /* RxPktRdy */
			
			if (epio->len == 0) {
				dcd_event_xfer_complete(rhport, tu_edpt_addr(ep, TUSB_DIR_OUT), epio->total, XFER_RESULT_SUCCESS, true);
			}
		}
	}
}

bool dcd_edpt_open(uint8_t rhport, const tusb_desc_endpoint_t *desc_edpt) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	const uint8_t epnum = tu_edpt_number(desc_edpt->bEndpointAddress);
	const uint8_t dir   = tu_edpt_dir(desc_edpt->bEndpointAddress);
	
	/* enable interrupt */
	if (dir == TUSB_DIR_IN || epnum == 0) {
		/* TX endpoint or the Endpoint Zero */
		regs->IntrTxE |= (1<<epnum);
	} else {
		/* RX endpoint */
		regs->IntrRxE |= (1<<epnum);
	}
	
	return true;
}

void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	const uint8_t epnum = tu_edpt_number(ep_addr);
	const uint8_t dir   = tu_edpt_dir(ep_addr);
	
	/* disable interrupt */
	if (dir == TUSB_DIR_IN || epnum == 0) {
		/* TX endpoint or the Endpoint Zero */
		regs->IntrTxE &= ~(1<<epnum);
	} else {
		/* RX endpoint */
		regs->IntrRxE &= ~(1<<epnum);
	}
}

void dcd_edpt_close_all(uint8_t rhport) {
	logd("TUSB_DCD", "Close All EndPoint!! %d", rhport);
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes) {
	const uint8_t epnum = tu_edpt_number(ep_addr);
	const uint8_t dir   = tu_edpt_dir(ep_addr);
	
	if (dir == TUSB_DIR_IN) {
		eptx_io[epnum].ptr = buffer;
		eptx_io[epnum].len = total_bytes;
		eptx_io[epnum].total = 0;
		
		/* TX first packet */
		do_edpt_tx(rhport, epnum);
	} else {
		eprx_io[epnum].ptr = buffer;
		eprx_io[epnum].len = total_bytes;
		eprx_io[epnum].total = 0;
	}

	return true;
}

void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr) {
	const uint8_t epnum = tu_edpt_number(ep_addr);
	const uint8_t dir   = tu_edpt_dir(ep_addr);
	
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = epnum;
	
	if (epnum == 0) {
		/* ep0 is no matter in or out */
		if (dir == TUSB_DIR_IN)
			regs->CSR0L |= (1<<5);
	} else {
		if (dir == TUSB_DIR_IN) {
			regs->TxCSRL |= (1<<4);
		} else {
			regs->RxCSRL |= (1<<5);
		}
	}
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr) {
	const uint8_t epnum = tu_edpt_number(ep_addr);
	const uint8_t dir   = tu_edpt_dir(ep_addr);
	
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = epnum;
	
	if (epnum == 0) {
		/* ep0 doesn't support stall clearing */
	} else {
		if (dir == TUSB_DIR_IN) {
			regs->TxCSRL &= ~(1<<4);
		} else {
			regs->RxCSRL &= ~(1<<5);
		}
	}
}

/* -------------- Interrupts --------------- */
static void handle_ep0_int(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = 0;
	
	if (regs->CSR0L & (1<<2)) { /* SentStall */
		regs->CSR0L &= ~(1<<2);
		ep0_fsm = 0;
	}
	
	if (regs->CSR0L & (1<<4)) { /* SetupEnd */
		regs->CSR0L |= (1<<7);
		ep0_fsm = 0;
	}
	
	if (eptx_io[0].len > 0) {
		/* TX */
		do_edpt_tx(rhport, 0);
	} else if (eprx_io[0].len > 0) {
		/* RX */
		do_edpt_rx(rhport, 0);
	} else {
		/* Idle */
		if (regs->CSR0L & (1<<0)) { /* RxPktRdy */
			uint32_t pkt[2];
			for (int i = 0; i < 2; i++) pkt[i] = regs->FIFOx[0];
			dcd_event_setup_received(rhport, (uint8_t*)&pkt, true);
			
			regs->CSR0L |= (1<<6);
		}
	}
}

static void handle_eptx_int(uint8_t rhport, char ep) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = ep;
	
	if (regs->TxCSRL & (1<<5)) { /* SentStall */
		regs->TxCSRL &= ~(1<<5);
	}
	
	do_edpt_tx(rhport, ep);
}

static void handle_eprx_int(uint8_t rhport, char ep) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	regs->Index = ep;
	
	logv("Tusb_dcd", "!! EPRX Int %d\n", ep);
	
	if (regs->RxCSRL & (1<<6)) { /* SentStall */
		regs->RxCSRL &= ~(1<<6);
	}
	
	do_edpt_rx(rhport, ep);
}

void dcd_int_handler(uint8_t rhport) {
	musb_regs_t *regs = MUSB_BASE(rhport);
	uint16_t pend;
	
	if (regs->L1IntS & regs->L1IntM & (1<<2)) {
		pend = regs->IntrUSB;
		regs->IntrUSB = pend;
		
		if (pend & (1<<1)) { /* Resume */
			dcd_event_bus_signal(rhport, DCD_EVENT_RESUME, true);
		}

		if (pend & (1<<0)) { /* Suspend */
			dcd_event_bus_signal(rhport, DCD_EVENT_SUSPEND, true);
		}
		
		if (pend & (1<<5)) { /* Disconnect */
			dcd_event_bus_signal(rhport, DCD_EVENT_UNPLUGGED, true);
		}
		
		if (pend & (1<<2)) { /* Reset/Babble */
			/* Do it by yourself
			 *         -- Mediatek
			 */
			regs->Index = 0;
			regs->FAddr = 0;
			
			/* just for sure */
			ep0_fsm = 0;
			
			regs->IntrTxE = 1;
			regs->IntrRxE = 0;
			regs->IntrRx = 0xffff;
			regs->IntrTx = 0xffff;
			
			dcd_event_bus_reset(rhport, (regs->Power & (1<<4))?TUSB_SPEED_HIGH:TUSB_SPEED_FULL, true);
		}
		
		if (pend & (1<<3)) { /* SOF */
			dcd_event_bus_signal(rhport, DCD_EVENT_SOF, true);
		}
	}
	
	if (regs->L1IntS & regs->L1IntM & (1<<0)) {
		pend = regs->IntrTx;
		regs->IntrTx = pend;
		
		if (pend & (1<<0)) { /* Endpoint 0 RX/TX */
			handle_ep0_int(rhport);
		}
		
		for (int i = 1; i < 16; i++) {
			if (pend & (1<<i)) { /* Endpoint x TX */
				handle_eptx_int(rhport, i);
			}
		}
	}
	
	if (regs->L1IntS & regs->L1IntM & (1<<1)) {
		pend = regs->IntrRx;
		regs->IntrRx = pend;
		
		for (int i = 1; i < 16; i++) {
			if (pend & (1<<i)) { /* Endpoint x RX */
				handle_eprx_int(rhport, i);
			}
		}
	}
}

#endif
