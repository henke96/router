hc_UNUSED
static void debug_printNum(const char *pre, int64_t num, const char *post) {
    char buffer[util_INT64_MAX_CHARS];
    char *numStr = util_intToStr(&buffer[util_INT64_MAX_CHARS], num);

    sys_writev(STDOUT_FILENO, (struct iovec[3]) {
        { .iov_base = (char *)pre,  .iov_len = util_cstrLen(pre) },
        { .iov_base = numStr,       .iov_len = (int64_t)(&buffer[util_INT64_MAX_CHARS] - numStr) },
        { .iov_base = (char *)post, .iov_len = util_cstrLen(post) }
    }, 3);
}

hc_UNUSED
static noreturn void debug_fail(int64_t res, const char *expression, const char *file, int32_t line) {
    char resBuffer[util_INT64_MAX_CHARS];
    char *resStr = util_intToStr(&resBuffer[util_INT64_MAX_CHARS], res);
    char lineBuffer[util_INT32_MAX_CHARS];
    char *lineStr = util_intToStr(&lineBuffer[util_INT32_MAX_CHARS], line);

    static const char fail[7] = " fail: ";
    static const char equals[3] = " = ";
    static const char end[1] = "\n";

    sys_writev(STDOUT_FILENO, (struct iovec[9]) {
        { .iov_base = (char *)file,       .iov_len = util_cstrLen(file) },
        { .iov_base = (char *)&fail[5],   .iov_len = 1 },
        { .iov_base = lineStr,            .iov_len = (int64_t)(&lineBuffer[util_INT32_MAX_CHARS] - lineStr) },
        { .iov_base = (char *)&fail[0],   .iov_len = sizeof(fail) },
        { .iov_base = (char *)expression, .iov_len = util_cstrLen(expression) },
        { .iov_base = (char *)&equals[0], .iov_len = sizeof(equals) },
        { .iov_base = resStr,             .iov_len = (int64_t)(&resBuffer[util_INT64_MAX_CHARS] - resStr) },
        { .iov_base = (char *)&end[0],    .iov_len = sizeof(end) }
    }, 9);
    sys_kill(sys_getpid(), SIGABRT);
    sys_exit_group(137);
}

#ifdef debug_NDEBUG
    #define debug_ASSERT(EXPR, COND) ((void)0)
    #define debug_CHECK(EXPR, COND) EXPR
#else
    #define debug_ASSERT(EXPR, COND) do { typeof(EXPR) RES = (EXPR); if (!(COND)) debug_fail((int64_t)RES, #EXPR, __FILE_NAME__, __LINE__); } while (0)
    #define debug_CHECK debug_ASSERT
#endif
