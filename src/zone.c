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
	if (SIZE_MAX - ZONE_SIZE - BLOCK_SIZE < size)
	{
	return NULL;
	}

	size_t zone_size;
	if (type == TINY)
		zone_size = TINY_ZONE_SIZE;
	else if (type == SMALL)
		zone_size = SMALL_ZONE_SIZE;
	else
		zone_size = ALIGN(size + ZONE_SIZE + BLOCK_SIZE);

	void *zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return (zone == MAP_FAILED ? NULL : zone);
}
