#ifndef DISPLAY_H
#define DISPLAY_H

#include "time.h"

typedef struct status_line {
    char* text;
    uint16_t len;
    time_t from;
    time_t until;
    struct status_line* next;
} status_line;

void start_display();
void stop_display();
void showstatus(const char* text, unsigned short duration);

#endif /* DISPLAY_H */
