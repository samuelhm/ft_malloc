#include "ft_malloc.h"
#include <stdio.h>
#include <string.h>

int	main(void)
{
	printf("=== ft_malloc test ===\n\n");
	
	// Test basic malloc
	printf("Testing malloc(100)...\n");
	char *str = malloc(100);
	if (str == NULL)
	{
		printf("malloc failed!\n");
		return (1);
	}
	strcpy(str, "Hello, ft_malloc!");
	printf("Allocated: %s\n", str);
	
	// Test realloc
	printf("\nTesting realloc...\n");
	str = realloc(str, 200);
	if (str == NULL)
	{
		printf("realloc failed!\n");
		return (1);
	}
	strcat(str, " Expanded!");
	printf("Reallocated: %s\n", str);
	
	// Test free
	printf("\nTesting free...\n");
	free(str);
	printf("Freed successfully\n");
	
	// Show memory state
	printf("\nMemory state:\n");
	show_alloc_mem();
	
	return (0);
}