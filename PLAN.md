# Plan: ft_malloc - Implementación Completa

## 📚 Entendiendo el Ejercicio a la Perfección

### ¿Qué es este proyecto?
Es una librería que reimplementa las funciones `malloc()`, `free()` y `realloc()` del sistema. No puedes usar la libc malloc internamente - debes usar `mmap(2)` directamente.

### Tres categorías de memoria:
| Categoría | Rango de bytes | Cómo se almacena |
|-----------|----------------|------------------|
| **TINY** | 1 a n bytes | Zonas pre-asignadas de N bytes |
| **SMALL** | (n+1) a m bytes | Zonas pre-asignadas de M bytes |
| **LARGE** | (m+1)+ bytes | mmap directo por cada asignación |

### Requisitos clave:
- Cada zona debe contener **al menos 100 asignaciones**
- Los tamaños de zona deben ser múltiplos de `getpagesize()`
- Solo **1 variable global** para tracking de asignaciones
- Solo **1 variable global** para thread-safety (bonus)
- La memoria devuelta debe estar **alineada** correctamente

---

## 🏗️ Estructura del Proyecto

```
ft_malloc/
├── Makefile                    # Build principal
├── include/
│   └── ft_malloc.h            # Headers públicos
├── src/
│   ├── malloc.c               # Implementación malloc
│   ├── free.c                 # Implementación free
│   ├── realloc.c              # Implementación realloc
│   ├── show_alloc_mem.c       # show_alloc_mem()
│   ├── zone.c                 # Gestión de zonas
│   ├── block.c                # Gestión de bloques
│   └── utils/
│       ├── align.c            # Alinear memoria
│       └── debug.c            # Funciones debug (bonus)
├── bonus/
│   ├── thread_safe.c          # Thread-safety
│   ├── show_alloc_mem_ex.c    # Versión extendida
│   └── defrag.c               # Defragmentación
└── libft/
    ├── Makefile               # Build de libft
    └── ...
```

### Archivos recomendados crear:

```c
// include/ft_malloc.h
#ifndef FT_MALLOC_H
# define FT_MALLOC_H

# include <stddef.h>  // size_t
# include <stdint.h>  // uintptr_t

// Prototipos públicos
void    *malloc(size_t size);
void     free(void *ptr);
void    *realloc(void *ptr, size_t size);
void     show_alloc_mem(void);

// Bonus
void     show_alloc_mem_ex(void);

// Configuración (tú defines estos valores)
# define TINY_LIMIT    ...  // ej: 128 bytes
# define SMALL_LIMIT  ...  // ej: 1024 bytes
# define TINY_ZONE_SIZE    ...
# define SMALL_ZONE_SIZE   ...

#endif
```

```c
// src/zone.c o include/zone.h
// Estructura para representar una zona de memoria
typedef struct s_zone {
    size_t      size;           // Tamaño total de la zona
    size_t      used;           // Memoria usada
    struct s_zone *next;        // Siguiente zona
    // ... metadatos adicionales
} t_zone;
```

```c
// src/block.c o include/block.h
// Estructura para cada bloque dentro de una zona
typedef struct s_block {
    size_t      size;           // Tamaño del bloque
    int         free;           // 1 si libre, 0 si ocupado
    struct s_block *next;       // Siguiente bloque
    // ... datos del usuario comienzan aquí
} t_block;
```

---

## 📋 Checklist de Tareas

### Fase 1: Fundamentos (Obligatorio)
- [ ] 1. Crear estructura de directorios
- [ ] 2. Crear Makefile con variables HOSTTYPE
- [ ] 3. Implementar libft básica (write, etc.)
- [ ] 4. Crear headers y estructuras básicas
- [ ] 5. Implementar `mmap_zone()` - crear zona nueva
- [ ] 6. Implementar `allocate_block()` - asignar bloque
- [ ] 7. Implementar `malloc()` básico - detección TINY/SMALL/LARGE

### Fase 2: Free y Realloc (Obligatorio)
- [ ] 8. Implementar `free()` - marcar bloque como libre
- [ ] 9. Implementar `realloc()` - realojar memoria
- [ ] 10. Manejar caso realloc que no cabe en bloque actual
- [ ] 11. Implementar `show_alloc_mem()` - mostrar estado

### Fase 3: Bonus - Thread Safety
- [ ] 12. Añadir mutex global para thread-safety
- [ ] 13. Proteger todas las funciones con mutex

### Fase 4: Bonus - Extras
- [ ] 14. Implementar variables de debug (MALLOC_DEBUG, etc.)
- [ ] 15. Implementar `show_alloc_mem_ex()` con detalles extra
- [ ] 16. Implementar defragmentación de memoria libre

### Fase 5: Verificación
- [ ] 17. Testing con múltiples hilos
- [ ] 18. Testing edge cases (free NULL, realloc NULL, etc.)
- [ ] 19. Verificar alineación de memoria
- [ ] 20. Compilar con valgrind/sanitizers

---

## 💡 Consejos y Errores Comunes a Evitar

### 1. Alinear la memoria
```c
// La memoria devuelta debe estar alineada para cualquier tipo
// Usa ALIGN_PTR para asegurar alineación de 16 bytes (típico)
#define ALIGNMENT 16
#define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
```

### 2. Gestión de metadatos
```
┌─────────────────────────────────────────────┐
│                 ZONA (mmap)                  │
│  ┌─────────┬──────────────────────────────┐ │
│  │ Metadatos│        Usuario              │ │
│  │ (t_block)│   (memoria devuelta)        │ │
│  └─────────┴──────────────────────────────┘ │
└─────────────────────────────────────────────┘
```
- Guarda el tamaño del bloque en los metadatos
- Guarda si el bloque está libre o no
- IMPORTANTE: necesitas saber dónde empieza el bloque del usuario

### 3. Cómo calcular n, m, N, M

**Ejemplo práctico:**
- `getpagesize()` en Linux = 4096 bytes
- Cada zona debe tener ≥100 bloques
- TINY: bloques de hasta 128 bytes → N = 128 * 100 = 12800 (múltiplo de 4096: 12288)
- SMALL: bloques de hasta 1024 bytes → M = 1024 * 100 = 102400 (múltiplo de 4096: 102400 ✓)

```c
#define TINY_LIMIT      128
#define SMALL_LIMIT     1024
#define TINY_ZONE_SIZE  12288       // 96 bloques de ~128 bytes
#define SMALL_ZONE_SIZE 102400      // 100 bloques de ~1024 bytes
```

### 4. Errores comunes:

| Error | Por qué ocurre | Cómo evitarlo |
|-------|----------------|---------------|
| Segfault al free | Intentar free() dos veces | Marcar bloque como usado antes de free |
| Memory leak | No hacer munmap de zonas vacías | Llevar registro de zonas |
| realloc falla | No manejar caso NULL ptr | Verificar ptr != NULL |
| Arena corrupta | Escribir más de lo asignado | Validar size solicitado |

### 5. Diseño de estructuras recomendadas:

```c
// Block metadata - va ANTES de la memoria del usuario
typedef struct s_block {
    size_t              size;       // Tamaño del bloque
    int                 free;       // 1 = libre, 0 = usado
    struct s_block      *next;      // Siguiente bloque
    // [datos del usuario comienzan aquí si free = 0]
} t_block;

// Para debugging (bonus)
typedef struct s_block {
    size_t              size;
    int                 free;
    struct s_block      *next;
    size_t              usable_size;    // Tamaño real usado (debug)
    const char          *file;           // Dónde se allocó (debug)
    int                 line;            // Línea (debug)
} t_block;
```

### 6. Algoritmo de malloc():
```
malloc(size):
    1. Si size == 0, puede retornar NULL o mínimo 1 byte
    2. Si size > SMALL_LIMIT → LARGE:
         - mmap(size + metadata)
         - crear bloque
         - retornar ptr al usuario
    3. Si TINY o SMALL:
         - buscar zona existente con bloque libre
         - si no hay, crear nueva zona (mmap)
         - asignar primer bloque libre
         - retornar ptr al usuario
```

### 7. Algoritmo de free():
```
free(ptr):
    1. Si ptr == NULL → return
    2. Obtener metadata (ptr - sizeof(t_block))
    3. Marcar free = 1
    4. NO hacer munmap siempre:
       - Solo si toda la zona está libre (opcional, bonus)
       - O nunca para zonas (para simplificar)
```

### 8. Algoritmo de realloc():
```
realloc(ptr, new_size):
    1. Si ptr == NULL → malloc(new_size)
    2. Si new_size == 0 → free(ptr), return NULL
    3. Obtener bloque actual
    4. Si bloque actual tiene espacio suficiente:
         - solo marcar nuevo size
         - retornar ptr
    5. Si no cabe:
         - alloc nueva memoria (malloc)
         - copiar datos old → new
         - free(old)
         - return new
```

### 9. Thread Safety (bonus):
```c
#include <pthread.h>

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
    pthread_mutex_lock(&g_mutex);
    // ... lógica de malloc
    pthread_mutex_unlock(&g_mutex);
    return ptr;
}
```

### 10. Debug environment variables (bonus):
```c
// Implementar soportando variables como:
// MALLOC_DEBUG=1       - basic info
// MALLOC_DEBUG=2       - verbose (shows allocs/free)
// MALLOC_DEBUG=0x3     - hexadecimal dump
char *env = getenv("MALLOC_DEBUG");
if (env && atoi(env) >= 1) {
    // log allocation
}
```

---

## ✅ Resumen de Archivos a Crear

| Archivo | Descripción |
|---------|-------------|
| `Makefile` | Build principal con HOSTTYPE |
| `include/ft_malloc.h` | Headers públicos |
| `include/internal.h` | Headers internos |
| `src/malloc.c` | Implementación malloc |
| `src/free.c` | Implementación free |
| `src/realloc.c` | Implementación realloc |
| `src/zone.c` | Creación/gestión de zonas |
| `src/block.c` | Gestión de bloques |
| `src/show_alloc_mem.c` | show_alloc_mem() |
| `src/utils.c` | Utilidades varias |
| `bonus/thread.c` | Thread-safety |
| `bonus/debug.c` | Debug functions |
| `libft/` | Tu libft del proyecto anterior |

---

## 🚀 Cómo Proceder

1. **Crea la estructura de directorios**
2. **Implementa paso a paso siguiendo la checklist**
3. **Testea cada función antes de continuar**
4. **Añade bonus al final**

> ⚠️ **Importante**: El bonus solo se evalúa si el mandatory está PERFECTO. No intentes hacer bonus hasta tener el mandatory funcionando al 100%.