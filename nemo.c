// nemo.c — Reminder by delay or exact time using fork (non-blocking)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NOTES_DIR "/.nemo_data/"

void ensure_notes_dir(char *dir_path) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not find HOME environment variable.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(dir_path, PATH_MAX, "%s%s", home, NOTES_DIR);
    mkdir(dir_path, 0755);
}

void run_background_reminder(const char *message, int sleep_seconds) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        sleep(sleep_seconds);
        FILE *tty = fopen("/dev/tty", "w");
        if (tty) {
            fprintf(tty, "\n🔔 Reminder: %s\n", message);
            fclose(tty);
        }
        exit(0);
    }
    printf("✅ Reminder scheduled in %d seconds.\n", sleep_seconds);
}

int parse_time_to_delay(const char *time_str) {
    int hh, mm;
    if (sscanf(time_str, "%d:%d", &hh, &mm) != 2 || hh < 0 || hh > 23 || mm < 0 || mm > 59) {
        return -1;
    }
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);
    struct tm target = *now_tm;
    target.tm_hour = hh;
    target.tm_min = mm;
    target.tm_sec = 0;

    time_t target_time = mktime(&target);
    if (target_time <= now) {
        target_time += 86400;  // next day
    }
    return (int)(target_time - now);
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        const char *msg = argv[1];
        const char *arg2 = argv[2];

        int delay;
        if (strncmp(arg2, "at:", 3) == 0) {
            delay = parse_time_to_delay(arg2 + 3);
            if (delay < 0) {
                fprintf(stderr, "❌ Invalid time format. Use HH:MM with at: prefix.\n");
                return 1;
            }
        } else {
            delay = atoi(arg2);
            if (delay <= 0) {
                fprintf(stderr, "❌ Invalid delay: must be a positive integer or use at:HH:MM\n");
                return 1;
            }
            delay *= 60;  // convert minutes to seconds
        }

        run_background_reminder(msg, delay);
        return 0;
    }

    printf("Usage: nemo \"message\" <minutes|at:HH:MM>\n");
    return 1;
}
