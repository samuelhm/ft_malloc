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
	(void)type;
	(void)size;
	return (NULL);
}
