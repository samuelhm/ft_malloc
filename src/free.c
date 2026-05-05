 #include "internal.h"

void	free(void *ptr)
{
	if (!ptr)
		return;
	block_t *block = get_block_from_ptr(ptr);
	block->free = 1;
	coalesce_blocks(block);
}
