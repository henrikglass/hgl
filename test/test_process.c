
#define HGL_PROCESS_IMPLEMENTATION
#include "hgl_process.h"

int main()
{
    char buf[4096];
    size_t n_read = 0;
    (void) n_read;

    /* ========================== example 1 ======================== */

    int return_code = hgl_process_run_sync("cowsay", "-e", "><", "Hello I'm a cow");

    printf("returned with exit code %d\n", return_code);

    //printf("%s\n", buf);
    //memset(buf, 0, n_read);

    /* ========================== example 2 ======================== */

    HglProcess p2 = hgl_process_make("ls", "hgl_da.h", "hgl_cmd.h", "hgl_process.h");
    hgl_process_append_args(&p2, "-a");
    hgl_process_append_args(&p2, "-l", "-h");
    hgl_process_redir_output_to_stdout(&p2);
    printf("1:\n");
    hgl_process_spawn(&p2);
    hgl_process_wait(&p2);
    printf("2:\n");
    hgl_process_spawn(&p2);
    hgl_process_wait(&p2);
    printf("3:\n");
    hgl_process_spawn(&p2);
    hgl_process_wait(&p2);

    //n_read = read(p2.output, buf, 4096);
    //printf("%s\n", buf);
    //memset(buf, 0, n_read);

    /* ========================== example 3 ======================== */

    HglProcess ps[] = {
        hgl_process_make("echo", "en\n"
                                 "ball\n"
                                 "groda\n"
                                 "groda\n"
                                 "groda\n"
                                 "dansar\n"
                                 "aldrig\n"
                                 "Ensam\n"),
        hgl_process_make("uniq"),
        hgl_process_make("sort"),
        hgl_process_make("cowsay", "-e", "^^"),
    };
    size_t n_ps = sizeof(ps)/sizeof(ps[0]);

    
    /* run #1 */
    hgl_process_chain(ps, n_ps);
    hgl_process_spawn_n(ps, n_ps);
    n_read = read(ps[n_ps - 1].output, buf, 4096);
    printf("%s\n", buf);
    hgl_process_wait_n(ps, n_ps);
    memset(buf, 0, n_read);

    /* run #2 */
    ps[0].argv[1] = "The\nquick\nbrown\nfox\njumped\nover\nthe\nlazy\ndog.";
    ps[3].argv[2] = "--";
    hgl_process_repipe_n(ps, n_ps);
    hgl_process_chain(ps, n_ps);
    hgl_process_spawn_n(ps, n_ps);
    n_read = read(ps[n_ps - 1].output, buf, 4096);
    printf("%s\n", buf);
    hgl_process_wait_n(ps, n_ps);

    hgl_process_destroy_n(ps, n_ps);

    return 0;
}
