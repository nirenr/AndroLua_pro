#ifndef BACKTRACE_H
#define BACKTRACE_H

const char* stacktrace();
const char* stacktrace_detailed(bool detailed, siginfo_t* sig_info, void* sig_context);

#if defined(DEBUG) || defined(STACKTRACE)
#define print_stacktrace() print_stacktrace_(__FILE__, __LINE__, __func__)
#else
#define print_stacktrace()
#endif

void print_stacktrace_(const char* file, int line, const char* func);

#endif /* BACKTRACE_H */
