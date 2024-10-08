/**
Based on sample code from Jason White

Description:
Reads joystick/gamepad events and displays them.

Compile:
gcc controller.c -o controller.o

Run:
./controller.o /dev/input/js1

See also:
https://www.kernel.org/doc/Documentation/input/joystick-api.txt
*/

#include <fcntl.h> // Contains file controls like O_RDWR
#include <stdio.h>
#include <unistd.h> // write(), read(), close()
#include <stdint.h> // for uint8_t

#include <linux/joystick.h>

#include <errno.h> // Error integer and strerror() function
// #include <stdlib.h>
#include <string.h>

#include "controller.h"

/**
 * Reads a joystick event from the joystick device.
 *
 * Returns 0 on success. Otherwise -1 is returned.
 */
int read_event(int fd, struct js_event *event)
{
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    /* Error, could not read full event. */
    return -1;
}

/**
 * Returns the number of axes on the controller or 0 if an error occurs.
 */
size_t get_axis_count(int fd)
{
    uint8_t axes;

    if (ioctl(fd, JSIOCGAXES, &axes) == -1)
        return 0;

    return axes;
}

/**
 * Returns the number of buttons on the controller or 0 if an error occurs.
 */
size_t get_button_count(int fd)
{
    uint8_t buttons;
    if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1)
        return 0;

    return buttons;
}

/**
 * Keeps track of the current axis state.
 *
 * NOTE: This function assumes that axes are numbered starting from 0, and that
 * the X axis is an even number, and the Y axis is an odd number. However, this
 * is usually a safe assumption.
 *
 * Returns the axis that the event indicated.
 */
size_t get_axis_state(struct js_event *event, struct axis_state axes[3])
{
    size_t axis = event->number / 2;

    if (axis < 3)
    {
        if (event->number % 2 == 0)
            axes[axis].x = event->value;
        else
            axes[axis].y = event->value;
    }

    return axis;
}

int controller_test(int argc, char *argv[])
{
    printf("Running controller test...\n");

    const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;

    if (argc > 1)
        device = argv[1];
    else
        device = "/dev/input/js0";

    // open connection to input device
    js = open(device, O_RDONLY);

    if (js < 0)
    {
        fprintf(stderr, "Could not open device '%s' : %s\n", device, strerror(errno));
        return 1;
    }

    /* This loop will exit if the controller is unplugged. */
    while (read_event(js, &event) == 0)
    {
        // Exit loop when select button is pressed
        if ((event.type == JS_EVENT_BUTTON) & (event.number == 6))
        {
            printf("Loop terminated by user.\n");
            break;
        }

        // Handle button, axis, and init events
        switch (event.type)
        {
        case JS_EVENT_BUTTON:
            // On button press, print button id and if it was pressed or released
            printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
            /*
             *  ID  Name
             *  0   A
             *  1   B
             *  2   X
             *  3   Y
             *  4   LB
             *  5   RB
             *  6   Select
             *  7   Start
             */
            break;

        case JS_EVENT_AXIS:
            // On use joystick, print out X and Y values
            axis = get_axis_state(&event, axes);
            printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
            break;

        default:
            // Ignore init events
            break;
        }

        fflush(stdout);
    }

    close(js);
    return 0;
}