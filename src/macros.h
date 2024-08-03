#ifndef QJS_MACROS_H_
#define QJS_MACROS_H_

#if defined(__cplusplus)

#define __EXTERN_START__ extern "C" {
#define __EXTERN_END__ }

#else

#define __EXTERN_START__
#define __EXTERN_END__

#endif // defined(__cplusplus)

#endif // QJS_MACROS_H_