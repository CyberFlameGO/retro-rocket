#pragma once

#include <kernel.h>

// PCI vendor/device ID for RTL8139
#define RTL8139_VENDOR_ID	0x10EC
#define RTL8139_DEVICE_ID	0x8139

// Receive buffer size
#define RX_BUF_SIZE 8192

#define CAPR			0x38
#define RX_READ_POINTER_MASK	(~3)
#define ROK			(1<<0)
#define RER			(1<<1)
#define TOK	 		(1<<2)
#define TER			(1<<3)
#define TX_TOK			(1<<15)

/**
 * @brief Port registers for RTL8139
 * https://datasheetspdf.com/pdf-file/1092361/RealtekMicroelectronics/RTL8139B/1
 * 5. Register Descriptions
 */
enum RTL8139_registers {
	MAG0			= 0x00, // Ethernet hardware address
	MAR0			= 0x08, // Multicast filter
	TxStatus0		= 0x10, // Transmit status (Four 32bit registers)
	TxAddr0		  	= 0x20, // Tx descriptors (also four 32bit)
	RxBuf			= 0x30,
	RxEarlyCnt	   	= 0x34,
	RxEarlyStatus		= 0x36,
	ChipCmd			= 0x37,
	RxBufPtr		= 0x38,
	RxBufAddr		= 0x3A,
	IntrMask		= 0x3C,
	IntrStatus	   	= 0x3E,
	TxConfig		= 0x40,
	RxConfig		= 0x44,
	Timer			= 0x48,	// A general-purpose counter
	RxMissed		= 0x4C,	// 24 bits valid, write clears
	Cfg9346		  	= 0x50,
	Config0		  	= 0x51,
	Config1		  	= 0x52,
	FlashReg		= 0x54,
	GPPinData		= 0x58,
	GPPinDir		= 0x59,
	MII_SMI		  	= 0x5A,
	HltClk		   	= 0x5B,
	MultiIntr		= 0x5C,
	TxSummary		= 0x60,
	MII_BMCR		= 0x62,
	MII_BMSR		= 0x64,
	NWayAdvert	   	= 0x66,
	NWayLPAR		= 0x68,
	NWayExpansion		= 0x6A,
	// Undocumented registers, but required for proper operation
	FIFOTMS		  	= 0x70,	// FIFO Control and test
	CSCR			= 0x74,	// Chip Status and Configuration Register
	PARA78		   	= 0x78,
	PARA7c		   	= 0x7C,	// Magic transceiver parameter register
};

/**
 * @brief Transfer description
 */
typedef struct tx_desc {
	uint32_t phys_addr;
	uint32_t packet_size;
}tx_desc_t;

/**
 * @brief PCI device configuration
 */
typedef struct rtl8139_dev {
	uint8_t bar_type;
	uint16_t io_base;
	uint32_t mem_base;
	int eeprom_exist;
	uint8_t mac_addr[6]; // MAC address in binary form
	char mac_addr_str[16]; // MAC address in string form
	uint32_t rx_buffer;
	int tx_cur;
} rtl8139_dev_t;

/**
 * @brief Send a raw packet
 * 
 * @param data packet data
 * @param len packet length
 */
void rtl8139_send_packet(void* data, uint32_t len);

/**
 * @brief Handle RTL8139 interrupt
 * 
 * @param isr ISR number (32 + ISR)
 * @param error error code
 * @param irq Actual IRQ number
 */
void rtl8139_handler(uint8_t isr, uint64_t error, uint64_t irq);

/**
 * @brief Initialise RTL8139
 * 
 * @return true card detected and initialised
 * @return false card not detected, or would not reset
 */
bool rtl8139_init();

/**
 * @brief Get the MAC address of the card in display format
 * 
 * @return char* MAC address
 */
char* read_mac_addr();

/**
 * @brief Handle receipt of packet from ISR
 */
void receive_packet();

/**
 * @brief Get the mac address from the IO ports
 */
void get_mac_addr();

/**
 * @brief A periodic timer fired by the task scheduler.
 * This is a hack to regulate interrupts to a sane level on qemu.
 */
void rtl8139_timer();