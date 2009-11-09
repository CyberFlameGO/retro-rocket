#include "../include/io.h"
#include "../include/kernel.h"
#include "../include/video.h"
#include "../include/printf.h"
#include "../include/interrupts.h"
#include "../include/errorhandler.h"
#include "../include/keyboard.h"
#include "../include/timer.h"
#include "../include/paging.h"
#include "../include/kmalloc.h"
#include "../include/ata.h"
#include "../include/iso9660.h"
#include "../include/filesystem.h"

#define MULTIBOOT_MAGIC 0x2BADB002

console* current_console = NULL;

void _memset(void *dest, char val, int len)
{
	char *temp = (char *)dest;
	for ( ; len != 0; len--) *temp++ = val;
}

void kmain(void* mbd, unsigned int magic)
{
	u32int memorysize = 0;
	if (magic == MULTIBOOT_MAGIC)
	{
		init_gdt();
		init_idt();
		init_error_handler();
		init_basic_keyboard();
		memorysize = init_paging(mbd);
		init_timer(50);
		interrupts_on();
	}

	console* cons = (console*)kmalloc(sizeof(console));
	initconsole(cons);
	current_console = cons;

	if (magic != MULTIBOOT_MAGIC)
	{
		printf("Invalid magic number %x from multiboot. System halted.\n", magic);
		blitconsole(current_console);
		wait_forever();
	}
	else
	{    
		/* You could either use multiboot.h */
		/* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
		/* or do your offsets yourself. The following is merely an example. */ 
		//char * boot_loader_name =(char*) ((long*)mbd)[16];

		printf("Sixty-Four kernel booting from %s...\n%dMb usable RAM detected.\n", (const char*)((long*)mbd)[16], memorysize / 1024 / 1024);

		ide_initialize();

		asm volatile("int $50");

		putstring(current_console, "VFS/ISO9660 tests, mounting filesystem on drive 0\n");
		iso9660* iso = iso_mount_volume(0);

		u32int directory_entries;
		LINKED_LIST_COUNT(FS_DirectoryEntry*, iso->root, directory_entries);

		FS_DirectoryEntry* n;
		printf("VFS dir of %d files:\n", directory_entries);
		for(n = iso->root; n->next; n = n->next)
			printf("    %s: size=%d flags=0x%02x\n", n->filename, n->size, n->flags);

		putstring(current_console, "Now change dir to 'boot'...\n");

		printf("iso_change_directory(): %s\n", iso_change_directory(iso, "boot") ? "success" : "failure");

		LINKED_LIST_COUNT(FS_DirectoryEntry*, iso->root, directory_entries);
		printf("VFS dir of %d files:\n", directory_entries);
		for(n = iso->root; n->next; n = n->next)
			printf("    %s: size=%d flags=0x%02x\n", n->filename, n->size, n->flags);

		FREE_LINKED_LIST(FS_DirectoryEntry*, iso->root);
		kfree(iso);

		wait_forever();
	}
}

