#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

static void usage(const char *a){
    fprintf(stderr,"Usage: %s <pid>\n",a); 
    exit(1);
}

static int isnum(const char*s){
    for(;*s;s++)
        if(!isdigit(*s)) 
            return 0; 
    return 1;
}

int main(int c, char **v) {
    if (c != 2 || !isnum(v[1]))
        usage(v[0]);

    int pid = atoi(v[1]);
    char path[256];
    FILE *f;

    
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    f = fopen(path, "r");
    if (!f) {
        perror("Error opening stat file");
        return 1;
    }

    int read_pid, ppid;
    char comm[256], state;
    unsigned long utime, stime;
    
    
    fscanf(f, "%d %s %c %d", &read_pid, comm, &state, &ppid);
    
    for (int i = 0; i < 9; i++) {
        unsigned long dummy;
        fscanf(f, "%lu", &dummy);
    }
    
    fscanf(f, "%lu %lu", &utime, &stime);
    fclose(f);

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f) {
        perror("Error opening status file");
        return 1;
    }

    long vmrss = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &vmrss);
            break;
        }
    }
    fclose(f);

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    f = fopen(path, "r");
    if (!f) {
        perror("Error opening cmdline file");
        return 1;
    }

    char cmdline[1024] = {0};
    size_t len = fread(cmdline, 1, sizeof(cmdline) - 1, f);
    fclose(f);

    for (size_t i = 0; i < len; i++) {
        if (cmdline[i] == '\0')
            cmdline[i] = ' ';
    }

    if (len > 0 && cmdline[len - 1] == ' ')
        cmdline[len - 1] = '\0';

    long clk_tck = sysconf(_SC_CLK_TCK);
    double cpu_time = (double)(utime + stime) / clk_tck;

    printf("Process ID: %d\n", pid);
    printf("Process State: %c\n", state);
    printf("Parent PID: %d\n", ppid);
    printf("Command Line: %s\n", cmdline[0] ? cmdline : "(none)");
    printf("CPU Time: %.2f seconds\n", cpu_time);
    printf("Resident Memory: %ld kB\n", vmrss);

    return 0;
}
