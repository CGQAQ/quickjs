#ifndef QJS_JS_VALUE_H_
#define QJS_JS_VALUE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// TODO: remove this when extract to a header file
typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;
typedef struct JSObject JSObject;
typedef struct JSClass JSClass;
typedef uint32_t JSClassID;
typedef uint32_t JSAtom;

#define JS_BOOL int
#define JS_FLOAT64_NAN NAN
#define JS_VALUE_IS_BOTH_INT(v1, v2) ((JS_VALUE_GET_TAG(v1) | JS_VALUE_GET_TAG(v2)) == 0)
#define JS_VALUE_IS_BOTH_FLOAT(v1, v2) (JS_TAG_IS_FLOAT64(JS_VALUE_GET_TAG(v1)) && JS_TAG_IS_FLOAT64(JS_VALUE_GET_TAG(v2)))

#define JS_VALUE_GET_OBJ(v) ((JSObject *)JS_VALUE_GET_PTR(v))
#define JS_VALUE_GET_STRING(v) ((JSString *)JS_VALUE_GET_PTR(v))
#define JS_VALUE_HAS_REF_COUNT(v) ((unsigned)JS_VALUE_GET_TAG(v) >= (unsigned)JS_TAG_FIRST)

/* special values */
#define JS_NULL      JS_MKVAL(JS_TAG_NULL, 0)
#define JS_UNDEFINED JS_MKVAL(JS_TAG_UNDEFINED, 0)
#define JS_FALSE     JS_MKVAL(JS_TAG_BOOL, 0)
#define JS_TRUE      JS_MKVAL(JS_TAG_BOOL, 1)
#define JS_EXCEPTION JS_MKVAL(JS_TAG_EXCEPTION, 0)
#define JS_UNINITIALIZED JS_MKVAL(JS_TAG_UNINITIALIZED, 0)

#if INTPTR_MAX >= INT64_MAX
#define JS_PTR64
#define JS_PTR64_DEF(a) a
#else
#define JS_PTR64_DEF(a)
#endif

#ifndef JS_PTR64
#define JS_NAN_BOXING
#endif

enum {
    /* all tags with a reference count are negative */
    JS_TAG_FIRST       = -11, /* first negative tag */
    JS_TAG_BIG_DECIMAL = -11,
    JS_TAG_BIG_INT     = -10,
    JS_TAG_BIG_FLOAT   = -9,
    JS_TAG_SYMBOL      = -8,
    JS_TAG_STRING      = -7,
    JS_TAG_MODULE      = -3, /* used internally */
    JS_TAG_FUNCTION_BYTECODE = -2, /* used internally */
    JS_TAG_OBJECT      = -1,

    JS_TAG_INT         = 0,
    JS_TAG_BOOL        = 1,
    JS_TAG_NULL        = 2,
    JS_TAG_UNDEFINED   = 3,
    JS_TAG_UNINITIALIZED = 4,
    JS_TAG_CATCH_OFFSET = 5,
    JS_TAG_EXCEPTION   = 6,
    JS_TAG_FLOAT64     = 7,
    /* any larger tag is FLOAT64 if JS_NAN_BOXING */
};

#ifdef CONFIG_CHECK_JSVALUE
/* JSValue consistency : it is not possible to run the code in this
   mode, but it is useful to detect simple reference counting
   errors. It would be interesting to modify a static C analyzer to
   handle specific annotations (clang has such annotations but only
   for objective C) */
typedef struct __JSValue *JSValue;
typedef const struct __JSValue *JSValueConst;

#define JS_VALUE_GET_TAG(v) (int)((uintptr_t)(v) & 0xf)
/* same as JS_VALUE_GET_TAG, but return JS_TAG_FLOAT64 with NaN boxing */
#define JS_VALUE_GET_NORM_TAG(v) JS_VALUE_GET_TAG(v)
#define JS_VALUE_GET_INT(v) (int)((intptr_t)(v) >> 4)
#define JS_VALUE_GET_BOOL(v) JS_VALUE_GET_INT(v)
#define JS_VALUE_GET_FLOAT64(v) (double)JS_VALUE_GET_INT(v)
#define JS_VALUE_GET_PTR(v) (void *)((intptr_t)(v) & ~0xf)

#define JS_MKVAL(tag, val) (JSValue)(intptr_t)(((val) << 4) | (tag))
#define JS_MKPTR(tag, p) (JSValue)((intptr_t)(p) | (tag))

#define JS_TAG_IS_FLOAT64(tag) ((unsigned)(tag) == JS_TAG_FLOAT64)

#define JS_NAN JS_MKVAL(JS_TAG_FLOAT64, 1)

static inline JSValue __JS_NewFloat64(JSContext *ctx, double d)
{
    return JS_MKVAL(JS_TAG_FLOAT64, (int)d);
}

static inline JS_BOOL JS_VALUE_IS_NAN(JSValue v)
{
    return 0;
}

#elif defined(JS_NAN_BOXING)

typedef uint64_t JSValue;

#define JSValueConst JSValue

#define JS_VALUE_GET_TAG(v) (int)((v) >> 32)
#define JS_VALUE_GET_INT(v) (int)(v)
#define JS_VALUE_GET_BOOL(v) (int)(v)
#define JS_VALUE_GET_PTR(v) (void *)(intptr_t)(v)

#define JS_MKVAL(tag, val) (((uint64_t)(tag) << 32) | (uint32_t)(val))
#define JS_MKPTR(tag, ptr) (((uint64_t)(tag) << 32) | (uintptr_t)(ptr))

#define JS_FLOAT64_TAG_ADDEND (0x7ff80000 - JS_TAG_FIRST + 1) /* quiet NaN encoding */

static inline double JS_VALUE_GET_FLOAT64(JSValue v)
{
    union {
        JSValue v;
        double d;
    } u;
    u.v = v;
    u.v += (uint64_t)JS_FLOAT64_TAG_ADDEND << 32;
    return u.d;
}

#define JS_NAN (0x7ff8000000000000 - ((uint64_t)JS_FLOAT64_TAG_ADDEND << 32))

static inline JSValue __JS_NewFloat64(JSContext *ctx, double d)
{
    union {
        double d;
        uint64_t u64;
    } u;
    JSValue v;
    u.d = d;
    /* normalize NaN */
    if (js_unlikely((u.u64 & 0x7fffffffffffffff) > 0x7ff0000000000000))
        v = JS_NAN;
    else
        v = u.u64 - ((uint64_t)JS_FLOAT64_TAG_ADDEND << 32);
    return v;
}

#define JS_TAG_IS_FLOAT64(tag) ((unsigned)((tag) - JS_TAG_FIRST) >= (JS_TAG_FLOAT64 - JS_TAG_FIRST))

/* same as JS_VALUE_GET_TAG, but return JS_TAG_FLOAT64 with NaN boxing */
static inline int JS_VALUE_GET_NORM_TAG(JSValue v)
{
    uint32_t tag;
    tag = JS_VALUE_GET_TAG(v);
    if (JS_TAG_IS_FLOAT64(tag))
        return JS_TAG_FLOAT64;
    else
        return tag;
}

static inline JS_BOOL JS_VALUE_IS_NAN(JSValue v)
{
    uint32_t tag;
    tag = JS_VALUE_GET_TAG(v);
    return tag == (JS_NAN >> 32);
}

#else /* !JS_NAN_BOXING */

typedef union JSValueUnion {
    int32_t int32;
    double float64;
    void *ptr;
} JSValueUnion;

typedef struct JSValue {
    JSValueUnion u;
    int64_t tag;
} JSValue;

#define JSValueConst JSValue

#define JS_VALUE_GET_TAG(v) ((int32_t)(v).tag)
/* same as JS_VALUE_GET_TAG, but return JS_TAG_FLOAT64 with NaN boxing */
#define JS_VALUE_GET_NORM_TAG(v) JS_VALUE_GET_TAG(v)
#define JS_VALUE_GET_INT(v) ((v).u.int32)
#define JS_VALUE_GET_BOOL(v) ((v).u.int32)
#define JS_VALUE_GET_FLOAT64(v) ((v).u.float64)
#define JS_VALUE_GET_PTR(v) ((v).u.ptr)

#define JS_MKVAL(tag, val) (JSValue){ (JSValueUnion){ .int32 = val }, tag }
#define JS_MKPTR(tag, p) (JSValue){ (JSValueUnion){ .ptr = p }, tag }

#define JS_TAG_IS_FLOAT64(tag) ((unsigned)(tag) == JS_TAG_FLOAT64)

#define JS_NAN (JSValue){ .u.float64 = JS_FLOAT64_NAN, JS_TAG_FLOAT64 }

static inline JSValue __JS_NewFloat64(JSContext *ctx, double d)
{
    JSValue v;
    v.tag = JS_TAG_FLOAT64;
    v.u.float64 = d;
    return v;
}

static inline JS_BOOL JS_VALUE_IS_NAN(JSValue v)
{
    union {
        double d;
        uint64_t u64;
    } u;
    if (v.tag != JS_TAG_FLOAT64)
        return 0;
    u.d = v.u.float64;
    return (u.u64 & 0x7fffffffffffffff) > 0x7ff0000000000000;
}

#endif /* !JS_NAN_BOXING */


/* value handling */

static js_force_inline JSValue JS_NewBool(JSContext *ctx, JS_BOOL val)
{
    return JS_MKVAL(JS_TAG_BOOL, (val != 0));
}

static js_force_inline JSValue JS_NewInt32(JSContext *ctx, int32_t val)
{
    return JS_MKVAL(JS_TAG_INT, val);
}

static js_force_inline JSValue JS_NewCatchOffset(JSContext *ctx, int32_t val)
{
    return JS_MKVAL(JS_TAG_CATCH_OFFSET, val);
}

static js_force_inline JSValue JS_NewInt64(JSContext *ctx, int64_t val)
{
    JSValue v;
    if (val == (int32_t)val) {
        v = JS_NewInt32(ctx, val);
    } else {
        v = __JS_NewFloat64(ctx, val);
    }
    return v;
}

static js_force_inline JSValue JS_NewUint32(JSContext *ctx, uint32_t val)
{
    JSValue v;
    if (val <= 0x7fffffff) {
        v = JS_NewInt32(ctx, val);
    } else {
        v = __JS_NewFloat64(ctx, val);
    }
    return v;
}

JSValue JS_NewBigInt64(JSContext *ctx, int64_t v);
JSValue JS_NewBigUint64(JSContext *ctx, uint64_t v);

static js_force_inline JSValue JS_NewFloat64(JSContext *ctx, double d)
{
    int32_t val;
    union {
        double d;
        uint64_t u;
    } u, t;
    if (d >= INT32_MIN && d <= INT32_MAX) {
        u.d = d;
        val = (int32_t)d;
        t.d = val;
        /* -0 cannot be represented as integer, so we compare the bit
           representation */
        if (u.u == t.u)
            return JS_MKVAL(JS_TAG_INT, val);
    }
    return __JS_NewFloat64(ctx, d);
}

static inline JS_BOOL JS_IsNumber(JSValueConst v)
{
    int tag = JS_VALUE_GET_TAG(v);
    return tag == JS_TAG_INT || JS_TAG_IS_FLOAT64(tag);
}

static inline JS_BOOL JS_IsBigInt(JSContext *ctx, JSValueConst v)
{
    int tag = JS_VALUE_GET_TAG(v);
    return tag == JS_TAG_BIG_INT;
}

static inline JS_BOOL JS_IsBigFloat(JSValueConst v)
{
    int tag = JS_VALUE_GET_TAG(v);
    return tag == JS_TAG_BIG_FLOAT;
}

static inline JS_BOOL JS_IsBigDecimal(JSValueConst v)
{
    int tag = JS_VALUE_GET_TAG(v);
    return tag == JS_TAG_BIG_DECIMAL;
}

static inline JS_BOOL JS_IsBool(JSValueConst v)
{
    return JS_VALUE_GET_TAG(v) == JS_TAG_BOOL;
}

static inline JS_BOOL JS_IsNull(JSValueConst v)
{
    return JS_VALUE_GET_TAG(v) == JS_TAG_NULL;
}

static inline JS_BOOL JS_IsUndefined(JSValueConst v)
{
    return JS_VALUE_GET_TAG(v) == JS_TAG_UNDEFINED;
}

static inline JS_BOOL JS_IsException(JSValueConst v)
{
    return js_unlikely(JS_VALUE_GET_TAG(v) == JS_TAG_EXCEPTION);
}

static inline JS_BOOL JS_IsUninitialized(JSValueConst v)
{
    return js_unlikely(JS_VALUE_GET_TAG(v) == JS_TAG_UNINITIALIZED);
}

static inline JS_BOOL JS_IsString(JSValueConst v)
{
    return JS_VALUE_GET_TAG(v) == JS_TAG_STRING;
}

static inline JS_BOOL JS_IsSymbol(JSValueConst v)
{
    return JS_VALUE_GET_TAG(v) == JS_TAG_SYMBOL;
}

static inline JS_BOOL JS_IsObject(JSValueConst v)
{
    return JS_VALUE_GET_TAG(v) == JS_TAG_OBJECT;
}



#ifdef __cplusplus
}
#endif // __cplusplus
#endif // QJS_JS_VALUE_H_