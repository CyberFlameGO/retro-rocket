/**
 * @file kernel.h
 * @author Craig Edwards (craigedwards@brainbox.cc)
 * @copyright Copyright (c) 2012-2023
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

#define kprintf printf

#include <limine.h>
#include "idt.h"
#include "spinlock.h"
#include "printf.h"
#include "hashmap.h"
#include "video.h"
#include "string.h"
#include "io.h"
#include "memcpy.h"
#include "apic.h"
#include "ioapic.h"
#include "timer.h"
#include "kmalloc.h"
#include "pci.h"
#include "maths.h"
#include "devicename.h"
#include "interrupt.h"
#include "ahci.h"
#include "ata.h"
#include "filesystem.h"
#include "partition.h"
#include "ramdisk.h"
#include "iso9660.h"
#include "devfs.h"
#include "fat32.h"
#include "debugger.h"
#include "errorhandler.h"
#include "keyboard.h"
#include "input.h"
#include "clock.h"
#include "lapic_timer.h"
#include "ubasic.h"
#include "tokenizer.h"
#include "taskswitch.h"
#include "acpi.h"
#include "net.h"
#include "rtl8139.h"
#include "e1000.h"
#include "arp.h"
#include "ip.h"
#include "ethernet.h"
#include "tcp.h"
#include "icmp.h"
#include "udp.h"
#include "dhcp.h"
/**
 * @file kernel.h
 * @author Craig Edwards (craigedwards@brainbox.cc)
 * @copyright Copyright (c) 2012-2023
 */
#include "dns.h"
#include "drawing.h"
#include "fpu.h"

#define assert(expr, line) if (!(expr)) { \
	kprintf("Assertion failure at %s:%s: %s", line, __FILE__,__LINE__); \
	wait_forever(); }

void network_up();
void network_down();
