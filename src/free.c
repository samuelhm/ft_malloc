#include "internal.h"

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