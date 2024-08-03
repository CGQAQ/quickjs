#ifndef QJS_MACROS_H_
#define QJS_MACROS_H_

#if defined(__cplusplus)

#define __EXTERN_START__ extern "C" {
#define __EXTERN_END__ }

#else

#define __EXTERN_START__
#define __EXTERN_END__

#endif // defined(__cplusplus)


#if defined(__GNUC__) || defined(__clang__)
#define js_likely(x)          __builtin_expect(!!(x), 1)
#define js_unlikely(x)        __builtin_expect(!!(x), 0)
#define js_force_inline       inline __attribute__((always_inline))
#define __js_printf_like(f, a)   __attribute__((format(printf, f, a)))
#else
#define js_likely(x)     (x)
#define js_unlikely(x)   (x)
#define js_force_inline  inline
#define __js_printf_like(a, b)
#endif

#endif // QJS_MACROS_H_