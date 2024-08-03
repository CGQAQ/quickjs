#ifndef QJS_JS_CONTEXT_H_
#define QJS_JS_CONTEXT_H_

#include "./macros.h"
__EXTERN_START__

typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;

JSContext *JS_NewContext(JSRuntime *rt);
void JS_FreeContext(JSContext *s);
JSContext *JS_DupContext(JSContext *ctx);
void *JS_GetContextOpaque(JSContext *ctx);
void JS_SetContextOpaque(JSContext *ctx, void *opaque);
JSRuntime *JS_GetRuntime(JSContext *ctx);
void JS_SetClassProto(JSContext *ctx, JSClassID class_id, JSValue obj);
JSValue JS_GetClassProto(JSContext *ctx, JSClassID class_id);


/* the following functions are used to select the intrinsic object to
   save memory */
JSContext *JS_NewContextRaw(JSRuntime *rt);
void JS_AddIntrinsicBaseObjects(JSContext *ctx);
void JS_AddIntrinsicDate(JSContext *ctx);
void JS_AddIntrinsicEval(JSContext *ctx);
void JS_AddIntrinsicStringNormalize(JSContext *ctx);
void JS_AddIntrinsicRegExpCompiler(JSContext *ctx);
void JS_AddIntrinsicRegExp(JSContext *ctx);
void JS_AddIntrinsicJSON(JSContext *ctx);
void JS_AddIntrinsicProxy(JSContext *ctx);
void JS_AddIntrinsicMapSet(JSContext *ctx);
void JS_AddIntrinsicTypedArrays(JSContext *ctx);
void JS_AddIntrinsicPromise(JSContext *ctx);
void JS_AddIntrinsicBigInt(JSContext *ctx);
void JS_AddIntrinsicBigFloat(JSContext *ctx);
void JS_AddIntrinsicBigDecimal(JSContext *ctx);
/* enable operator overloading */
void JS_AddIntrinsicOperators(JSContext *ctx);
/* enable "use math" */
void JS_EnableBignumExt(JSContext *ctx, JS_BOOL enable);

JSValue js_string_codePointRange(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);


void *js_malloc(JSContext *ctx, size_t size);
void js_free(JSContext *ctx, void *ptr);
void *js_realloc(JSContext *ctx, void *ptr, size_t size);
size_t js_malloc_usable_size(JSContext *ctx, const void *ptr);
void *js_realloc2(JSContext *ctx, void *ptr, size_t size, size_t *pslack);
void *js_mallocz(JSContext *ctx, size_t size);
char *js_strdup(JSContext *ctx, const char *str);
char *js_strndup(JSContext *ctx, const char *s, size_t n);


JSValue JS_Throw(JSContext *ctx, JSValue obj);
JSValue JS_GetException(JSContext *ctx);
JS_BOOL JS_HasException(JSContext *ctx);
JS_BOOL JS_IsError(JSContext *ctx, JSValueConst val);
void JS_SetUncatchableError(JSContext *ctx, JSValueConst val, JS_BOOL flag);
void JS_ResetUncatchableError(JSContext *ctx);
JSValue JS_NewError(JSContext *ctx);
JSValue __js_printf_like(2, 3) JS_ThrowSyntaxError(JSContext *ctx, const char *fmt, ...);
JSValue __js_printf_like(2, 3) JS_ThrowTypeError(JSContext *ctx, const char *fmt, ...);
JSValue __js_printf_like(2, 3) JS_ThrowReferenceError(JSContext *ctx, const char *fmt, ...);
JSValue __js_printf_like(2, 3) JS_ThrowRangeError(JSContext *ctx, const char *fmt, ...);
JSValue __js_printf_like(2, 3) JS_ThrowInternalError(JSContext *ctx, const char *fmt, ...);
JSValue JS_ThrowOutOfMemory(JSContext *ctx);


void __JS_FreeValue(JSContext *ctx, JSValue v);
static inline void JS_FreeValue(JSContext *ctx, JSValue v)
{
    if (JS_VALUE_HAS_REF_COUNT(v)) {
        JSRefCountHeader *p = (JSRefCountHeader *)JS_VALUE_GET_PTR(v);
        if (--p->ref_count <= 0) {
            __JS_FreeValue(ctx, v);
        }
    }
}

static inline JSValue JS_DupValue(JSContext *ctx, JSValueConst v)
{
    if (JS_VALUE_HAS_REF_COUNT(v)) {
        JSRefCountHeader *p = (JSRefCountHeader *)JS_VALUE_GET_PTR(v);
        p->ref_count++;
    }
    return (JSValue)v;
}


JS_BOOL JS_StrictEq(JSContext *ctx, JSValueConst op1, JSValueConst op2);
JS_BOOL JS_SameValue(JSContext *ctx, JSValueConst op1, JSValueConst op2);
JS_BOOL JS_SameValueZero(JSContext *ctx, JSValueConst op1, JSValueConst op2);


int JS_ToBool(JSContext *ctx, JSValueConst val); /* return -1 for JS_EXCEPTION */
int JS_ToInt32(JSContext *ctx, int32_t *pres, JSValueConst val);
static inline int JS_ToUint32(JSContext *ctx, uint32_t *pres, JSValueConst val)
{
    return JS_ToInt32(ctx, (int32_t*)pres, val);
}
int JS_ToInt64(JSContext *ctx, int64_t *pres, JSValueConst val);
int JS_ToIndex(JSContext *ctx, uint64_t *plen, JSValueConst val);
int JS_ToFloat64(JSContext *ctx, double *pres, JSValueConst val);
/* return an exception if 'val' is a Number */
int JS_ToBigInt64(JSContext *ctx, int64_t *pres, JSValueConst val);
/* same as JS_ToInt64() but allow BigInt */
int JS_ToInt64Ext(JSContext *ctx, int64_t *pres, JSValueConst val);


JSValue JS_NewStringLen(JSContext *ctx, const char *str1, size_t len1);
JSValue JS_NewString(JSContext *ctx, const char *str);
JSValue JS_NewAtomString(JSContext *ctx, const char *str);
JSValue JS_ToString(JSContext *ctx, JSValueConst val);
JSValue JS_ToPropertyKey(JSContext *ctx, JSValueConst val);
const char *JS_ToCStringLen2(JSContext *ctx, size_t *plen, JSValueConst val1, JS_BOOL cesu8);
static inline const char *JS_ToCStringLen(JSContext *ctx, size_t *plen, JSValueConst val1)
{
    return JS_ToCStringLen2(ctx, plen, val1, 0);
}
static inline const char *JS_ToCString(JSContext *ctx, JSValueConst val1)
{
    return JS_ToCStringLen2(ctx, NULL, val1, 0);
}
void JS_FreeCString(JSContext *ctx, const char *ptr);


JSValue JS_NewObjectProtoClass(JSContext *ctx, JSValueConst proto, JSClassID class_id);
JSValue JS_NewObjectClass(JSContext *ctx, int class_id);
JSValue JS_NewObjectProto(JSContext *ctx, JSValueConst proto);
JSValue JS_NewObject(JSContext *ctx);

JS_BOOL JS_IsFunction(JSContext* ctx, JSValueConst val);
JS_BOOL JS_IsConstructor(JSContext* ctx, JSValueConst val);
JS_BOOL JS_SetConstructorBit(JSContext *ctx, JSValueConst func_obj, JS_BOOL val);


__EXTERN_END__
#endif // QJS_JS_CONTEXT_H_