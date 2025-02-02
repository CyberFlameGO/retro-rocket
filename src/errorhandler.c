#include <kernel.h>
#include <homer.h>

void error_handler(uint8_t int_no, uint64_t errorcode, uint64_t irq_no, void* opaque);

void init_error_handler()
{
	int interrupt = 0;
	for (; interrupt < 19; ++interrupt)
		register_interrupt_handler(interrupt, error_handler, dev_zero, NULL);
}

void error_handler(uint8_t int_no, [[maybe_unused]] uint64_t errorcode, [[maybe_unused]] uint64_t irq_no, void* opaque)
{
	static const char* const error_table[] = {
		"Division by zero exception",
		"Debug exception",
		"Non maskable interrupt",
		"Breakpoint exception",
		"Into detected overflow",
		"Out of bounds exception",
		"Invalid opcode exception",
		"No coprocessor exception",
		"Double fault",
		"Coprocessor segment overrun",
		"Bad TSS",
		"Segment not present",
		"Stack fault",
		"General protection fault",
		"Page fault",
		"Unknown interrupt exception",
		"Coprocessor fault",
		"Alignment check exception",
		"Machine check exception",
	};
	interrupts_off();
	PANIC_BANNER;
	setforeground(current_console, COLOUR_LIGHTRED);
	kprintf("Fatal exception %02X: %s\n", int_no, error_table[int_no]);
	setforeground(current_console, COLOUR_WHITE);
	backtrace();
	wait_forever();
}


