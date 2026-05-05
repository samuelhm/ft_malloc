#ifndef INTERNAL_H
# define INTERNAL_H
# define _GNU_SOURCE // for MAP_ANONYMOUS in vscode's C17 standard library
# include <stddef.h>
# include <unistd.h>
# include <sys/mman.h> // for mmap and munmap
# include <stdint.h> // for SIZE_MAX


#define ALIGNMENT 16
/*
** ALIGN(x) rounds x up to the next multiple of ALIGNMENT.
**
** The idea is:
** - add ALIGNMENT - 1 so values that are already aligned stay in place
**   and values in between two boundaries move far enough to reach the next one
** - clear the low bits with the mask ~(ALIGNMENT - 1)
*/
#define ALIGN(x) (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

#define TINY_MAX 128
#define SMALL_MAX 1024

#define TINY_ZONE_SIZE (4 * sysconf(_SC_PAGESIZE))
#define SMALL_ZONE_SIZE (32 * sysconf(_SC_PAGESIZE))

typedef enum e_zone_type
{
	TINY,
	SMALL,
	LARGE
}				zone_type;

typedef struct s_block
{
	size_t			size;
	int				free;
	struct s_block	*next;
	struct s_block	*prev;
}				block_t;

typedef struct s_zone
{
	zone_type		type;
	size_t			size;
	size_t			free_size;
	block_t			*first_block;
	struct s_zone	*next;
}				zone_t;

typedef struct s_heap
{
	zone_t	*tiny_zones;
	zone_t	*small_zones;
	zone_t	*large_zones;
}				heap_t;

#define BLOCK_META_SIZE ALIGN(sizeof(block_t))
#define ZONE_META_SIZE ALIGN(sizeof(zone_t))

zone_type	get_zone_type(size_t size);
void		*create_zone(zone_type type, size_t size);
block_t		*find_free_block(zone_t *zone, size_t size);
block_t		*split_block(block_t *block, size_t size);
block_t		*get_block_from_ptr(void *ptr);
void		coalesce_blocks(block_t *block);
block_t		*split_block(block_t *block, size_t size);
block_t		*find_free_block(zone_t *zone, size_t size);
block_t		*get_block_from_ptr(void *ptr);
void		coalesce_blocks(block_t *block);

#endif
