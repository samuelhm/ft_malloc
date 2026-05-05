# ft_malloc - Plan de Implementación

## Conceptos Fundamentales

### ¿Qué es ft_malloc?
Reimplementar las funciones de asignación dinámica de memoria del sistema (`malloc`, `free`, `realloc`) usando únicamente `mmap` y `munmap`. No puedes usar `malloc` de la libc internamente.

### Las tres categorías de memoria

| Tipo | Tamaño | Estrategia |
|------|--------|------------|
| TINY | pequeño (ej: ≤128 bytes) | Pre-asignar zonas grandes con mmap |
| SMALL | mediano (ej: ≤1024 bytes) | Pre-asignar zonas grandes con mmap |
| LARGE | grande (>1024 bytes) | mmap directo para cada petición |

### ¿Por qué pre-asignar?
Para minimizar llamadas al sistema. Cada `mmap` es costoso. Si haces 1000 mallocs pequeños sin pre-asignación, harías 1000 mmaps. Con pre-asignación, haces ~1 mmap y gestionas los 1000 bloques dentro de esa zona.

---

## Estructura de Datos

### Diseño conceptual

```
Memoria obtenida con mmap:

┌──────────────────────────────────────────────────────────┐
│                    ZONA (ej: 12KB)                       │
│  ┌─────────────────────────────────────────────────────┐ │
│  │ BLOQUE 1  │ BLOQUE 2  │ BLOQUE 3  │ ... │ BLOQUE N │ │
│  │ [meta]    │ [meta]    │ [meta]    │     │ [meta]   │ │
│  │ datos...  │ datos...  │ datos...  │     │ datos... │ │
│  └─────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────┘

Cada bloque:
┌────────────┬─────────────────────┐
│  METADATA  │    DATOS USUARIO    │
│  (header)  │    (lo que retorna) │
└────────────┴─────────────────────┘
```

### Estructuras C recomendadas

```c
//Cabecera de cada bloque - SIN EXPOSICIÓN AL USUARIO
typedef struct s_block {
    size_t               size;    // Tamaño de datos (sin contar header)
    int                  free;   // 0 ocupado, 1 libre
    struct s_block      *next;   // Siguiente bloque en la zona
    struct s_block      *prev;   // Bloque anterior (opcional, útil para coalescing)
} t_block;

// Una zona de memoria
typedef struct s_zone {
    e_zone_type          type;   // TINY, SMALL, LARGE
    size_t               size;   // Tamaño total de la zona
    size_t               free_size; // Bytes libres
    t_block             *first_block; // Primer bloque
    struct s_zone       *next;   // Siguiente zona en lista
} t_zone;

// Variable global - lista de zonas
t_zone *g_zones = NULL;
```

---

## Tamaños Recomendados

```c
// Límites de categorías
#define TINY_MAX        128     //变小 бл. до 128 bytes
#define SMALL_MAX       1024    // SMALL де 129 a 1024 bytes
// LARGE = > 1024 bytes

// Tamaños de zona (múltiplos de página)
// página = 4096 en Linux, 4096 en macOS
#define TINY_ZONE_SIZE   (4 * 4096)   // 16KB para muchos tiny
#define SMALL_ZONE_SIZE  (32 * 4096)  // 128KB para small

// Alinhamiento
#define ALIGNMENT        16      // Alineación de memoria
#define ALIGN(x)         (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define BLOCK_SIZE       (ALIGN(sizeof(t_block)))
```

---

## Estructura del Proyecto

```
ft_malloc/
├── Makefile
├── include/
│   └── ft_malloc.h
├── src/
│   ├── main/
│   │   ├── malloc.c
│   │   ├── free.c
│   │   └── realloc.c
│   ├── zones/
│   │   ├── zone_manager.c
│   │   └── zone_init.c
│   ├── blocks/
│   │   ├── block_manager.c
│   │   └── block_split.c
│   ├── utils/
│   │   ├── align.c
│   │   └── debug.c
│   └── display/
│       └── show_alloc_mem.c
├── bonus/
│   └── thread_safe.c
└── libft/
    └── ... (tu libft existente)
```

---

## Checklist de Implementación

### Milestone 1: Setup
- [x] Crear estructura de directorios
- [x] Crear Makefile con reglas estándar y HOSTTYPE
- [x] Crear header `ft_malloc.h` con structs y prototipos
- [x] Configurar libft como submódulo/subdirectorio

### Milestone 2: Malloc Básico
- [x] Implementar `get_zone_type(size)` - determinar TINY/SMALL/LARGE
- [x] Implementar `create_zone(type)` - hacer mmap de nueva zona
- [x] Implementar `find_free_block(zone, size)` - buscar bloque libre
- [x] Implementar `split_block(block, size)` - dividir bloque grande
- [x] Implementar `malloc(size)` - combinar todo lo anterior

### Milestone 3: Free
- [ ] Implementar `get_block_from_ptr(ptr)` - obtener bloque desde puntero
- [ ] Implementar `mark_block_free(block)`
- [ ] Implementar `coalesce_blocks(zone)` - fusionar bloques libres adyacentes
- [ ] Implementar `free(ptr)` - manejar NULL y liberar

### Milestone 4: Realloc
- [ ] Implementar caso `realloc(NULL, size)` → malloc
- [ ] Implementar caso `realloc(ptr, 0)` → free
- [ ] Implementar caso cuando el bloque actual tiene espacio
- [ ] Implementar caso cuando necesita nueva asignación

### Milestone 5: Display
- [ ] Implementar `show_alloc_mem()` con formato requerido

### Milestone 6: Bonus
- [ ] Thread-safety con `pthread_mutex_t`
- [ ] Variables de debug (`MALLOC_DEBUG`)
- [ ] `show_alloc_mem_ex()`
- [ ] Defragmentación avanzada

### Milestone 7: Testing
- [ ] Probar con `strings`, arrays, estructuras
- [ ] Probar edge cases (NULL, size 0, overflow)
- [ ] Probar con múltiples hilos (bonus)
- [ ] Verificar con valgrind/sanitizers

---

## Algoritmos Detallados

### malloc(size)

```
FUNCIÓN malloc(size):
    SI size == 0:
        RETORNAR NULL (o malloc(1) según implementación)

    size = ALIGN(size)
    type = get_zone_type(size)

    SI type == LARGE:
        block = mmap(size + BLOCK_SIZE)
        SI mmap falló:
            RETORNAR NULL
        inicializar block con size
        agregar a lista de zonas LARGE
        RETORNAR (block + BLOCK_SIZE)  // puntero para usuario

    // TINY o SMALL
    zone = encontrar zona con espacio suficiente
    SI zone == NULL:
        zone = crear nueva zona(type)
        SI zona no se pudo crear:
            RETORNAR NULL

    block = encontrar bloque libre en zone
    SI block->size > size + BLOCK_SIZE + ALIGNMENT:
        // El bloque es muy grande, dividirlo
        new_block = (block + BLOCK_SIZE + size)
        new_block->size = block->size - size - BLOCK_SIZE
        new_block->free = 1
        block->size = size

    block->free = 0
    RETORNAR (block + BLOCK_SIZE)
```

### free(ptr)

```
FUNCIÓN free(ptr):
    SI ptr == NULL:
        RETORNAR  // no hacer nada

    block = (ptr - BLOCK_SIZE)
    SI block->free == 1:
        RETORNAR  // ya está libre, evitar double-free

    block->free = 1

    // Coalescing (fusionar con bloques libres adyacentes)
    SI block->next existe Y block->next->free == 1:
        block->size = block->size + BLOCK_SIZE + block->next->size
        block->next = block->next->next

    // Opcional: si toda la zona está libre, hacer munmap
    // Solo para LARGE zones
```

### realloc(ptr, size)

```
FUNCIÓN realloc(ptr, size):
    SI ptr == NULL:
        RETORNAR malloc(size)

    SI size == 0:
        free(ptr)
        RETORNAR NULL

    block = (ptr - BLOCK_SIZE)

    SI block->size >= size:
        // El bloque actual tiene espacio suficiente
        RETORNAR ptr

    // Necesitamos más espacio
    new_ptr = malloc(size)
    SI new_ptr == NULL:
        RETORNAR NULL

    // Copiar datos del bloque antiguo al nuevo
    memcpy(new_ptr, ptr, block->size)

    free(ptr)
    RETORNAR new_ptr
```

---

## El Makefile

```makefile
CC = cc
CFLAGS = -Wall -Wextra -Werror -fPIC
NAME = libft_malloc_$(HOSTTYPE).so

# Si HOSTTYPE no está definido
ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

SRC = src/main/malloc.c src/main/free.c src/main/realloc.c \
      src/zones/zone_manager.c src/zones/zone_init.c \
      src/blocks/block_manager.c src/blocks/block_split.c \
      src/utils/align.c src/display/show_alloc_mem.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^
	ln -sf $(NAME) libft_malloc.so

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -I include

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME) libft_malloc.so

re: fclean all

.PHONY: all clean fclean re
```

---

## Consejos Prácticos

### 1. Debuggear
Para ver qué está pasando:
```c
#ifdef DEBUG
# define DPRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
# define DPRINT(fmt, ...)
#endif
```

### 2. Alineación siempre
Los punteros deben estar alineados a 16 bytes (o 8 en sistemas de 32 bits):
```c
void *aligned_ptr = (void *)ALIGN((size_t)ptr);
```

### 3. Cómo encontrar una zona desde un puntero
```c
t_zone *find_zone(void *ptr) {
    t_zone *zone = g_zones;
    while (zone) {
        void *start = (void *)zone + sizeof(t_zone);
        void *end = start + zone->size;
        if (ptr >= start && ptr < end)
            return zone;
        zone = zone->next;
    }
    return NULL;
}
```

### 4. El problema de la variable global
Solo puedes tener UNA variable global para todo el tracking. Solución: que todo cuelgue de una struct:
```c
typedef struct s_malloc_info {
    t_zone     *tiny_zones;
    t_zone     *small_zones;
    t_zone     *large_zones;
    // ...
} t_malloc_info;

t_malloc_info g_malloc;
```

### 5. Cosas que no se pueden usar
- `malloc`, `calloc`, `realloc`, `free` de libc
- `brk`, `sbrk` (solo mmap/munmap)
- Memoria estática grande (solo para metadatos pequeños)

---

## Test Básicos

```c
// test.c
#include "ft_malloc.h"
#include <stdio.h>

int main() {
    char *str = malloc(100);
    sprintf(str, "Hello, ft_malloc!");
    printf("%s\n", str);

    str = realloc(str, 200);
    sprintf(str, "Hello, ft_malloc! Expanded!");
    printf("%s\n", str);

    free(str);

    show_alloc_mem();
    return 0;
}
```

Compilar y ejecutar:
```bash
make
gcc -L. -o test test.c -lft_malloc -Wl,-rpath,.
./test
```

---

## Flujo de Trabajo Recomendado

1. **Setup inicial** → estructura + Makefile + headers
2. **Implementar malloc(TINY)** → el caso más simple primero
3. **Implementar free(TINY)** → testing básico
4. **Implementar SMALL y LARGE** → extender lo ya funcionando
5. **Implementar realloc** → combinar malloc + free
6. **Implementar show_alloc_mem** → visualización
7. **Añadir bonus** → thread-safety y features extra
8. **Testing exhaustivo** → edge cases y stress tests

---

## Referencias

- `man mmap` → leer sobre mmap y sus flags
- `man getpagesize` → tamaño de página
- `man pthread_mutex_init` → para thread-safety
- Revisar implementaciones de malloc reales para inspiración (ptmalloc, tcmalloc)

---

¡Empieza por el Milestone 1 y avanza paso a paso!
