#include "internal.h"

block	*find_free_block(zone *current_zone, size_t size)
{
	(void)current_zone;
	(void)size;
	return (NULL);
}

block	*split_block(block *current_block, size_t size)
{
	(void)current_block;
	(void)size;
	return (NULL);
}

block	*get_block_from_ptr(void *ptr)
{
	(void)ptr;
	return (NULL);
}

void	coalesce_blocks(block *current_block)
{
	(void)current_block;
}
