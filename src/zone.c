 #include "internal.h"


zone_type	get_zone_type(size_t size)
{
	if (size <= TINY_MAX)
		return (TINY);
	else if (size <= SMALL_MAX)
		return (SMALL);
	else
		return (LARGE);
}

void	*create_zone(zone_type type, size_t size)
{
	if (SIZE_MAX - ZONE_META_SIZE - BLOCK_META_SIZE < size)
	{
	return NULL;
	}

	size_t zone_size;
	if (type == TINY)
		zone_size = TINY_ZONE_SIZE;
	else if (type == SMALL)
		zone_size = SMALL_ZONE_SIZE;
	else
		zone_size = ALIGN(size + ZONE_META_SIZE + BLOCK_META_SIZE);

	void *zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (zone == MAP_FAILED)
		return (NULL);
	zone_t *zone_ptr = (zone_t *)zone;
	zone_ptr->type = type;
	zone_ptr->size = zone_size;
	zone_ptr->next = NULL;
	zone_ptr->free_size = zone_size - ZONE_META_SIZE - BLOCK_META_SIZE;
	zone_ptr->first_block = (block_t *)((char *)zone + ZONE_META_SIZE);
	zone_ptr->first_block->size = zone_size - ZONE_META_SIZE - BLOCK_META_SIZE;
	zone_ptr->first_block->free = 1;
	zone_ptr->first_block->next = NULL;
	zone_ptr->first_block->prev = NULL;
	return (zone);
}
