/*
 * QuickJS Javascript Engine
 *
 * Copyright (c) 2017-2021 Fabrice Bellard
 * Copyright (c) 2017-2021 Charlie Gordon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef QUICKJS_H
#define QUICKJS_H

#include <stdio.h>
#include <stdint.h>

#include "src/macros.h"
#include "src/js_value.h"
#include "src/js_runtime.h"
#include "src/js_context.h"
#include "src/js_atom.h"
#include "src/js_objects.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: remove this when extract to a header file
typedef struct JSObject JSObject;
typedef struct JSClass JSClass;
typedef uint32_t JSClassID;


/* flags for object properties */
#define JS_PROP_CONFIGURABLE  (1 << 0)
#define JS_PROP_WRITABLE      (1 << 1)
#define JS_PROP_ENUMERABLE    (1 << 2)
#define JS_PROP_C_W_E         (JS_PROP_CONFIGURABLE | JS_PROP_WRITABLE | JS_PROP_ENUMERABLE)
#define JS_PROP_LENGTH        (1 << 3) /* used internally in Arrays */
#define JS_PROP_TMASK         (3 << 4) /* mask for NORMAL, GETSET, VARREF, AUTOINIT */
#define JS_PROP_NORMAL         (0 << 4)
#define JS_PROP_GETSET         (1 << 4)
#define JS_PROP_VARREF         (2 << 4) /* used internally */
#define JS_PROP_AUTOINIT       (3 << 4) /* used internally */

/* flags for JS_DefineProperty */
#define JS_PROP_HAS_SHIFT        8
#define JS_PROP_HAS_CONFIGURABLE (1 << 8)
#define JS_PROP_HAS_WRITABLE     (1 << 9)
#define JS_PROP_HAS_ENUMERABLE   (1 << 10)
#define JS_PROP_HAS_GET          (1 << 11)
#define JS_PROP_HAS_SET          (1 << 12)
#define JS_PROP_HAS_VALUE        (1 << 13)

/* throw an exception if false would be returned
   (JS_DefineProperty/JS_SetProperty) */
#define JS_PROP_THROW            (1 << 14)
/* throw an exception if false would be returned in strict mode
   (JS_SetProperty) */
#define JS_PROP_THROW_STRICT     (1 << 15)

#define JS_PROP_NO_ADD           (1 << 16) /* internal use */
#define JS_PROP_NO_EXOTIC        (1 << 17) /* internal use */

#define JS_DEFAULT_STACK_SIZE (256 * 1024)

/* JS_Eval() flags */
#define JS_EVAL_TYPE_GLOBAL   (0 << 0) /* global code (default) */
#define JS_EVAL_TYPE_MODULE   (1 << 0) /* module code */
#define JS_EVAL_TYPE_DIRECT   (2 << 0) /* direct call (internal use) */
#define JS_EVAL_TYPE_INDIRECT (3 << 0) /* indirect call (internal use) */
#define JS_EVAL_TYPE_MASK     (3 << 0)

#define JS_EVAL_FLAG_STRICT   (1 << 3) /* force 'strict' mode */
#define JS_EVAL_FLAG_STRIP    (1 << 4) /* force 'strip' mode */
/* compile but do not run. The result is an object with a
   JS_TAG_FUNCTION_BYTECODE or JS_TAG_MODULE tag. It can be executed
   with JS_EvalFunction(). */
#define JS_EVAL_FLAG_COMPILE_ONLY (1 << 5)
/* don't include the stack frames before this eval in the Error() backtraces */
#define JS_EVAL_FLAG_BACKTRACE_BARRIER (1 << 6)
/* allow top-level await in normal script. JS_Eval() returns a
   promise. Only allowed with JS_EVAL_TYPE_GLOBAL */
#define JS_EVAL_FLAG_ASYNC (1 << 7)

typedef JSValue JSCFunction(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
typedef JSValue JSCFunctionMagic(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
typedef JSValue JSCFunctionData(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data);




JSValue JS_NewArray(JSContext *ctx);
int JS_IsArray(JSContext *ctx, JSValueConst val);

JSValue JS_NewDate(JSContext *ctx, double epoch_ms);

JSValue JS_GetPropertyInternal(JSContext *ctx, JSValueConst obj,
                               JSAtom prop, JSValueConst receiver,
                               JS_BOOL throw_ref_error);
static js_force_inline JSValue JS_GetProperty(JSContext *ctx, JSValueConst this_obj,
                                              JSAtom prop)
{
    return JS_GetPropertyInternal(ctx, this_obj, prop, this_obj, 0);
}
JSValue JS_GetPropertyStr(JSContext *ctx, JSValueConst this_obj,
                          const char *prop);
JSValue JS_GetPropertyUint32(JSContext *ctx, JSValueConst this_obj,
                             uint32_t idx);

int JS_SetPropertyInternal(JSContext *ctx, JSValueConst obj,
                           JSAtom prop, JSValue val, JSValueConst this_obj,
                           int flags);
static inline int JS_SetProperty(JSContext *ctx, JSValueConst this_obj,
                                 JSAtom prop, JSValue val)
{
    return JS_SetPropertyInternal(ctx, this_obj, prop, val, this_obj, JS_PROP_THROW);
}
int JS_SetPropertyUint32(JSContext *ctx, JSValueConst this_obj,
                         uint32_t idx, JSValue val);
int JS_SetPropertyInt64(JSContext *ctx, JSValueConst this_obj,
                        int64_t idx, JSValue val);
int JS_SetPropertyStr(JSContext *ctx, JSValueConst this_obj,
                      const char *prop, JSValue val);
int JS_HasProperty(JSContext *ctx, JSValueConst this_obj, JSAtom prop);
int JS_IsExtensible(JSContext *ctx, JSValueConst obj);
int JS_PreventExtensions(JSContext *ctx, JSValueConst obj);
int JS_DeleteProperty(JSContext *ctx, JSValueConst obj, JSAtom prop, int flags);
int JS_SetPrototype(JSContext *ctx, JSValueConst obj, JSValueConst proto_val);
JSValue JS_GetPrototype(JSContext *ctx, JSValueConst val);

#define JS_GPN_STRING_MASK  (1 << 0)
#define JS_GPN_SYMBOL_MASK  (1 << 1)
#define JS_GPN_PRIVATE_MASK (1 << 2)
/* only include the enumerable properties */
#define JS_GPN_ENUM_ONLY    (1 << 4)
/* set theJSPropertyEnum.is_enumerable field */
#define JS_GPN_SET_ENUM     (1 << 5)

int JS_GetOwnPropertyNames(JSContext *ctx, JSPropertyEnum **ptab,
                           uint32_t *plen, JSValueConst obj, int flags);
int JS_GetOwnProperty(JSContext *ctx, JSPropertyDescriptor *desc,
                      JSValueConst obj, JSAtom prop);

JSValue JS_Call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj,
                int argc, JSValueConst *argv);
JSValue JS_Invoke(JSContext *ctx, JSValueConst this_val, JSAtom atom,
                  int argc, JSValueConst *argv);
JSValue JS_CallConstructor(JSContext *ctx, JSValueConst func_obj,
                           int argc, JSValueConst *argv);
JSValue JS_CallConstructor2(JSContext *ctx, JSValueConst func_obj,
                            JSValueConst new_target,
                            int argc, JSValueConst *argv);
JS_BOOL JS_DetectModule(const char *input, size_t input_len);
/* 'input' must be zero terminated i.e. input[input_len] = '\0'. */
JSValue JS_Eval(JSContext *ctx, const char *input, size_t input_len,
                const char *filename, int eval_flags);
/* same as JS_Eval() but with an explicit 'this_obj' parameter */
JSValue JS_EvalThis(JSContext *ctx, JSValueConst this_obj,
                    const char *input, size_t input_len,
                    const char *filename, int eval_flags);
JSValue JS_GetGlobalObject(JSContext *ctx);
int JS_IsInstanceOf(JSContext *ctx, JSValueConst val, JSValueConst obj);
int JS_DefineProperty(JSContext *ctx, JSValueConst this_obj,
                      JSAtom prop, JSValueConst val,
                      JSValueConst getter, JSValueConst setter, int flags);
int JS_DefinePropertyValue(JSContext *ctx, JSValueConst this_obj,
                           JSAtom prop, JSValue val, int flags);
int JS_DefinePropertyValueUint32(JSContext *ctx, JSValueConst this_obj,
                                 uint32_t idx, JSValue val, int flags);
int JS_DefinePropertyValueStr(JSContext *ctx, JSValueConst this_obj,
                              const char *prop, JSValue val, int flags);
int JS_DefinePropertyGetSet(JSContext *ctx, JSValueConst this_obj,
                            JSAtom prop, JSValue getter, JSValue setter,
                            int flags);
void JS_SetOpaque(JSValue obj, void *opaque);
void *JS_GetOpaque(JSValueConst obj, JSClassID class_id);
void *JS_GetOpaque2(JSContext *ctx, JSValueConst obj, JSClassID class_id);

/* 'buf' must be zero terminated i.e. buf[buf_len] = '\0'. */
JSValue JS_ParseJSON(JSContext *ctx, const char *buf, size_t buf_len,
                     const char *filename);
#define JS_PARSE_JSON_EXT (1 << 0) /* allow extended JSON */
JSValue JS_ParseJSON2(JSContext *ctx, const char *buf, size_t buf_len,
                      const char *filename, int flags);
JSValue JS_JSONStringify(JSContext *ctx, JSValueConst obj,
                         JSValueConst replacer, JSValueConst space0);

typedef void JSFreeArrayBufferDataFunc(JSRuntime *rt, void *opaque, void *ptr);
JSValue JS_NewArrayBuffer(JSContext *ctx, uint8_t *buf, size_t len,
                          JSFreeArrayBufferDataFunc *free_func, void *opaque,
                          JS_BOOL is_shared);
JSValue JS_NewArrayBufferCopy(JSContext *ctx, const uint8_t *buf, size_t len);
void JS_DetachArrayBuffer(JSContext *ctx, JSValueConst obj);
uint8_t *JS_GetArrayBuffer(JSContext *ctx, size_t *psize, JSValueConst obj);

typedef enum JSTypedArrayEnum {
    JS_TYPED_ARRAY_UINT8C = 0,
    JS_TYPED_ARRAY_INT8,
    JS_TYPED_ARRAY_UINT8,
    JS_TYPED_ARRAY_INT16,
    JS_TYPED_ARRAY_UINT16,
    JS_TYPED_ARRAY_INT32,
    JS_TYPED_ARRAY_UINT32,
    JS_TYPED_ARRAY_BIG_INT64,
    JS_TYPED_ARRAY_BIG_UINT64,
    JS_TYPED_ARRAY_FLOAT32,
    JS_TYPED_ARRAY_FLOAT64,
} JSTypedArrayEnum;

JSValue JS_NewTypedArray(JSContext *ctx, int argc, JSValueConst *argv,
                         JSTypedArrayEnum array_type);
JSValue JS_GetTypedArrayBuffer(JSContext *ctx, JSValueConst obj,
                               size_t *pbyte_offset,
                               size_t *pbyte_length,
                               size_t *pbytes_per_element);
typedef struct {
    void *(*sab_alloc)(void *opaque, size_t size);
    void (*sab_free)(void *opaque, void *ptr);
    void (*sab_dup)(void *opaque, void *ptr);
    void *sab_opaque;
} JSSharedArrayBufferFunctions;
void JS_SetSharedArrayBufferFunctions(JSRuntime *rt,
                                      const JSSharedArrayBufferFunctions *sf);

typedef enum JSPromiseStateEnum {
    JS_PROMISE_PENDING,
    JS_PROMISE_FULFILLED,
    JS_PROMISE_REJECTED,
} JSPromiseStateEnum;

JSValue JS_NewPromiseCapability(JSContext *ctx, JSValue *resolving_funcs);
JSPromiseStateEnum JS_PromiseState(JSContext *ctx, JSValue promise);
JSValue JS_PromiseResult(JSContext *ctx, JSValue promise);

/* is_handled = TRUE means that the rejection is handled */
typedef void JSHostPromiseRejectionTracker(JSContext *ctx, JSValueConst promise,
                                           JSValueConst reason,
                                           JS_BOOL is_handled, void *opaque);
void JS_SetHostPromiseRejectionTracker(JSRuntime *rt, JSHostPromiseRejectionTracker *cb, void *opaque);

/* return != 0 if the JS code needs to be interrupted */
typedef int JSInterruptHandler(JSRuntime *rt, void *opaque);
void JS_SetInterruptHandler(JSRuntime *rt, JSInterruptHandler *cb, void *opaque);
/* if can_block is TRUE, Atomics.wait() can be used */
void JS_SetCanBlock(JSRuntime *rt, JS_BOOL can_block);
/* set the [IsHTMLDDA] internal slot */
void JS_SetIsHTMLDDA(JSContext *ctx, JSValueConst obj);

typedef struct JSModuleDef JSModuleDef;

/* return the module specifier (allocated with js_malloc()) or NULL if
   exception */
typedef char *JSModuleNormalizeFunc(JSContext *ctx,
                                    const char *module_base_name,
                                    const char *module_name, void *opaque);
typedef JSModuleDef *JSModuleLoaderFunc(JSContext *ctx,
                                        const char *module_name, void *opaque);

/* module_normalize = NULL is allowed and invokes the default module
   filename normalizer */
void JS_SetModuleLoaderFunc(JSRuntime *rt,
                            JSModuleNormalizeFunc *module_normalize,
                            JSModuleLoaderFunc *module_loader, void *opaque);
/* return the import.meta object of a module */
JSValue JS_GetImportMeta(JSContext *ctx, JSModuleDef *m);
JSAtom JS_GetModuleName(JSContext *ctx, JSModuleDef *m);
JSValue JS_GetModuleNamespace(JSContext *ctx, JSModuleDef *m);

/* JS Job support */

typedef JSValue JSJobFunc(JSContext *ctx, int argc, JSValueConst *argv);
int JS_EnqueueJob(JSContext *ctx, JSJobFunc *job_func, int argc, JSValueConst *argv);

JS_BOOL JS_IsJobPending(JSRuntime *rt);
int JS_ExecutePendingJob(JSRuntime *rt, JSContext **pctx);

/* Object Writer/Reader (currently only used to handle precompiled code) */
#define JS_WRITE_OBJ_BYTECODE  (1 << 0) /* allow function/module */
#define JS_WRITE_OBJ_BSWAP     (1 << 1) /* byte swapped output */
#define JS_WRITE_OBJ_SAB       (1 << 2) /* allow SharedArrayBuffer */
#define JS_WRITE_OBJ_REFERENCE (1 << 3) /* allow object references to
                                           encode arbitrary object
                                           graph */
uint8_t *JS_WriteObject(JSContext *ctx, size_t *psize, JSValueConst obj,
                        int flags);
uint8_t *JS_WriteObject2(JSContext *ctx, size_t *psize, JSValueConst obj,
                         int flags, uint8_t ***psab_tab, size_t *psab_tab_len);

#define JS_READ_OBJ_BYTECODE  (1 << 0) /* allow function/module */
#define JS_READ_OBJ_ROM_DATA  (1 << 1) /* avoid duplicating 'buf' data */
#define JS_READ_OBJ_SAB       (1 << 2) /* allow SharedArrayBuffer */
#define JS_READ_OBJ_REFERENCE (1 << 3) /* allow object references */
JSValue JS_ReadObject(JSContext *ctx, const uint8_t *buf, size_t buf_len,
                      int flags);
/* instantiate and evaluate a bytecode function. Only used when
   reading a script or module with JS_ReadObject() */
JSValue JS_EvalFunction(JSContext *ctx, JSValue fun_obj);
/* load the dependencies of the module 'obj'. Useful when JS_ReadObject()
   returns a module. */
int JS_ResolveModule(JSContext *ctx, JSValueConst obj);

/* only exported for os.Worker() */
JSAtom JS_GetScriptOrModuleName(JSContext *ctx, int n_stack_levels);
/* only exported for os.Worker() */
JSValue JS_LoadModule(JSContext *ctx, const char *basename,
                      const char *filename);

/* C function definition */
typedef enum JSCFunctionEnum {  /* XXX: should rename for namespace isolation */
    JS_CFUNC_generic,
    JS_CFUNC_generic_magic,
    JS_CFUNC_constructor,
    JS_CFUNC_constructor_magic,
    JS_CFUNC_constructor_or_func,
    JS_CFUNC_constructor_or_func_magic,
    JS_CFUNC_f_f,
    JS_CFUNC_f_f_f,
    JS_CFUNC_getter,
    JS_CFUNC_setter,
    JS_CFUNC_getter_magic,
    JS_CFUNC_setter_magic,
    JS_CFUNC_iterator_next,
} JSCFunctionEnum;

typedef union JSCFunctionType {
    JSCFunction *generic;
    JSValue (*generic_magic)(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
    JSCFunction *constructor;
    JSValue (*constructor_magic)(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv, int magic);
    JSCFunction *constructor_or_func;
    double (*f_f)(double);
    double (*f_f_f)(double, double);
    JSValue (*getter)(JSContext *ctx, JSValueConst this_val);
    JSValue (*setter)(JSContext *ctx, JSValueConst this_val, JSValueConst val);
    JSValue (*getter_magic)(JSContext *ctx, JSValueConst this_val, int magic);
    JSValue (*setter_magic)(JSContext *ctx, JSValueConst this_val, JSValueConst val, int magic);
    JSValue (*iterator_next)(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, int *pdone, int magic);
} JSCFunctionType;

JSValue JS_NewCFunction2(JSContext *ctx, JSCFunction *func,
                         const char *name,
                         int length, JSCFunctionEnum cproto, int magic);
JSValue JS_NewCFunctionData(JSContext *ctx, JSCFunctionData *func,
                            int length, int magic, int data_len,
                            JSValueConst *data);

static inline JSValue JS_NewCFunction(JSContext *ctx, JSCFunction *func, const char *name,
                                      int length)
{
    return JS_NewCFunction2(ctx, func, name, length, JS_CFUNC_generic, 0);
}

static inline JSValue JS_NewCFunctionMagic(JSContext *ctx, JSCFunctionMagic *func,
                                           const char *name,
                                           int length, JSCFunctionEnum cproto, int magic)
{
    return JS_NewCFunction2(ctx, (JSCFunction *)func, name, length, cproto, magic);
}
void JS_SetConstructor(JSContext *ctx, JSValueConst func_obj,
                       JSValueConst proto);

/* C property definition */

typedef struct JSCFunctionListEntry {
    const char *name;
    uint8_t prop_flags;
    uint8_t def_type;
    int16_t magic;
    union {
        struct {
            uint8_t length; /* XXX: should move outside union */
            uint8_t cproto; /* XXX: should move outside union */
            JSCFunctionType cfunc;
        } func;
        struct {
            JSCFunctionType get;
            JSCFunctionType set;
        } getset;
        struct {
            const char *name;
            int base;
        } alias;
        struct {
            const struct JSCFunctionListEntry *tab;
            int len;
        } prop_list;
        const char *str;
        int32_t i32;
        int64_t i64;
        double f64;
    } u;
} JSCFunctionListEntry;

#define JS_DEF_CFUNC          0
#define JS_DEF_CGETSET        1
#define JS_DEF_CGETSET_MAGIC  2
#define JS_DEF_PROP_STRING    3
#define JS_DEF_PROP_INT32     4
#define JS_DEF_PROP_INT64     5
#define JS_DEF_PROP_DOUBLE    6
#define JS_DEF_PROP_UNDEFINED 7
#define JS_DEF_OBJECT         8
#define JS_DEF_ALIAS          9

/* Note: c++ does not like nested designators */
#define JS_CFUNC_DEF(name, length, func1) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, .u = { .func = { length, JS_CFUNC_generic, { .generic = func1 } } } }
#define JS_CFUNC_MAGIC_DEF(name, length, func1, magic) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, magic, .u = { .func = { length, JS_CFUNC_generic_magic, { .generic_magic = func1 } } } }
#define JS_CFUNC_SPECIAL_DEF(name, length, cproto, func1) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, .u = { .func = { length, JS_CFUNC_ ## cproto, { .cproto = func1 } } } }
#define JS_ITERATOR_NEXT_DEF(name, length, func1, magic) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, magic, .u = { .func = { length, JS_CFUNC_iterator_next, { .iterator_next = func1 } } } }
#define JS_CGETSET_DEF(name, fgetter, fsetter) { name, JS_PROP_CONFIGURABLE, JS_DEF_CGETSET, 0, .u = { .getset = { .get = { .getter = fgetter }, .set = { .setter = fsetter } } } }
#define JS_CGETSET_MAGIC_DEF(name, fgetter, fsetter, magic) { name, JS_PROP_CONFIGURABLE, JS_DEF_CGETSET_MAGIC, magic, .u = { .getset = { .get = { .getter_magic = fgetter }, .set = { .setter_magic = fsetter } } } }
#define JS_PROP_STRING_DEF(name, cstr, prop_flags) { name, prop_flags, JS_DEF_PROP_STRING, 0, .u = { .str = cstr } }
#define JS_PROP_INT32_DEF(name, val, prop_flags) { name, prop_flags, JS_DEF_PROP_INT32, 0, .u = { .i32 = val } }
#define JS_PROP_INT64_DEF(name, val, prop_flags) { name, prop_flags, JS_DEF_PROP_INT64, 0, .u = { .i64 = val } }
#define JS_PROP_DOUBLE_DEF(name, val, prop_flags) { name, prop_flags, JS_DEF_PROP_DOUBLE, 0, .u = { .f64 = val } }
#define JS_PROP_UNDEFINED_DEF(name, prop_flags) { name, prop_flags, JS_DEF_PROP_UNDEFINED, 0, .u = { .i32 = 0 } }
#define JS_OBJECT_DEF(name, tab, len, prop_flags) { name, prop_flags, JS_DEF_OBJECT, 0, .u = { .prop_list = { tab, len } } }
#define JS_ALIAS_DEF(name, from) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_ALIAS, 0, .u = { .alias = { from, -1 } } }
#define JS_ALIAS_BASE_DEF(name, from, base) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_ALIAS, 0, .u = { .alias = { from, base } } }

void JS_SetPropertyFunctionList(JSContext *ctx, JSValueConst obj,
                                const JSCFunctionListEntry *tab,
                                int len);

/* C module definition */

typedef int JSModuleInitFunc(JSContext *ctx, JSModuleDef *m);

JSModuleDef *JS_NewCModule(JSContext *ctx, const char *name_str,
                           JSModuleInitFunc *func);
/* can only be called before the module is instantiated */
int JS_AddModuleExport(JSContext *ctx, JSModuleDef *m, const char *name_str);
int JS_AddModuleExportList(JSContext *ctx, JSModuleDef *m,
                           const JSCFunctionListEntry *tab, int len);
/* can only be called after the module is instantiated */
int JS_SetModuleExport(JSContext *ctx, JSModuleDef *m, const char *export_name,
                       JSValue val);
int JS_SetModuleExportList(JSContext *ctx, JSModuleDef *m,
                           const JSCFunctionListEntry *tab, int len);

#undef js_unlikely
#undef js_force_inline

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* QUICKJS_H */
