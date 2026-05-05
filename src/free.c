#include "internal.h"
#include <stdbool.h>

static bool is_valid_ptr(void *ptr);
static bool find_block_in_zone_list(zone_t *zone_list, void *ptr);

void	free(void *ptr)
{
	if (!ptr || !is_valid_ptr(ptr))
		return;
	block_t *block = get_block_from_ptr(ptr);

	if (block->free)
		return;
	zone_t	*candidate;
	zone_t	**prev;

	candidate = (zone_t *)((char *)block - ZONE_META_SIZE);
	prev = &g_heap.large_zones;
	while (*prev)
	{
		if (*prev == candidate)
		{
			*prev = candidate->next;
			munmap(candidate, candidate->size);
			return ;
		}
		prev = &(*prev)->next;
	}
	block->free = 1;
	coalesce_blocks(block);
}

static bool	is_valid_ptr(void *ptr)
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