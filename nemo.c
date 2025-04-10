// nemo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#define MAX_NOTES 100
#define NOTES_DIR "/.nemo_data/"

typedef struct {
    char message[256];
    char time_str[16];
    int interval_minutes;
    time_t last_triggered;
} Note;

char *get_storage_path(char *buffer, size_t size) {
    char real[PATH_MAX];
    if (!realpath(".", real)) {
        perror("realpath failed");
        exit(EXIT_FAILURE);
    }

    char safe_name[PATH_MAX] = {0};
    for (int i = 0; real[i] != '\0'; i++) {
        safe_name[i] = (real[i] == '/') ? '_' : real[i];
    }

    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not find HOME environment variable.\n");
        exit(EXIT_FAILURE);
    }

    snprintf(buffer, size, "%s%s%s.notes", home, NOTES_DIR, safe_name);
    return buffer;
}

void ensure_notes_dir() {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s%s", getenv("HOME"), NOTES_DIR);
    mkdir(path, 0755);
}

int load_notes(Note notes[]) {
    char path[PATH_MAX];
    get_storage_path(path, sizeof(path));

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    int count = 0;
    while (fscanf(fp, "%255[^|]|%15[^|]|%d|%ld\n",
                  notes[count].message,
                  notes[count].time_str,
                  &notes[count].interval_minutes,
                  &notes[count].last_triggered) == 4) {
        count++;
    }
    fclose(fp);
    return count;
}

void save_notes(Note notes[], int count) {
    char path[PATH_MAX];
    get_storage_path(path, sizeof(path));

    FILE *fp = fopen(path, "w");
    if (!fp) return;
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%d|%ld\n",
                notes[i].message,
                notes[i].time_str,
                notes[i].interval_minutes,
                notes[i].last_triggered);
    }
    fclose(fp);
}

void list_notes(Note notes[], int count) {
    printf("\n📌 Notes for this directory:\n");
    for (int i = 0; i < count; i++) {
        printf("[%d] %s", i + 1, notes[i].message);
        if (strlen(notes[i].time_str) > 0) printf(" @ %s", notes[i].time_str);
        if (notes[i].interval_minutes > 0) printf(" every %d min", notes[i].interval_minutes);
        printf("\n");
    }
}

void delete_note(Note notes[], int *count, int index) {
    if (index < 1 || index > *count) return;
    for (int i = index - 1; i < *count - 1; i++) {
        notes[i] = notes[i + 1];
    }
    (*count)--;
    save_notes(notes, *count);
    printf("🗑️ Note %d deleted.\n", index);
}

void check_and_show_notes(Note notes[], int count) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    int shown = 0;
    for (int i = 0; i < count; i++) {
        int show = 0;

        if (strlen(notes[i].time_str) > 0) {
            int hh, mm;
            sscanf(notes[i].time_str, "%d:%d", &hh, &mm);
            if (tm_now->tm_hour == hh && tm_now->tm_min == mm) {
                show = 1;
            }
        }

        if (notes[i].interval_minutes > 0) {
            double elapsed = difftime(now, notes[i].last_triggered);
            if (elapsed >= notes[i].interval_minutes * 60) {
                show = 1;
            }
        }

        if (show) {
            printf("🔔 %s\n", notes[i].message);
            notes[i].last_triggered = now;
            shown = 1;
        }
    }

    if (shown) save_notes(notes, count);
}

int main(int argc, char *argv[]) {
    ensure_notes_dir();

    Note notes[MAX_NOTES];
    int count = load_notes(notes);

    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        list_notes(notes, count);
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "--del") == 0) {
        int index = atoi(argv[2]);
        delete_note(notes, &count, index);
        return 0;
    }

    if (argc >= 2) {
        Note new;
        strncpy(new.message, argv[1], sizeof(new.message));
        new.message[sizeof(new.message) - 1] = '\0';

        if (argc >= 3) strncpy(new.time_str, argv[2], sizeof(new.time_str));
        else new.time_str[0] = '\0';

        new.interval_minutes = (argc >= 4) ? atoi(argv[3]) : 0;
        new.last_triggered = 0;

        notes[count++] = new;
        save_notes(notes, count);
        printf("✅ Note saved for this directory.\n");
        return 0;
    }

    check_and_show_notes(notes, count);
    return 0;
}
