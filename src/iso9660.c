#include <kernel.h>
#include <filesystem.h>
#include <iso9660.h>

#define VERIFY_ISO9660(n) (n->standardidentifier[0] == 'C' && n->standardidentifier[1] == 'D' \
				&& n->standardidentifier[2] == '0' && n->standardidentifier[3] == '0' && n->standardidentifier[4] == '1')

fs_directory_entry_t* parse_directory(fs_tree_t* node, iso9660* info, uint32_t start_lba, uint32_t lengthbytes); 

/**
 * @brief Mount an ISO 9660 filesystem on a given block device name.
 *
 * Returns either NULL or an iso9660* which references the volume information and initially the
 * root directory of the disk.
 * 
 * @param device device name
 * @return iso9660* detail of root directory, or null on error
 */
iso9660* iso_mount_volume(const char* device);

bool iso_read_file(void* file, uint64_t start, uint32_t length, unsigned char* buffer);

static filesystem_t* iso9660_fs = NULL;

void parse_boot([[maybe_unused]] iso9660* info, [[maybe_unused]] unsigned char* buffer)
{
}

int parse_pvd(iso9660* info, unsigned char* buffer)
{
	PVD* pvd = (PVD*)buffer;
	if (!VERIFY_ISO9660(pvd))
	{
		kprintf("ISO9660: Invalid PVD found, identifier is not 'CD001'\n");
		return 0;
	}
	char* ptr = pvd->volumeidentifier + 31;
	for (; ptr != pvd->volumeidentifier && *ptr == ' '; --ptr)
	{
		// Backpeddle over the trailing spaces in the volume name
		if (*ptr == ' ')
			*ptr = 0;
	}

	int j = 0;
	info->volume_name = (char*)kmalloc(strlen(pvd->volumeidentifier) + 1);
	for (ptr = pvd->volumeidentifier; *ptr; ++ptr)
		info->volume_name[j++] = *ptr;
	// Null-terminate volume name
	info->volume_name[j] = 0;

	info->joliet = 0;
	info->pathtable_lba = pvd->lsb_pathtable_L_lba;
	info->rootextent_lba = pvd->root_directory.extent_lba_lsb;
	info->rootextent_len = pvd->root_directory.data_length_lsb;
	info->root = parse_directory(NULL, info, pvd->root_directory.extent_lba_lsb, pvd->root_directory.data_length_lsb);

	return info->root != 0;
}

fs_directory_entry_t* parse_directory(fs_tree_t* node, iso9660* info, uint32_t start_lba, uint32_t lengthbytes)
{
	unsigned char* dirbuffer = (unsigned char*)kmalloc(lengthbytes);
	int j;

	_memset(dirbuffer, 0, lengthbytes);

	if (!read_storage_device(info->device->name, start_lba, lengthbytes, dirbuffer))
	{
		kprintf("ISO9660: Could not read LBA sectors 0x%x+0x%x when loading directory!\n", start_lba, lengthbytes / 2048);
		kfree(dirbuffer);
		return NULL;
	}

	uint32_t dir_items = 0;
	fs_directory_entry_t* list = NULL;

	// Iterate each of the entries in this directory, enumerating files
	unsigned char* walkbuffer = dirbuffer;
	int entrycount = 0;
	while (walkbuffer < dirbuffer + lengthbytes)
	{
		entrycount++;
		ISO9660_directory* fentry = (ISO9660_directory*)walkbuffer;

		if (fentry->length == 0)
			break;

		// Skip the first two entries, '.' and '..'
		if (entrycount > 2)
		{
			fs_directory_entry_t* thisentry = (fs_directory_entry_t*)kmalloc(sizeof(fs_directory_entry_t));

			if (info->joliet == 0)
			{
				thisentry->filename = (char*)kmalloc(fentry->filename_length + 1);
				j = 0;
				char* ptr = fentry->filename;
				// Stop at end of string or at ; which seperates the version id from the filename.
				// We don't want the version ids.
				for (; j < fentry->filename_length && *ptr != ';'; ++ptr)
					thisentry->filename[j++] = tolower(*ptr);
				thisentry->filename[j] = 0;

				/* Filenames ending in '.' are not allowed */
				if (thisentry->filename[j - 1] == '.')
					thisentry->filename[j - 1] = 0;
			}
			else
			{
				// Parse joliet filename, 16-bit unicode UCS-2
				thisentry->filename = (char*)kmalloc((fentry->filename_length / 2) + 1);
				j = 0;
				char* ptr = fentry->filename;
				for (; j < fentry->filename_length / 2; ptr += 2)
				{
					if (*ptr != 0)
						thisentry->filename[j++] = '?';
					else
						thisentry->filename[j++] = *(ptr + 1);
				}
				thisentry->filename[j] = 0;
				//kprintf("'%s' ", thisentry->filename);
			}

			thisentry->year = fentry->recording_date.years_since_1900 + 1900;
			thisentry->month = fentry->recording_date.month;
			thisentry->day = fentry->recording_date.day;
			thisentry->hour = fentry->recording_date.hour;
			thisentry->min = fentry->recording_date.minute;
			thisentry->sec = fentry->recording_date.second;
			strlcpy(thisentry->device_name, info->device->name, 16);
			thisentry->device = 0;
			thisentry->lbapos = fentry->extent_lba_lsb;
			thisentry->size = fentry->data_length_lsb;
			thisentry->flags = 0;
			thisentry->directory = node;

			if (fentry->file_flags & 0x02)
				thisentry->flags |= FS_DIRECTORY;
			dir_items++;

			//kprintf(" %d\n", thisentry->flags);

			thisentry->next = list;
			list = thisentry;
		}
		walkbuffer += fentry->length;

	}

	kfree(dirbuffer);

	return list;
}

void parse_svd(iso9660* info, unsigned char* buffer)
{
	PVD* svd = (PVD*)buffer;
        if (!VERIFY_ISO9660(svd))
	{
		kprintf("ISO9660: Invalid SVD found, identifier is not 'CD001'\n");
		return;
	}

	int joliet = 0;
	if (svd->escape_seq[0] == '%' && svd->escape_seq[1] == '/')
	{
		switch (svd->escape_seq[2])
		{
			case '@':
				joliet = 1;
			break;
			case 'C':
				joliet = 2;
			break;
			case 'E':
				joliet = 3;
			break;
		}
	}

	if (joliet)
	{
		//kprintf("Joliet extensions found on CD drive %s, UCS-2 Level %d\n", info->device->name, joliet);
		info->joliet = joliet;
		info->pathtable_lba = svd->lsb_pathtable_L_lba;
		info->rootextent_lba = svd->root_directory.extent_lba_lsb;
		info->rootextent_len = svd->root_directory.data_length_lsb;
		info->root = parse_directory(NULL, info, svd->root_directory.extent_lba_lsb, svd->root_directory.data_length_lsb);
	}
	return;
}

void parse_VPD([[maybe_unused]] iso9660* info, [[maybe_unused]] unsigned char* buffer)
{
}

fs_directory_entry_t* hunt_entry(iso9660* info, const char* filename, uint32_t flags)
{
	fs_directory_entry_t* currententry = info->root;
	for(; currententry->next; currententry = currententry->next)
	{
		if ((flags != 0) && !(currententry->flags & flags))
			continue;
		if (!strcmp(filename, currententry->filename))
		{
			return currententry;
		}
	}
	return NULL;
}

void* iso_get_directory(void* t)
{
	fs_tree_t* treeitem = (fs_tree_t*)t;
	if (treeitem)
	{
		iso9660* info = (iso9660*)treeitem->opaque;
		return (void*)parse_directory(treeitem, (iso9660*)treeitem->opaque, treeitem->lbapos ? treeitem->lbapos : info->rootextent_lba, treeitem->size ? treeitem->size : info->rootextent_len);
	}
	else
	{
		kprintf("*** BUG *** iso_get_directory: null fs_tree_t*!\n");
		return NULL;
	}
}

bool iso_read_file(void* f, uint64_t start, uint32_t length, unsigned char* buffer)
{
	fs_directory_entry_t* file = (fs_directory_entry_t*)f;
	storage_device_t* fs = find_storage_device(file->device_name);

	uint64_t sectors_size = length / fs->block_size;
	uint64_t sectors_start = start / fs->block_size + file->lbapos;

	// Because its not valid to read 0 sectors, we must make sure we read at least one,
	// and to make sure we read the remainder because this is an integer division,
	// we must read one more than we asked for for safety.
	sectors_size++;

	unsigned char* readbuf = (unsigned char*)kmalloc(sectors_size * fs->block_size);
	if (!read_storage_device(file->device_name, sectors_start, length, readbuf))
	{
		kprintf("ISO9660: Could not read LBA sectors 0x%x-0x%x!\n", sectors_start, sectors_start + sectors_size);
		kfree(readbuf);
		return false;
	}
	memcpy(buffer, readbuf + (start % fs->block_size), length);

	//kprintf("Freeing ptr %08x\n", readbuf);
	kfree(readbuf);
	return true;
}

iso9660* iso_mount_volume(const char* name)
{
	storage_device_t* fs = find_storage_device(name);
	if (!fs) {
		dprintf("Can't find storage device %s\n", name);
		return NULL;
	}

	unsigned char* buffer = (unsigned char*)kmalloc(fs->block_size);
	iso9660* info = (iso9660*)kmalloc(sizeof(iso9660));
	_memset(buffer, 0, 2048);
	uint32_t VolumeDescriptorPos = PVD_LBA;
	info->device = fs;
	while (1)
	{
		if (!read_storage_device(name, VolumeDescriptorPos++, fs->block_size, buffer)) {
			kprintf("ISO9660: Could not read LBA sector 0x%x from %s!\n", VolumeDescriptorPos, name);
			kfree(info);
			return NULL;
		}
		unsigned char VolumeDescriptorID = buffer[0];
		if (VolumeDescriptorID == 0xFF) {
			// Volume descriptor terminator
			break;
		} else if (VolumeDescriptorID == 0x00) {
			parse_boot(info, buffer);
		} else if (VolumeDescriptorID == 0x01) {
			// Primary volume descriptor
			if (!parse_pvd(info, buffer)) {
				kfree(info);
				kfree(buffer);
				return NULL;
			}
		} else if (VolumeDescriptorID == 0x02) {
			// Supplementary volume descriptor
			parse_svd(info, buffer);
		} else if (VolumeDescriptorID == 0x03) {
			// Volume partition descriptor
			parse_VPD(info, buffer);
		} else if (VolumeDescriptorID >= 0x04 && VolumeDescriptorID <= 0xFE) {
			// Reserved and unknown ID
			kprintf("ISO9660: WARNING: Unknown volume descriptor 0x%x at LBA 0x%x!\n", VolumeDescriptorID, VolumeDescriptorPos);
		}
	}

	kfree(buffer);
	return info;
}

int iso9660_attach(const char* device, const char* path)
{
	iso9660* vol = iso_mount_volume(device);
	if (!vol) {
		return 0;
	}
	return attach_filesystem(path, iso9660_fs, vol);
}

void init_iso9660()
{
	iso9660_fs = kmalloc(sizeof(filesystem_t));
	strlcpy(iso9660_fs->name, "iso9660", 31);
	iso9660_fs->mount = iso9660_attach;
	iso9660_fs->getdir = iso_get_directory;
	iso9660_fs->readfile = iso_read_file;
	iso9660_fs->writefile = NULL;
	iso9660_fs->truncatefile = NULL;
	iso9660_fs->createfile = NULL;
	iso9660_fs->createdir = NULL;
	iso9660_fs->rmdir = NULL;
	iso9660_fs->rm = NULL;
	register_filesystem(iso9660_fs);
}

