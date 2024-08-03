#ifndef QJS_JS_ATOM_H_
#define QJS_JS_ATOM_H_
#include "./macros.h"
__EXTERN_START__

typedef uint32_t JSAtom;
typedef struct JSContext JSContext;
typedef struct JSRuntime JSRuntime;

/* atom support */
#define JS_ATOM_NULL 0

JSAtom JS_NewAtomLen(JSContext *ctx, const char *str, size_t len);
JSAtom JS_NewAtom(JSContext *ctx, const char *str);
JSAtom JS_NewAtomUInt32(JSContext *ctx, uint32_t n);
JSAtom JS_DupAtom(JSContext *ctx, JSAtom v);
void JS_FreeAtom(JSContext *ctx, JSAtom v);
void JS_FreeAtomRT(JSRuntime *rt, JSAtom v);
JSValue JS_AtomToValue(JSContext *ctx, JSAtom atom);
JSValue JS_AtomToString(JSContext *ctx, JSAtom atom);
const char *JS_AtomToCString(JSContext *ctx, JSAtom atom);
JSAtom JS_ValueToAtom(JSContext *ctx, JSValueConst val);



__EXTERN_END__
#endif // QJS_JS_ATOM_H_