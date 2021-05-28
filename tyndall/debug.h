#include <stdio.h>
#include <errno.h>
#include <string.h>

#define _debug_stringify(x) #x
#define debug_stringify(x) _debug_stringify(x)

#define debug_f(fd, ...) fprintf(fd, "[" __FILE__ " line\t" debug_stringify(__LINE__) "] " __VA_ARGS__)

#define debug_flush(fd, ...) do { debug_f(fd, __VA_ARGS__); fflush(fd); } while(0)

#ifdef debug_stdout
#define debug(...) debug_f(debug_stdout, __VA_ARGS__)
#define debug_plain(...) fprintf(debug_stdout, __VA_ARGS__)
#define debug_instant(...) debug_flush(debug_stdout, __VA_ARGS__)
#else
#define debug(...)
#define debug_plain(...)
#define debug_instant(...)
#endif


#ifdef debug_stderr
#define debug_color_red "\033[31m"
#define debug_color_reset   "\033[0m"
#define debug_color_bold    "\033[1m"
#define debug_error(fmt, ...) debug_f(debug_stderr, debug_color_red debug_color_bold "[errno: %s] " fmt debug_color_reset, strerror(errno) __VA_OPT__(,) __VA_ARGS__)
#define debug_error_plain(...) fprintf(debug_stderr, __VA_ARGS__);
#else
#define debug_error(...)
#define debug_error_plain(...)
#endif



#define debug_assert(cond, ...) do { if (!(cond)) { debug_error("assertion failed: (" debug_stringify(cond) ") errno: %s\n", strerror(errno)); __VA_ARGS__; } } while(0)

#define debug_assert_v(cond, ...) do { if (!(cond)) { debug_error(__VA_ARGS__); debug_error_plain("%s\n", strerror(errno)); /*exit(-1);*/ } } while(0)

#define debug_abort(...) do { debug_assert_v(false, __VA_ARGS__); exit(1); } while(0)
