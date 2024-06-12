#ifndef HGL_PROCESS_H
#define HGL_PROCESS_H

#include <sys/types.h>

#define HGL_PROCESS_MAX_N_ARGS 127

#define HGL_PROCESS_PIPE_IN        0
#define HGL_PROCESS_PIPE_OUT       1
#define HGL_PROCESS_PIPE_READ_END  0
#define HGL_PROCESS_PIPE_WRITE_END 1

typedef struct
{
    pid_t pid;
    union {
        int pipes[2][2];
        struct {
            int input_read_end;
            int input;
            int output;
            int output_write_end;
        };
    };
    char *argv[HGL_PROCESS_MAX_N_ARGS + 1];
} HglProcess;


#ifndef HGL_PROCESS_ISOC
#define hgl_process_run_async(...)                              \
    ({                                                          \
        HglProcess p_ = hgl_process_prepare(__VA_ARGS__, NULL); \
        hgl_process_spawn(&p_);                                 \
        p_;                                                     \
    })

#define hgl_process_run_sync(...)                               \
    ({                                                          \
        HglProcess p_ = hgl_process_prepare(__VA_ARGS__, NULL); \
        hgl_process_spawn(&p_);                                 \
        int status_ = hgl_process_wait(&p_);                    \
        hgl_process_destroy(&p_);                               \
        status_;                                                \
    })
#endif

#define hgl_process_prepare(...) (hgl_process_prepare_(__VA_ARGS__, NULL))
HglProcess hgl_process_prepare_(const char *exe, ...);

void hgl_process_chain(HglProcess *ps, size_t n);

void hgl_process_spawn(HglProcess *p);
void hgl_process_spawn_n(HglProcess *ps, size_t n);

int hgl_process_wait(HglProcess *p);
int hgl_process_wait_n(HglProcess *ps, size_t n);

void hgl_process_signal(HglProcess *p, int signal);
void hgl_process_signal_n(HglProcess *ps, size_t n, int signal);

void hgl_process_destroy(HglProcess *p);
void hgl_process_destroy_n(HglProcess *ps, size_t n);

#endif /* HGL_PROCESS_H */

#ifdef HGL_PROCESS_IMPLEMENTATION

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

HglProcess hgl_process_prepare_(const char *exe, ...)
{
    int err = 0;

    HglProcess process = {
        .pid   = -1,
        .pipes = {{-1, -1}, {-1,-1}},
        .argv  = {NULL},
    };

    err = pipe(process.pipes[HGL_PROCESS_PIPE_IN]);
    if (err != 0) {
        fprintf(stderr, "[hgl_process] Error: Failed to create input pipe. errno=%s\n",
                strerror(errno));
        return process;
    }

    err = pipe(process.pipes[HGL_PROCESS_PIPE_OUT]);
    if (err != 0) {
        fprintf(stderr, "[hgl_process] Error: Failed to create output pipe. errno=%s\n",
                strerror(errno));
        close(process.pipes[HGL_PROCESS_PIPE_IN][HGL_PROCESS_PIPE_READ_END]);
        close(process.pipes[HGL_PROCESS_PIPE_IN][HGL_PROCESS_PIPE_WRITE_END]);
        return process;
    }

    /* populate arguments */
    va_list args;
    va_start(args, exe);
    char *next_arg;
    size_t idx = 1;
    process.argv[0] = (char *) exe;
    while (((next_arg = va_arg(args, char *)) != NULL) &&
           (idx < HGL_PROCESS_MAX_N_ARGS)) {
        process.argv[idx++] = next_arg;
    }
    process.argv[idx] = NULL;
    va_end(args);

    return process;
}

void hgl_process_chain(HglProcess *ps, size_t n)
{
    for (size_t i = 0; i < n - 1; i++) {
        /* close old pipes */
        close(ps[i].output_write_end);
        close(ps[i].output);
        close(ps[i + 1].input_read_end);
        close(ps[i + 1].input);
        ps[i].output = -1;
        ps[i].output_write_end = -1;
        ps[i + 1].input = -1;
        ps[i + 1].input_read_end = -1;

        /* make a new pipe */
        int new_pipe[2];
        pipe(new_pipe);
        ps[i].output_write_end = new_pipe[1];
        ps[i + 1].input_read_end = new_pipe[0];
    }
}

void hgl_process_spawn(HglProcess *p)
{
    assert(p->argv[0] != NULL);
    assert(p->pipes[0][0] != -1);
    //assert(p->pipes[0][1] != -1);
    //assert(p->pipes[1][0] != -1);
    assert(p->pipes[1][1] != -1);

    p->pid = fork();

    if (p->pid == 0 /* we are the child*/ ) {
        /* close unused ends of pipes */
        close(p->pipes[HGL_PROCESS_PIPE_IN][HGL_PROCESS_PIPE_WRITE_END]);
        close(p->pipes[HGL_PROCESS_PIPE_OUT][HGL_PROCESS_PIPE_READ_END]);

        /* replace stdin and stdout with used ends of pipes */
        dup2(p->pipes[HGL_PROCESS_PIPE_IN][HGL_PROCESS_PIPE_READ_END], STDIN_FILENO);
        dup2(p->pipes[HGL_PROCESS_PIPE_OUT][HGL_PROCESS_PIPE_WRITE_END], STDOUT_FILENO);

        /* jump execution to new process image */
        if (execvp(p->argv[0], p->argv) == -1) {
            fprintf(stderr, "[hgl_process]: Error (in child): failed to execute. errno=%s\n",
                    strerror(errno));
            exit(1);
        }

        /* execvp will never return on success */
        assert(0 && "Unreachable");
    }

    /* close unused ends of pipes */
    close(p->pipes[HGL_PROCESS_PIPE_IN][HGL_PROCESS_PIPE_READ_END]);
    close(p->pipes[HGL_PROCESS_PIPE_OUT][HGL_PROCESS_PIPE_WRITE_END]);
    p->pipes[HGL_PROCESS_PIPE_IN][HGL_PROCESS_PIPE_READ_END] = -1;
    p->pipes[HGL_PROCESS_PIPE_OUT][HGL_PROCESS_PIPE_WRITE_END] = -1;
}

void hgl_process_spawn_n(HglProcess *ps, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        hgl_process_spawn(&ps[i]);
    }
}


int hgl_process_wait(HglProcess *p)
{
    assert(p->pid != -1);

    pid_t wait_pid;
    int wstatus = 0;
    while ((wait_pid = waitpid(p->pid, &wstatus, 0)) != p->pid) {
        if (wait_pid == -1) {
            fprintf(stderr, "[hgl_process] Error: waitpid returned an error. errno=%s\n",
                    strerror(errno));
            return -1;
        }
    }

    p->pid = -1;

    return WEXITSTATUS(wstatus);
}

int hgl_process_wait_n(HglProcess *ps, size_t n)
{
    int exit_status = 0;
    for (size_t i = 0; i < n; i++) {
        exit_status |= hgl_process_wait(&ps[i]);
    }
    return exit_status;
}

void hgl_process_signal(HglProcess *p, int signal)
{
    assert(p->pid != -1);
    kill(p->pid, signal);
}

void hgl_process_signal_n(HglProcess *ps, size_t n, int signal)
{
    for (size_t i = 0; i < n; i++) {
        hgl_process_signal(&ps[i], signal);
    }
}

void hgl_process_destroy(HglProcess *p)
{
    if (p->pid != -1) {
        kill(p->pid, SIGKILL);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (p->pipes[i][j] != -1) {
                close(p->pipes[i][j]);
                p->pipes[i][j] = -1;
            }
        }
    }

}

void hgl_process_destroy_n(HglProcess *ps, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        hgl_process_destroy(&ps[i]);
    }
}

#endif /* HGL_PROCESS_IMPLEMENTATION */

