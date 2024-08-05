#ifndef QJS_GC_H
#define QJS_GC_H

#include "./basic_types.h"

#include "./macros.h"
__EXTERN_START__

typedef struct JSRuntime JSRuntime;

typedef struct JSGCObjectHeader JSGCObjectHeader;
typedef struct JSRefCountHeader {
  int ref_count;
} JSRefCountHeader;

typedef struct JSMallocState {
  size_t malloc_count;
  size_t malloc_size;
  size_t malloc_limit;
  void *opaque; /* user opaque */
} JSMallocState;

typedef struct JSMallocFunctions {
  void *(*js_malloc)(JSMallocState *s, size_t size);
  void (*js_free)(JSMallocState *s, void *ptr);
  void *(*js_realloc)(JSMallocState *s, void *ptr, size_t size);
  size_t (*js_malloc_usable_size)(const void *ptr);
} JSMallocFunctions;

typedef void JS_MarkFunc(JSRuntime *rt, JSGCObjectHeader *gp);

__EXTERN_END__
#endif // QJS_GC_H_