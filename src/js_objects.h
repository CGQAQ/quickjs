#ifndef QJS_JS_OBJECTS_H_
#define QJS_JS_OBJECTS_H_
#include "./macros.h"
__EXTERN_START__

typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;

/* object class support */

typedef struct JSPropertyEnum {
    JS_BOOL is_enumerable;
    JSAtom atom;
} JSPropertyEnum;

typedef struct JSPropertyDescriptor {
    int flags;
    JSValue value;
    JSValue getter;
    JSValue setter;
} JSPropertyDescriptor;

typedef struct JSClassExoticMethods {
    /* Return -1 if exception (can only happen in case of Proxy object),
       FALSE if the property does not exists, TRUE if it exists. If 1 is
       returned, the property descriptor 'desc' is filled if != NULL. */
    int (*get_own_property)(JSContext *ctx, JSPropertyDescriptor *desc,
                             JSValueConst obj, JSAtom prop);
    /* '*ptab' should hold the '*plen' property keys. Return 0 if OK,
       -1 if exception. The 'is_enumerable' field is ignored.
    */
    int (*get_own_property_names)(JSContext *ctx, JSPropertyEnum **ptab,
                                  uint32_t *plen,
                                  JSValueConst obj);
    /* return < 0 if exception, or TRUE/FALSE */
    int (*delete_property)(JSContext *ctx, JSValueConst obj, JSAtom prop);
    /* return < 0 if exception or TRUE/FALSE */
    int (*define_own_property)(JSContext *ctx, JSValueConst this_obj,
                               JSAtom prop, JSValueConst val,
                               JSValueConst getter, JSValueConst setter,
                               int flags);
    /* The following methods can be emulated with the previous ones,
       so they are usually not needed */
    /* return < 0 if exception or TRUE/FALSE */
    int (*has_property)(JSContext *ctx, JSValueConst obj, JSAtom atom);
    JSValue (*get_property)(JSContext *ctx, JSValueConst obj, JSAtom atom,
                            JSValueConst receiver);
    /* return < 0 if exception or TRUE/FALSE */
    int (*set_property)(JSContext *ctx, JSValueConst obj, JSAtom atom,
                        JSValueConst value, JSValueConst receiver, int flags);
} JSClassExoticMethods;

typedef void JSClassFinalizer(JSRuntime *rt, JSValue val);
typedef void JSClassGCMark(JSRuntime *rt, JSValueConst val,
                           JS_MarkFunc *mark_func);
#define JS_CALL_FLAG_CONSTRUCTOR (1 << 0)
typedef JSValue JSClassCall(JSContext *ctx, JSValueConst func_obj,
                            JSValueConst this_val, int argc, JSValueConst *argv,
                            int flags);


typedef struct JSClassDef {
    const char *class_name;
    JSClassFinalizer *finalizer;
    JSClassGCMark *gc_mark;
    /* if call != NULL, the object is a function. If (flags &
       JS_CALL_FLAG_CONSTRUCTOR) != 0, the function is called as a
       constructor. In this case, 'this_val' is new.target. A
       constructor call only happens if the object constructor bit is
       set (see JS_SetConstructorBit()). */
    JSClassCall *call;
    /* XXX: suppress this indirection ? It is here only to save memory
       because only a few classes need these methods */
    JSClassExoticMethods *exotic;
} JSClassDef;

#define JS_INVALID_CLASS_ID 0
JSClassID JS_NewClassID(JSClassID *pclass_id);
/* Returns the class ID if `v` is an object, otherwise returns JS_INVALID_CLASS_ID. */
JSClassID JS_GetClassID(JSValue v);
int JS_NewClass(JSRuntime *rt, JSClassID class_id, const JSClassDef *class_def);
int JS_IsRegisteredClass(JSRuntime *rt, JSClassID class_id);




__EXTERN_END__
#endif // QJS_JS_OBJECTS_H_