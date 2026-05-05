#include "internal.h"

static bool find_block_in_zone_list(zone_t *zone_list, void *ptr);

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
	if (block->size <= size + BLOCK_META_SIZE)
	{
		block->free = 0;
		return (block);
	}
	block_t *new_block = (block_t *)((char *)block + BLOCK_META_SIZE + size);
	new_block->size = block->size - size - BLOCK_META_SIZE;
	new_block->free = 1;
	new_block->next = block->next;
	new_block->prev = block;
	if (new_block->next) 
		new_block->next->prev = new_block;
	block->size = size;
	block->free = 0;
	block->next = new_block;
	return (block);
}

block_t	*get_block_from_ptr(void *ptr)
{
	return ((block_t *)((char *)ptr - BLOCK_META_SIZE));
}

void	coalesce_blocks(block_t *block)
{
	if (block->next && block->next->free)
	{
		block->size += BLOCK_META_SIZE + block->next->size;
		block->next = block->next->next;
		if (block->next)
			block->next->prev = block;
	}
	if (block->prev && block->prev->free)
	{
		block->prev->size += BLOCK_META_SIZE + block->size;
		block->prev->next = block->next;
		if (block->next)
			block->next->prev = block->prev;
	}
}

bool	is_valid_ptr(void *ptr)
{
	return (find_block_in_zone_list(g_heap.tiny_zones, ptr)
		|| find_block_in_zone_list(g_heap.small_zones, ptr)
		|| find_block_in_zone_list(g_heap.large_zones, ptr));
}

static bool find_block_in_zone_list(zone_t *zone_list, void *ptr)
{
	while (zone_list)
	{
		block_t *block = zone_list->first_block;
		while (block)
		{
			if ((void *)((char *)block + BLOCK_META_SIZE) == ptr)
				return (!block->free);
			block = block->next;
		}
		zone_list = zone_list->next;
	}
	return (false);
}