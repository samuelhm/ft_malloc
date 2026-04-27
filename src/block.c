#include "internal.h"

block_t	*find_free_block(zone_t *zone, size_t size)
{
	block_t *current = zone->first_block;

	while (current != NULL)
	{
		if (current->free && current->size >= size)
			return (current);
		current = current->next;
	}
	return (NULL);
}

block_t	*split_block(block_t *block, size_t size)
{
	(void)block;
	(void)size;
	return (NULL);
}

block_t	*get_block_from_ptr(void *ptr)
{
	(void)ptr;
	return (NULL);
}

void	coalesce_blocks(block_t *block)
{
	(void)block;
}
