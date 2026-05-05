#include "internal.h"
#include <stdbool.h>

static bool is_valid_ptr(void *ptr);
static bool check_in_zone(zone_t *zone, void *ptr);

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
	return (check_in_zone(g_heap.tiny_zones, ptr) || 
			check_in_zone(g_heap.small_zones, ptr) || 
			check_in_zone(g_heap.large_zones, ptr)
			);
}

static bool check_in_zone(zone_t *zone, void *ptr)
{
	while (zone)
	{
		if (ptr >= (void *)((char *)zone + ZONE_META_SIZE + BLOCK_META_SIZE)
			&& ptr < (void *)((char *)zone + zone->size))
			return (true);
		zone = zone->next;
	}
	return (false);
}