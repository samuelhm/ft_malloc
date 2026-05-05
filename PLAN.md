# ft_malloc — Plan del Proyecto

## Requisitos del subject (mandatory)

| # | Requisito | Cumple? |
|---|---|---|
| 1 | `malloc()`, `free()`, `realloc()` con prototipos de libc | ⚠️ `realloc` es stub |
| 2 | Usar solo `mmap(2)` y `munmap(2)` | ✅ |
| 3 | NO usar `malloc` de libc internamente | ✅ (verificar libft) |
| 4 | Minimizar llamadas a `mmap`/`munmap`. Pre-asignar zonas para TINY/SMALL | ✅ |
| 5 | Tamaños de zona múltiplos de `sysconf(_SC_PAGESIZE)` | ✅ |
| 6 | ≥100 allocations por zona | ✅ |
| 7 | TINY: 1–n bytes, SMALL: (n+1)–m, LARGE: >m | ✅ (n=128, m=1024) |
| 8 | `show_alloc_mem()` con formato exacto | ❌ Stub |
| 9 | Memoria alineada correctamente | ✅ (ALIGNMENT=16) |
| 10 | Solo 1 variable global para tracking | ✅ (`g_heap`) |
| 11 | Sin undefined behavior ni segfaults | ⚠️ Bug en coalesce + falta validación en `free` |
| 12 | Biblioteca: `libft_malloc_$(HOSTTYPE).so` + symlink | ✅ |
| 13 | Carpeta `libft/` con su Makefile | ✅ |
| 14 | Funciones autorizadas: mmap, munmap, sysconf, getrlimit, write, pthread | ✅ |

---

## Estado actual por fichero

| Fichero | Estado |
|---|---|
| `src/malloc.c` | Funcional |
| `src/free.c` | Parcial — bug crítico, sin double-free check, sin validación de ptr, sin munmap de LARGE |
| `src/realloc.c` | Stub |
| `src/zone.c` | Funcional |
| `src/block.c` | Parcial — `coalesce_blocks` no ejecuta |
| `src/show_alloc_mem.c` | Stub vacío |
| `bonus/thread_safe.c` | Stub |
| `bonus/show_alloc_mem_ex.c` | Stub |
| `bonus/defrag.c` | Stub |
| `src/utils/debug.c` | Stub |

---

## Bugs — Prioridad ALTA

### Bug 1 (CRÍTICO): `coalesce_blocks` nunca se ejecuta

`src/block.c:43`: `if (!block || block->free) return;`

`free()` pone `block->free = 1` antes de llamar a `coalesce_blocks`, así que esta función **siempre retorna sin hacer nada**. Los bloques liberados nunca se fusionan → fragmentación permanente → los huecos no se reutilizan.

**Fix:** Mover la comprobación `block->free` a `free()` (como protección double-free), no a `coalesce_blocks`.

---

## Mejoras necesarias — Prioridad MEDIA

### 2. Protección contra double-free
`free()` no verifica si el bloque ya está libre. Si se llama dos veces con el mismo puntero, corrompe la lista doblemente enlazada al intentar coalescer dos veces.

### 3. `free()` debe hacer `munmap` de zonas LARGE
Cada `malloc(LARGE)` crea una zona con un solo bloque. Al liberar, la zona entera debe devolverse al sistema con `munmap`. Ahora queda mapeada para siempre (memory leak real en LARGE).

### 4. Validar que el puntero de `free()` pertenece a nuestro heap
`get_block_from_ptr(ptr)` no valida que `ptr` esté dentro de alguna zona. Un puntero cualquiera causará corrupción. Recorrer las zonas y verificar `zone < ptr < zone + zone->size`.

### 5. Mantener `zone_t::free_size` actualizado
El campo se inicializa pero nunca se modifica al hacer `split_block` o `coalesce_blocks`.

### 6. Verificar que `libft` no usa `malloc`
`ft_printf` (usado por `show_alloc_mem`) DEBE usar solo `write(2)`. Si usa `malloc` de libc internamente, viola el requisito. Revisar `libft/src/ft_printf/ft_printf.c`.

---

## Checklist de implementación

### Fase 1: Arreglar bugs del mandatory (URGENTE)

- [ ] **Fix `coalesce_blocks`** — quitar `block->free` de la guarda
- [ ] **Protección double-free en `free()`** — `if (block->free) return;` antes de marcar
- [ ] **Validar `ptr` de `free()`** — buscar el puntero en todas las zonas antes de manipular
- [ ] **`munmap` de zonas LARGE al liberar** — si el bloque liberado está en zona LARGE → `munmap(zone, zone->size)` y quitar de la lista
- [ ] **Actualizar `zone->free_size`** en `split_block` (restar) y `coalesce_blocks` (sumar)

### Fase 2: Implementar `realloc()`

- [ ] `realloc(NULL, size)` → `return malloc(size);`
- [ ] `realloc(ptr, 0)` → `free(ptr); return NULL;`
- [ ] Si el bloque actual tiene espacio suficiente (`block->size >= ALIGN(size)`) → mismo `ptr`
- [ ] Si no basta: `malloc(size)` + `memcpy(dst, src, min(old_size, size))` + `free(ptr)`
- [ ] No usar `memcpy` de libc — implementar con `ft_memcpy` de libft

### Fase 3: Implementar `show_alloc_mem()`

Formato exacto exigido por el subject (orden creciente de direcciones):

```
TINY : 0xAAAA0000
0xAAAA0020 - 0xAAAA004A : 42 bytes
0xAAAA006A - 0xAAAA00BE : 84 bytes
SMALL : 0xAD000000
0xAD000020 - 0xAD0EAD00 : 3725 bytes
LARGE : 0xB0000000
0xB0000020 - 0xB00BBEEF : 48847 bytes
Total : 52698 bytes
```

- [ ] Iterar zonas en orden: TINY → SMALL → LARGE
- [ ] Imprimir cabecera de cada zona: `TIPO : 0xDIR_ZONA`
- [ ] Imprimir cada bloque **ocupado** (no libres): `0xDATA - 0xDATA+size : size bytes`
- [ ] Total de bytes ocupados
- [ ] Usar solo `write(2)` (via `ft_putstr_fd`, `ft_putnbr_fd`, `ft_putchar_fd` de libft)
- [ ] Hex de direcciones sin prefijo, en mayúsculas

### Fase 4: Bonus

#### 4a. Thread-safety
- [ ] Añadir `static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;` (segunda variable global, permitida para threads)
- [ ] `mutex_lock()` al inicio de `malloc`, `free`, `realloc`, todas las funciones de display
- [ ] `mutex_unlock()` al final de cada función
- [ ] Cuidado: no hacer lock durante `mmap`/`munmap` largos si no es necesario

#### 4b. Variables de entorno de debug
- [ ] `MALLOC_DEBUG=1` — activa mensajes de debug en cada operación
- [ ] `MALLOC_SCRIBBLE=1` — rellena memoria liberada con `0xAA` para detectar use-after-free
- [ ] `MALLOC_VERBOSE=1` — imprime estadísticas al final del programa via `__attribute__((destructor))`
- [ ] Leer `getenv()` una sola vez en inicialización (lazy init)

#### 4c. `show_alloc_mem_ex()`
- [ ] Historial de operaciones (lista circular de últimas N operaciones: tipo, ptr, size, timestamp)
- [ ] Hex dump de zonas: 16 bytes por línea, formato `0xADDR: xx xx xx xx ... |ASCII|`
- [ ] Estadísticas: total allocs, total frees, fragmentación %, memoria pico usada
- [ ] Mostrar también bloques libres

#### 4d. Defragmentación
- [ ] Al llamar a `defragment()` (o automáticamente cuando fragmentación > umbral):
  - Fusionar bloques libres adyacentes (ya hecho con coalesce)
  - Mover bloques ocupados para consolidar espacio libre al final de cada zona
  - Si una zona queda completamente libre, `munmap` y quitarla de la lista
- [ ] Cuidado con invalidar punteros que el usuario tiene (no podemos mover sin saber)

### Fase 5: Testing exhaustivo

- [ ] Test `malloc(0)` → `NULL`
- [ ] Test `free(NULL)` → no-op
- [ ] Test alloc + free masivo (10000 iteraciones)
- [ ] Test double-free
- [ ] Test `realloc`: shrink, grow, move, `NULL`, size `0`
- [ ] Test alineación: verificar que punteros retornados son múltiplos de 16
- [ ] Test con `LD_PRELOAD=./libft_malloc.so ls` (drop-in replacement)
- [ ] Valgrind / ASan sin leaks ni errores
- [ ] Test multi-thread (si se implementa bonus)
- [ ] Test `show_alloc_mem` formato

---

## Arquitectura

```
g_heap (global única)
├── tiny_zones  → zone_t → zone_t → ...
│   └── first_block → block_t ⇄ block_t ⇄ ...
├── small_zones → zone_t → zone_t → ...
│   └── first_block → block_t ⇄ block_t ⇄ ...
└── large_zones → zone_t → zone_t → ...
    └── first_block → block_t  (1 bloque por zona LARGE)
```

```c
#define TINY_MAX        128
#define SMALL_MAX       1024
#define TINY_ZONE_SIZE  (4 * sysconf(_SC_PAGESIZE))    // 16 KB → ≥100 allocs
#define SMALL_ZONE_SIZE (32 * sysconf(_SC_PAGESIZE))   // 128 KB → ≥100 allocs
#define ALIGNMENT       16
#define BLOCK_META_SIZE ALIGN(sizeof(block_t))          // 32 bytes
#define ZONE_META_SIZE  ALIGN(sizeof(zone_t))           // 48 bytes
```

### Verificación de ≥100 allocs por zona:
- **TINY**: (16384 − 48 − 32) / (128 + 32) = 16304 / 160 = **101** ≥ 100 ✅
- **SMALL**: (131072 − 48 − 32) / (1024 + 32) = 130992 / 1056 = **124** ≥ 100 ✅

---

## Funciones autorizadas (y cómo usarlas)

| Función | Uso en el proyecto |
|---|---|
| `mmap(2)` | Crear zonas TINY/SMALL/LARGE |
| `munmap(2)` | Liberar zonas completas (LARGE, o zonas vacías en bonus defrag) |
| `sysconf(_SC_PAGESIZE)` | Calcular tamaños de zona y alinear LARGE |
| `getrlimit(2)` | (Opcional) Limitar memoria total mmapeada para evitar agotar el sistema |
| `write(2)` (via libft) | Output en `show_alloc_mem` y `show_alloc_mem_ex` |
| `pthread_mutex_*` | Bonus thread-safety |

### Advertencia sobre libft:
`libft` no debe usar `malloc` internamente. `ft_printf` y `ft_put*_fd` deben usar solo `write(2)`. Si `ft_printf` aloja buffers con `malloc` → ilegal en este proyecto. Verificar antes de usarlo en `show_alloc_mem`.

---

## Notas técnicas

- **1 variable global**: `g_heap` (tipo `heap_t`). Para threads: +1 `g_mutex` (permitido por subject).
- **Alineación**: 16 bytes en todas las plataformas (suficiente para `long double` en x86_64). Verificar con `__BIGGEST_ALIGNMENT__` o `alignof(max_align_t)`.
- **Código limpio**: aunque no hay Norminette, el código debe ser legible, con nombres claros y sin funciones kilométricas.
- **Build**: `make` → `libft_malloc_x86_64_Linux.so` + `libft_malloc.so` (symlink).
- **Símbolos exportados**: `malloc`, `free`, `realloc`, `show_alloc_mem`, `show_alloc_mem_ex`.
- **No usar**: `brk(2)`, `sbrk(2)`, `malloc`/`calloc`/`free` de libc en NINGUNA parte del código ni de las dependencias.
    