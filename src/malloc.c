#include "internal.h"

heap_t g_heap = {0};

static void	*allocate_in_zone_list(zone_type type, size_t asize);

void	*malloc(size_t size)
{
	if (size == 0)
		return (NULL);
	size_t asize = ALIGN(size);
	zone_type type = get_zone_type(asize);
	if (type == LARGE)
	{
		zone_t *new_zone = create_zone(type, asize);
		if (new_zone == NULL)
			return (NULL);
		new_zone->next = g_heap.large_zones;
		g_heap.large_zones = new_zone;
		new_zone->first_block->free = 0;
		return (void*)((char*)new_zone->first_block + BLOCK_META_SIZE);
	}
	return (allocate_in_zone_list(type, asize));
}

static void	*allocate_in_zone_list(zone_type type, size_t asize)
{
	zone_t	**zones;
	zone_t	*current_zone;

	zones = (type == TINY) ? &g_heap.tiny_zones : &g_heap.small_zones;
	current_zone = *zones;
	while (current_zone != NULL)
	{
		block_t *block = find_free_block(current_zone, asize);
		if (block != NULL)
			return (void*)((char*)split_block(block, asize) + BLOCK_META_SIZE);
		current_zone = current_zone->next;
	}
	zone_t *new_zone = create_zone(type, asize);
	if (new_zone == NULL)
		return (NULL);
	new_zone->next = *zones;
	*zones = new_zone;
	return (void*)((char*)split_block(new_zone->first_block, asize) + BLOCK_META_SIZE);
}