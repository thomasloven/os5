#pragma once

#ifndef __ASSEMBLER__

#define BREAK asm volatile ("xchg %bx, %bx;");

#ifdef NDEBUG
#define debug_print(M, ...)
#else
#define debug_print(M, ...) debug("DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define log_err(M, ...) debug("[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_warn(M, ...) debug("[WARNING] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(M, ...) debug("[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) {if(!(A)) { log_err(M, ##__VA_ARGS__); goto error; }}
#define sentinel(M, ...) { log_err(M, ##__VA_ARGS__); goto error; }
#define check_mem(A) check((A), "Out of memory")
#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); goto error; }

#endif

#ifdef __ASSEMBLER__

#define BREAK xchg bx, bx

#endif
