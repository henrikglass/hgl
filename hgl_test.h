
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_test.h is a minimal header-only unit testing framework with automatic test
 * registration, inspired by Snaipe/Criterion.
 *
 *
 * USAGE:
 *
 * Simply create a C source file and include hgl_test.h at the very top. hgl_test.h
 * defines its own main function, and automatically registers and runs tests defined
 * after its inclusion. Actually, just including `hgl_test.h` is enough to create a valid
 * program (it just won't run any tests). Each test is run inside its own process,
 * which is what allows tests to crash, print stuff to stdout*, and make changes to
 * the program state without it affecting other tests.
 *
 * You can define tests using the `TEST`-macro (described below). Optionally, you can
 * also define custom setup- and teardown routines that are run before and after the
 * tests using the `GLOBAL_SETUP` and `GLOBAL_TEARDOWN` macros (again, described below).
 * The `ASSERT` macro may be used to assert conditions inside tests. The `LOG` macro
 * may be used in place of `printf` to print stuff without affecting the output channel.
 *
 *     *It's not actually stdout (or stdin for that matter). stdin and stdout are
 *      replaced with pipes that the parent process can write to and read from and
 *      thus be able to, for instance, verify output with the `.expect_output`
 *      option.
 *
 * Compile the program and run it like this:
 *
 *     $ gcc my_test_program.c -o test
 *     $ ./test --help
 *     Usage: ./test [options]
 *     Options:
 *       -s,--silent              Don't show log messages or verbose errors. (default = 0)
 *       -ff,--fail-fast          Stop running tests after first failing test (default = 0)
 *       -sof,--show-only-fails   Only show failed tests in the test summary (default = 0)
 *       -h,--help                Display this help message (default = 0)
 *
 * Note: hgl_test.h depends on hgl_flags.h (for now). Both files must be present in
 *       your project.
 *
 *
 * EXAMPLE:
 *
 *     #include "hgl_test.h"
 *
 *     #include "other_tests.c"
 *     #include "more_other_tests.c"
 *     #include "even_more.c"
 *
 *     static FILE *fp;
 *
 *     GLOBAL_SETUP
 *     {
 *         fp = fopen("test.data", "rb");
 *     }
 *
 *     GLOBAL_TEARDOWN
 *     {
 *         fclose(fp);
 *     }
 *
 *     TEST(five_ants)
 *     {
 *         int n_ants = 5;
 *         int n_elephants = 4;
 *         ASSERT(n_ants > n_elephants);
 *     }
 *
 *     TEST(should_segfault, .expect_signal = SIGSEGV)
 *     {
 *         int *ptr = (int *) NULL;
 *         *ptr = 69;
 *     }
 *
 *     TEST(should_print_hello_word, .expect_output = "Hello World!\n")
 *     {
 *         printf("Hello World!\n");
 *     }
 *
 *     void reset_fp()
 *     {
 *         rewind(fp);
 *     }
 *
 *     TEST(assert_file_is_not_empty, .setup = reset_fp)
 *     {
 *         fseek(fp, 0L, SEEK_END);
 *         ASSERT(ftell(fp) > 0);
 *     }
 *
 *     TEST(test_to_upper, .input = "hello :3\n", .expect_output = "HELLO :3\n")
 *     {
 *         char c;
 *         do {
 *             c = getchar();
 *             c = (c >= 'a' && c <= 'z') ? c - 0x20 : c;
 *             putchar(c);
 *         } while(c != '\n');
 *     }
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_TEST_H
#define HGL_TEST_H

#define _POSIX_C_SOURCE 200809L

/*--- Include files ---------------------------------------------------------------------*/

#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

/*--- Public macros ---------------------------------------------------------------------*/

/* These are exit codes from the child test process */
#define EXIT_CODE_SUCCESS                         (0)
#define EXIT_CODE_ASSERT_FAIL               (1u << 0)
#define EXIT_CODE_CAUGHT_EXPECTED_SIGNAL    (1u << 1)

/* Result status codes */
#define EXPECT_OUTPUT_FAIL                  (1u << 1)
#define EXPECT_SIGNAL_FAIL                  (1u << 2)
#define GOT_UNEXPECTED_SIGNAL_FAIL          (1u << 3)
#define EXPECT_EXIT_CODE_FAIL               (1u << 4)
#define TIMEOUT_FAIL                        (1u << 5)
#define ASSERTION_FAIL                      (1u << 6)

/* Style thingies */
#define ANSI_RED       "\033[31m"
#define ANSI_GREEN     "\033[32m"
#define ANSI_AMBER     "\033[33m"
#define ANSI_MAGENTA   "\033[35m"
#define ANSI_NC        "\033[0m"
#define ANSI_BOLD      "\e[1m"
#define ANSI_UNDERLINE "\e[4m"
#define ANSI_NS        "\e[m"
#define AMBER_PLUS ANSI_AMBER ANSI_BOLD "+" ANSI_NC ANSI_NS

/* Macros for user */

/**
 * The `GLOBAL_SETUP` macro is used to register a user-defined global setup
 * function that is run before all tests. See `hgl_test_global_setup`.
 *
 * Example:
 *
 *     GLOBAL_SETUP {
 *         my_global_file_handle = fopen("test.txt", "rb");
 *     }
 *
 */
#define GLOBAL_SETUP void hgl_test_global_setup(void)

/**
 * The `GLOBAL_TEARDOWN` macro is used to register a user-defined global teardown
 * function that is run after all tests have completed. See `hgl_test_global_teardown`.
 *
 * Example:
 *
 *     GLOBAL_TEARDOWN {
 *         fclose(my_global_file_handle);
 *     }
 *
 */
#define GLOBAL_TEARDOWN void hgl_test_global_teardown(void)

/**
 * The `TEST` macro is used to register a user-defined test. The `TEST` macro
 * accepts a name as the first and only required argument. In the variadic
 * arguments list, the following may be defined (shown as examples):
 *
 *     .input = "hello"              // Put "hello" on stdin*
 *     .expect_output = "goodbye"    // Expect "goodbye" on stdout* (otherwise fail test)
 *     .expect_signal = SIGSEGV      // Expect test to generate a segmentation fault (otherwise fail test)
 *     .expect_fail = true           // Expect test to fail (otherwise fail test)
 *     .expect_exit_code = 13        // Expect test to exit with code 13 (will override .expect_signal and 
 *                                   // cause any failed ASSERTS to PASS)
 *     .timeout = 10                 // Fail test automatically after 10 seconds
 *     .setup = my_setup_func        // Run `my_setup_func` before spawning the test process
 *     .teardown = my_teardown_func  // Run `my_teardown_func` after the test process exits
 *
 * *NOTE: It's not really stdin and stdout.
 *
 * After the macro, a function body is expected. This function contains the
 * test code. Each test is run as a separete process, which is what allows
 * tests to, for instance, crash, call printf, leak memory, and make whatever
 * changes to the program state it wants without affecting any other tests.
 * Tests registered with the `TEST` macro are automatically looked up and run
 * by hgl_test.h using ₊‧.°.⋆magic linker fuckery₊‧.°.⋆.
 *
 * Example:
 *
 *     TEST(my_test_name, .expect_output = "Hello World!\n", .expect_signal = SIGSEGV) {
 *         printf("Hello World!\n");
 *         int *ptr = (int *) NULL;
 *         *ptr = 1337;
 *     }
 *
 */
#define TEST(name_, ...)                                                 \
    void test_fn_##name_(void);                                          \
    static const HglTest name_                                           \
    __attribute__((used, __section__("hgl_test_vtable"))) = {            \
        .hidden__.name    = __FILE__ ": " #name_,                        \
        .hidden__.test_fn = test_fn_##name_,                             \
        .hidden__.id      = __COUNTER__,                                 \
        __VA_ARGS__                                                      \
    };                                                                   \
    void test_fn_##name_()

/**
 * The `ASSERT` macro may be used to assert expressions inside a
 * user-defined test.
 */
#define ASSERT(cond_)                                                    \
    do {                                                                 \
        if (!(cond_)) {                                                  \
            if (!hgl_test_opt_silent__) {                                \
                fprintf(stderr, ANSI_BOLD ANSI_RED "  ASSERTION FAILED"  \
                        ANSI_NC ANSI_NS ": `%s` <%s:%d>\n", #cond_,      \
                        __FILE__, __LINE__);                             \
            }                                                            \
            exit(EXIT_CODE_ASSERT_FAIL);                                 \
        }                                                                \
    } while (0)

/**
 * The `ASSERT_CSTR_EQ` macro may be used to assert equality of two 
 * null-terminated strings inside a user-defined test.
 */
#define ASSERT_CSTR_EQ(a_, b_)                                           \
    do {                                                                 \
        if (strcmp((a_), (b_)) != 0) {                                   \
            if (!hgl_test_opt_silent__) {                                \
                fprintf(stderr, ANSI_BOLD ANSI_RED "  ASSERTION FAILED"  \
                        ANSI_NC ANSI_NS ": `%s` == `%s` <%s:%d>\n",      \
                        #a_, #b_, __FILE__, __LINE__);                   \
            }                                                            \
            exit(EXIT_CODE_ASSERT_FAIL);                                 \
        }                                                                \
    } while (0)

/**
 * The `ASSERT_CSTR_NEQ` macro may be used to assert nonequality of two 
 * null-terminated strings inside a user-defined test.
 */
#define ASSERT_CSTR_NEQ(a_, b_)                                          \
    do {                                                                 \
        if (strcmp((a_), (b_)) == 0) {                                   \
            if (!hgl_test_opt_silent__) {                                \
                fprintf(stderr, ANSI_BOLD ANSI_RED "  ASSERTION FAILED"  \
                        ANSI_NC ANSI_NS ": `%s` != `%s` <%s:%d>\n",      \
                        #a_, #b_, __FILE__, __LINE__);                   \
            }                                                            \
            exit(EXIT_CODE_ASSERT_FAIL);                                 \
        }                                                                \
    } while (0)

/**
 * The `LOG` macro may be used in place of regular the regular printf to
 * actually print stuff to the terminal. For simplicity, the `LOG` macro
 * just prints stuff to stderr.
 */
#define LOG(...)                                                         \
    do {                                                                 \
        if (!hgl_test_opt_silent__) {                                    \
            fprintf(stderr, ANSI_BOLD "  LOG" ANSI_NS ": " __VA_ARGS__); \
            fflush(stderr);                                              \
        }                                                                \
    } while (0)

/**
 * The `FILE_CONTENTS` macro can be used to extract then contents of a file
 * as a null-terminated string. Can be useful, for instance, in combination
 * with the `ASSERT_CSTR_EQ` macro.
 */
#define FILE_CONTENTS(filepath_) hgl_test_read_file((filepath_))

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct __attribute__((aligned (32)))
{
    struct {
        const char *name;
        void (*test_fn)(void);
        uint32_t result;
        uint32_t id;
        uint8_t exit_code;
        bool pass;
    } hidden__; /* i.e. don't touch this */

    /* user defined */
    const char *input;            // Start test with `input` on "stdin".
    const char *expect_output;    // Expect test to terminate with `expect_output` on "stdout".
    int         expect_signal;    // Expect test to exit with signal, otherwise fail test. E.g. `.expect_signal = SIGSEGV`
    bool        expect_fail;      // Expect test to fail. Reports failure as success and vice versa.
    uint8_t     expect_exit_code; // Expect test to terminate with the specified exit code.
    float       timeout;          // Fail test if it hasn't returned after the specified number of seconds
    void (*setup)(void);          // Specify a test-specific setup function to be run before the test.
    void (*teardown)(void);       // Specify a test-specific teardown function to be run after the test.
} HglTest;

/*
 * GCC seems to place symbols at 32-byte boundaries when creating objects
 * inside the `hgl_test_vtable` section. I'm pretty sure this entirely up
 * to the compiler/platform, thus not very portable. Making sure HglTest
 * is a multiple of this size ensures that we can simply iterate a
 * HglTest pointer and end up at the correct address for the next item
 * in the table.
 *
 * TODO figure out a more robust way to do this.
 */
static_assert(sizeof(HglTest) % 32 == 0, "");

/*--- Public variables ------------------------------------------------------------------*/

static bool hgl_test_opt_silent__;
static bool hgl_test_opt_fail_fast__;
static bool hgl_test_opt_show_only_fails__;

/*--- Public function prototypes --------------------------------------------------------*/

/**
 * Optional user-defined setup function. If this function is defined, then it is run once,
 * before the first test is run. You may use the `GLOBAL_SETUP` macro to define this function.
 */
void hgl_test_global_setup(void) __attribute__((weak));

/**
 * Optional user-defined teardown function. If this function is defined, then it is run once,
 * after the last test is run. You may use the `GLOBAL_TEARDOWN` macro to define this function.
 *
 *     GLOBAL_TEARDOWN {
 *         // do teardown
 *     }
 */
void hgl_test_global_teardown(void) __attribute__((weak));

/**
 * A signal handler that is registered by the test process for whatever signal it is
 * expected to exit with. If the test is not expected to exit with a particular signal
 * then `hgl_test_signal_handler` is never registered.
 */
void hgl_test_signal_handler(int sig);

/**
 * Prints a cstr with all the newlines, tabs, carriage returns, etc. escaped.
 */
void hgl_test_print_escaped(const char *cstr);

/**
 * Returns the contents of the file at `filepath` as a null-terminated string. The pointer
 * returned from this function should be freed, but honestly, it's a unit testing framework,
 * so who cares.
 */
char *hgl_test_read_file(const char *filepath);

/**
 * Runs test `test`. The test is run in a separate process by calling `fork` before the
 * user-defined test function (See the TEST macro) is run. This way the test is allowed
 * to crash and make whatever changes it wants to the program state without affecting
 * other tests.
 */
void hgl_test_run_test(HglTest *test);

/*--- Public functions ------------------------------------------------------------------*/

void hgl_test_signal_handler(int sig)
{
    (void) sig;
    exit(EXIT_CODE_CAUGHT_EXPECTED_SIGNAL);
}

void hgl_test_print_escaped(const char *cstr)
{
    size_t str_len = strlen(cstr);
    for (size_t i = 0; i < str_len; i++) {
        switch (cstr[i]) {
            case '\n': fprintf(stderr, "\\n"); break;
            case '\r': fprintf(stderr, "\\r"); break;
            case '\t': fprintf(stderr, "\\t"); break;
            case '\v': fprintf(stderr, "\\v"); break;
            default: putc(cstr[i], stderr);
        }
    }
}

char *hgl_test_read_file(const char *filepath)
{
    /* open file in read binary mode */
    FILE *fp = fopen(filepath, "rb"); 
    if (fp == NULL) {
        ASSERT(0 && "[hgl_test_read_file]: Error opening file");
    }

    /* get file size */
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);
    rewind(fp);
    if (file_size < 0) {
        ASSERT(0 && "[hgl_test_read_file]: Error getting file size");
    }

    /* allocate memory for data */
    char *data = malloc(file_size + 1);
    if (data == NULL) {
        ASSERT(0 && "[hgl_test_read_file]: Error calling malloc");
    }

    /* read file data */
    ssize_t n_read_bytes = fread(data, 1, file_size, fp);
    if (n_read_bytes != file_size) {
        ASSERT(0 && "[hgl_test_read_file]: Error reading data from file");
    }

    /* append null byte */
    data[file_size] = '\0';

    return data;
}

void hgl_test_run_test(HglTest *test)
{
    static char stdout_buffer[0x10000] = {0}; // 64 KiB
    bool pass = true;
    int err;
    int pipes[2][2]; // {{input read end, input write end},
                     //  {output read end, output write end}}
    err  = pipe(pipes[0]);
    err |= pipe(pipes[1]);
    assert(err != -1);

    if (!hgl_test_opt_silent__) {
        fprintf(stderr, "[" ANSI_MAGENTA ANSI_BOLD "%u" ANSI_NS ANSI_NC "] %s:\n",
                        test->hidden__.id, test->hidden__.name);
    }

    test->hidden__.result = 0; // clear test result

    pid_t pid = fork();

    /* ======== child ======== */
    if (pid == 0) {
        /* maybe register signal handler*/
        if (test->expect_signal != 0) {
            signal(test->expect_signal, hgl_test_signal_handler);
        }

        /* Replace stdin & stdout with respective pipe and close unused ends */
        close(pipes[0][1]);
        dup2(pipes[0][0], STDIN_FILENO);
        close(pipes[1][0]);
        dup2(pipes[1][1], STDOUT_FILENO);

        /* execute setup function before test, if it exists */
        if (test->setup != NULL) {
            (test->setup)();
        }

        /* execute test */
        (test->hidden__.test_fn)();

        /* execute teardown function after test, if it exists */
        if (test->teardown != NULL) {
            (test->teardown)();
        }

        /*
         * close remaining pipe ends (Immediately before process exits, because we
         * are being polite to the OS :3)
         */
        close(pipes[0][0]);
        close(pipes[1][1]);

        /* exit  */
        exit(EXIT_CODE_SUCCESS);
        assert(0 && "unreachable");
    }

    /* ======== parent ======== */

    /* maybe write input */
    if (test->input != NULL) {
        write(pipes[0][1], test->input, strlen(test->input));
    }

    /* close unused pipe ends */
    close(pipes[0][0]);
    close(pipes[1][1]);

    /* maybe clear stdout_buffer */
    if (test->expect_output != NULL) {
        memset(stdout_buffer, 0, sizeof(stdout_buffer));
    }

    /* handle timeout */
    if (test->timeout != 0.0f) {
        sigset_t mask;
        sigset_t orig_mask;
        
        assert(test->timeout > 0.0f);
        struct timespec timeout = {
            .tv_sec  = (uint64_t) test->timeout,
            .tv_nsec = (uint64_t) 1000000000.0f*(test->timeout - (int) test->timeout),
        };

        sigemptyset(&mask);
        sigaddset(&mask, SIGCHLD);
        assert(0 == sigprocmask(SIG_BLOCK, &mask, &orig_mask));
        while (sigtimedwait(&mask, NULL, &timeout) < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN) {
                kill (pid, SIGKILL);
                test->hidden__.result |= TIMEOUT_FAIL;
                pass = false;
                break;
            }
        }
        assert(0 == sigprocmask(SIG_UNBLOCK, &mask, &orig_mask));
    }

    /* wait for process to die. */
    pid_t wait_pid;
    int wstatus = 0;
    while((wait_pid = waitpid(pid, &wstatus, 0)) != pid) {
        assert(wait_pid != -1);
    }

    /* maybe read output */
    if (test->expect_output != NULL) {
        read(pipes[1][0],
             stdout_buffer, sizeof(stdout_buffer) - 1);
    }

    /* close remaining pipe ends */
    close(pipes[0][1]);
    close(pipes[1][0]);

    /* determine test result */
    test->hidden__.exit_code = WEXITSTATUS(wstatus);

    /* handle unexpected signal */
    if (WIFSIGNALED(wstatus) && (0 == (test->hidden__.result & TIMEOUT_FAIL))) {
        test->hidden__.result |= GOT_UNEXPECTED_SIGNAL_FAIL;
        pass = false;
    }

    /* handle exit code */
    if (test->expect_exit_code != 0) {
        /* expect specific exit code */
        if (test->hidden__.exit_code != test->expect_exit_code) {
            test->hidden__.result |= EXPECT_EXIT_CODE_FAIL;
            pass = false;
        }
    } else {
        /* hgl_test.h exit code semantics */
        if ((test->expect_signal != 0) &&
            (test->hidden__.exit_code != EXIT_CODE_CAUGHT_EXPECTED_SIGNAL)) {
            test->hidden__.result |= EXPECT_SIGNAL_FAIL;
            pass = false;
        }
    
        if (test->hidden__.exit_code == EXIT_CODE_ASSERT_FAIL) {
            test->hidden__.result |= ASSERTION_FAIL;
            pass = false;
        }
    }

    /* handle .expect_output */
    if (test->expect_output != NULL) {
        if ((strlen(stdout_buffer) != strlen(test->expect_output)) ||
            (strcmp(stdout_buffer, test->expect_output) != 0)) {
            test->hidden__.result |= EXPECT_OUTPUT_FAIL;
            pass = false;
        }
    }

    /* handle .expect_fail */
    pass = (test->expect_fail) ? !pass : pass;

    /* Print errors */
    if (!pass & !hgl_test_opt_silent__) {
        if (test->hidden__.result & EXPECT_OUTPUT_FAIL) {
            fprintf(stderr, ANSI_RED ANSI_BOLD "  OUTPUT ERROR" ANSI_NS ANSI_NC ": ");
            fprintf(stderr, ANSI_BOLD " Got output: " ANSI_NS "\"");
            hgl_test_print_escaped(stdout_buffer);
            fprintf(stderr, "\"");
            fprintf(stderr, ANSI_BOLD " Expected output: " ANSI_NS "\"");
            hgl_test_print_escaped(test->expect_output);
            fprintf(stderr, "\"\n");
        }

        if (test->hidden__.result & GOT_UNEXPECTED_SIGNAL_FAIL) {
            fprintf(stderr, ANSI_RED ANSI_BOLD "  SIGNAL ERROR" ANSI_NS ANSI_NC ": ");
            fprintf(stderr, ANSI_BOLD " Terminated by signal: " ANSI_NS "%d (%s)\n",
                            WTERMSIG(wstatus), strsignal(WTERMSIG(wstatus)));
        }

        if (test->hidden__.result & EXPECT_SIGNAL_FAIL) {
            fprintf(stderr, ANSI_RED ANSI_BOLD "  SIGNAL ERROR" ANSI_NS ANSI_NC ": ");
            fprintf(stderr, ANSI_BOLD " Expected termination by signal: " ANSI_NS "%d (%s)\n",
                            test->expect_signal, strsignal(test->expect_signal));
        }

        if (test->hidden__.result & EXPECT_EXIT_CODE_FAIL) {
            fprintf(stderr, ANSI_RED ANSI_BOLD "  EXIT CODE ERROR" ANSI_NS ANSI_NC ": ");
            fprintf(stderr, ANSI_BOLD " Expected exit code: " ANSI_NS "%d  " ANSI_BOLD 
                            "Got: " ANSI_NS "%d\n", test->expect_exit_code, WEXITSTATUS(wstatus));
        }

        if (test->hidden__.result & TIMEOUT_FAIL) {
            fprintf(stderr, ANSI_RED ANSI_BOLD "  TIMED OUT" ANSI_NS ANSI_NC ": ");
            fprintf(stderr, ANSI_BOLD " Test took more than: " ANSI_NS "%f seconds\n", (double) test->timeout);
        }
    }

    test->hidden__.pass = pass;
}

/*--- Main ------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    HglTest *test;
    extern HglTest __start_hgl_test_vtable;
    extern HglTest __stop_hgl_test_vtable;
    size_t n_tests = ((uint8_t *)&__stop_hgl_test_vtable -
                      (uint8_t *)&__start_hgl_test_vtable)
                      / sizeof(HglTest);
    size_t n_passing = 0;

    /* Parse cli options */
    bool *opt_silent          = hgl_flags_add_bool("-s,--silent", "Don't show log messages or verbose errors.", false, 0);
    bool *opt_fail_fast       = hgl_flags_add_bool("-ff,--fail-fast", "Stop running tests after first failing test", false, 0);
    bool *opt_show_only_fails = hgl_flags_add_bool("-sof,--show-only-fails", "Only show failed tests in the test summary", false, 0);
    bool *opt_help            = hgl_flags_add_bool("-h,--help", "Display this help message", false, 0);

    if (hgl_flags_parse(argc, argv) != 0 || *opt_help) {
        printf("Usage: %s [options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }

    /* Copy options and reset the global hgl_flags state so that the tests may use hgl_flags.h as well */
    hgl_test_opt_silent__          = *opt_silent;
    hgl_test_opt_fail_fast__       = *opt_fail_fast;
    hgl_test_opt_show_only_fails__ = *opt_show_only_fails;
    hgl_flags_reset();

    /* run setup, if it exists */
    if (hgl_test_global_setup != NULL) {
        hgl_test_global_setup();
    }

    /* run all registered tests */
    bool failed = false;
    for (test = &__start_hgl_test_vtable; test != &__stop_hgl_test_vtable; test++) {
        hgl_test_run_test(test);
        if (hgl_test_opt_fail_fast__ && !test->hidden__.pass) {
            failed = true;
            break; /* exit prematurely */
        }
    }

    /* run teardown, if it exists */
    if (hgl_test_global_teardown != NULL) {
        hgl_test_global_teardown();
    }

    /* Skip test summary if --fail-fast is enabled and a test failed */
    if (hgl_test_opt_fail_fast__ && failed) {
        return 1;
    }

    /* print test summary */
    printf("\n\e[4m\e[1m%-8s %-59s %-64s\e[m\n", " ID:", "File/Test:", "Result (cause):");
    for (test = &__start_hgl_test_vtable; test != &__stop_hgl_test_vtable; test++) {
        if (test->hidden__.pass) {
            n_passing++;
        }
        if (hgl_test_opt_show_only_fails__ && test->hidden__.pass) {
            continue;
        }
        printf("%7u| %-58s | %s   ",
               test->hidden__.id,
               test->hidden__.name,
               test->hidden__.pass ? ANSI_GREEN ANSI_BOLD "PASS \u2713" ANSI_NC ANSI_NS
                                   : ANSI_RED ANSI_BOLD "FAIL \u2715" ANSI_NC ANSI_NS);
        if (!test->hidden__.pass) {
            if (test->hidden__.result & ASSERTION_FAIL) {
                printf(AMBER_PLUS "Assertion failed ");
            }
            if (test->hidden__.result & EXPECT_OUTPUT_FAIL) {
                printf(AMBER_PLUS "Incorrect output ");
            }
            if (test->hidden__.result & EXPECT_SIGNAL_FAIL) {
                printf(AMBER_PLUS "Did not exit with expected signal ");
            }
            if (test->hidden__.result & GOT_UNEXPECTED_SIGNAL_FAIL) {
                printf(AMBER_PLUS "Exited with unexpected signal ");
            }
            if (test->hidden__.result & EXPECT_EXIT_CODE_FAIL) {
                printf(AMBER_PLUS "Exited with unexpected exit code ");
            }
            if (test->hidden__.result & TIMEOUT_FAIL) {
                printf(AMBER_PLUS "Timed out ");
            }
        }
        printf("\n");
    }
    printf("\n\e[1mTest results:\e[m %zu/%zu tests passed.\n\n", n_passing, n_tests);

    return (n_passing == n_tests) ? 0 : 1;
}

#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif /* HGL_TEST_H */

// TODO compare output with file contents (.expect_output = FILE_CONTENTS("some_file.txt") ?) 

