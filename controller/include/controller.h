#pragma once

struct axis_state
{
    short x, y;
};

int read_event(int fd, struct js_event *event);

size_t get_axis_state(struct js_event *event, struct axis_state axes[3]);

int controller_test(int argc, char *argv[]);