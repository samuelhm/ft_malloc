 #include "internal.h"

static heap_t g_heap = {0};

void	*get_free_zone(zone_type type, size_t size)
{
		zone_t *zone = (type == TINY) ? g_heap.tiny_zones : g_heap.small_zones;
		while (zone != NULL)
		{
			block_t *block = find_free_block(zone, size);
			if (block != NULL)
				return (split_block(block, size));
			zone = zone->next;
		}
		zone = create_zone(type, size);
		if (zone == NULL)
			return (NULL);
		zone->next = (type == TINY) ? g_heap.tiny_zones : g_heap.small_zones;
		if (type == TINY)
			g_heap.tiny_zones = zone;
		else
			g_heap.small_zones = zone;
		return (split_block(zone->first_block, size));
}

void	*malloc(size_t size)
{
	if (size <= 0)
		return (NULL);
	zone_type type = get_zone_type(size);
	block_t *block;

	if (type == LARGE)
	{
		zone_t *zone = create_zone(LARGE, size);
		if (zone == NULL)
			return (NULL);
		return (void *)((char *)zone + ZONE_META_SIZE);
	}
	else
	{
		block = get_free_zone(type, size);
		if (block == NULL)
			return (NULL);
	}
	return ((void *)((char *)block + BLOCK_META_SIZE));

}
