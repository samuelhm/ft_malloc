# AGENTS.md - ft_malloc

## Project Overview
C library implementing `malloc(3)`, `free(3)`, and `realloc(3)` using `mmap(2)`/`munmap(2)`. Required for 42 school.

## Build Commands
```bash
# Build the library (produces libft_malloc_$HOSTTYPE.so and symlink libft_malloc.so)
make

# Clean build artifacts
make clean

# Full clean including objects
make fclean

# Rebuild
make re
```

## Architecture
- **TINY**: allocations 1 to n bytes, stored in N-byte zones (pre-allocate)
- **SMALL**: allocations (n+1) to m bytes, stored in M-byte zones (pre-allocate)
- **LARGE**: allocations (m+1)+ bytes, via direct mmap per allocation

Each zone must contain at least 100 allocations. Zone sizes must be multiples of `getpagesize()` (Linux) or `getpagesize()` (macOS).

## Key Constraints
- Only one global allowed for allocation tracking + one for thread-safety
- Must NOT use libc malloc internally - use mmap only
- No undefined behavior or segfaults allowed
- Memory returned must be properly aligned
- Library name: `libft_malloc_$HOSTTYPE.so` with symlink `libft_malloc.so`

## Bonus Features
- Thread-safety using pthread
- Debug environment variables (imitate system or invent own)
- `show_alloc_mem_ex()` with extended details
- Defragment freed memory