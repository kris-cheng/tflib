#include "tflib.h"
#include "tf_system.h"
#include <fcntl.h> /* open O_RDWR */

void create_pid_file(const char *filename) {
    /* Try to write the pid file in a best-effort way. */
    FILE *fp = fopen(filename,"w");
    if (fp) {
        fprintf(fp,"%d\n",(int)getpid());
        fclose(fp);
    }
}

void daemonize(void) {
    int fd;

    if (fork() != 0) exit(0); /* parent exits */
    setsid(); /* create a new session */

    /* Every output goes to /dev/null. If Redis is daemonized but
     * the 'logfile' is set to 'stdout' in the configuration file
     * it will not log at all. */
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
    }
}

#ifdef _GNU_SOURCE
#include <sched.h>
void setaffinity(unsigned int cpu)
{
    cpu_set_t    mask;
    unsigned int i;

    CPU_ZERO(&mask);
    i = 0;
    do {
        if (cpu & 1) {
            CPU_SET(i, &mask);
        }
        i++;
        cpu >>= 1;
    } while (cpu);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
        error(0, "sched_setaffinity() failed");
    }
}
#endif
