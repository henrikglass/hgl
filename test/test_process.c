
#define HGL_PROCESS_IMPLEMENTATION
#include "hgl_process.h"

int main()
{
    HglProcess p = hgl_process_run_async("cowsay", "-e", "><", "Hello I'm a cow");
    char buf[4096];
    ssize_t n_read = read(p.output, buf, 4096);
    hgl_process_wait(&p);

    printf("%s\n", buf);
    memset(buf, 0, n_read);

    HglProcess ps[] = {
        hgl_process_prepare("echo", "en\n"
                                    "ball\n"
                                    "groda\n"
                                    "groda\n"
                                    "groda\n"
                                    "dansar\n"
                                    "aldrig\n"
                                    "Ensam\n"),
        hgl_process_prepare("uniq"),
        hgl_process_prepare("sort"),
        hgl_process_prepare("cowsay", "-e", "^^"),
    };
    size_t n_ps = sizeof(ps)/sizeof(ps[0]);

    hgl_process_chain(ps, n_ps);
    hgl_process_spawn_n(ps, n_ps);

    n_read = read(ps[n_ps - 1].output, buf, 4096);
    printf("%s\n", buf);

    hgl_process_wait_n(ps, n_ps);
    hgl_process_destroy_n(ps, n_ps);

    return 0;
}
