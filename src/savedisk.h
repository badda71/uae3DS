
#ifndef SAVEDISK_SLOT
#define SAVEDISK_SLOT 512
#endif

unsigned savedisk_get_checksum(void *mem, unsigned size);
void savedisk_apply_changes(void *mem, void *patch, unsigned patch_size);
unsigned savedisk_get_changes_file(void *mem, unsigned size, void *patch, char *filename);
unsigned savedisk_get_changes(void *mem, unsigned size, void *patch, void *orig);
