#include <kernel.h>

void error_handler(uint8_t int_no, uint64_t errorcode, uint64_t irq_no);

void init_error_handler()
{
	int interrupt = 0;
	for (; interrupt < 19; ++interrupt)
		register_interrupt_handler(interrupt, error_handler);
}

void error_handler(uint8_t int_no, uint64_t errorcode, uint64_t irq_no)
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
	//PANIC_BANNER;
	setforeground(current_console, COLOUR_LIGHTRED);
	kprintf("Fatal exception %02x: %s\n", int_no, error_table[int_no]);
	backtrace();
	asm volatile("cli");
	wait_forever();
}


