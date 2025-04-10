// nemo.c — Reminder by delay or exact time with optional repeating notices (rm:X) and cancellation
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
#include <signal.h>

#define NOTES_DIR "/.nemo_data/"
#define PID_FILE "/tmp/nemo_reminder.pid"

void ensure_notes_dir(char *dir_path) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not find HOME environment variable.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(dir_path, PATH_MAX, "%s%s", home, NOTES_DIR);
    mkdir(dir_path, 0755);
}

void save_pid(pid_t pid) {
    FILE *fp = fopen(PID_FILE, "w");
    if (fp) {
        fprintf(fp, "%d", pid);
        fclose(fp);
    }
}

void clear_pid_file() {
    unlink(PID_FILE);
}

void run_background_reminder(const char *message, int total_delay, int repeat_interval) {
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

        int remaining = total_delay;
        while (repeat_interval > 0 && remaining > repeat_interval) {
            sleep(repeat_interval);
            remaining -= repeat_interval;
            FILE *tty = fopen("/dev/tty", "w");
            if (tty) {
                fprintf(tty, "\n⏳ %s — %d minutes remaining\n", message, remaining / 60);
                fclose(tty);
            }
        }

        sleep(remaining);
        FILE *tty = fopen("/dev/tty", "w");
        if (tty) {
            fprintf(tty, "\n🔔 Reminder: %s\n", message);
            fclose(tty);
        }
        clear_pid_file();
        exit(0);
    }
    save_pid(pid);
    printf("✅ Reminder scheduled in %d minute(s)%s\n",
           total_delay / 60,
           repeat_interval > 0 ? " with repeating notices" : "");
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

void cancel_reminder() {
    FILE *fp = fopen(PID_FILE, "r");
    if (!fp) {
        printf("❌ No active reminder found.\n");
        return;
    }
    pid_t pid;
    if (fscanf(fp, "%d", &pid) == 1) {
        if (kill(pid, SIGTERM) == 0) {
            printf("🧹 Reminder canceled successfully.\n");
        } else {
            perror("Failed to cancel reminder");
        }
    }
    fclose(fp);
    clear_pid_file();
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "done") == 0) {
        cancel_reminder();
        return 0;
    }

    if (argc >= 3) {
        const char *msg = argv[1];
        const char *arg2 = argv[2];
        int delay = 0;
        int repeat = 0;

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
            delay *= 60; // minutes to seconds
        }

        if (argc == 4 && strncmp(argv[3], "rm:", 3) == 0) {
            repeat = atoi(argv[3] + 3);
            if (repeat <= 0) {
                fprintf(stderr, "❌ Invalid repeat interval in rm:X. Must be > 0\n");
                return 1;
            }
            repeat *= 60;
        }

        run_background_reminder(msg, delay, repeat);
        return 0;
    }

    printf("Usage: nemo \"message\" <minutes|at:HH:MM> [rm:X]\n       nemo done   # cancel running reminder\n");
    return 1;
}