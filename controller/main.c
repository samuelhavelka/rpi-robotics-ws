#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <math.h> // for round function

#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
// #include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <linux/joystick.h>

#include "controller.h"
#include "serial.h"

int main()
{
    // open conection to Arduino
    int serial_port = init_serial_port("/dev/ttyUSB0");
    if (serial_port < 0)
    {
        printf("[init_serial_port]: Error %i from init_serial_port(): %s\n", errno, strerror(errno));
        return 1;
    }

    waitForArduino(serial_port);

    // ============================

    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;

    // open connection to input device
    js = open("/dev/input/js1", O_RDONLY);

    if (js < 0)
    {
        fprintf(stderr, "Could not open device '%s' : %s\n", "/dev/input/js0", strerror(errno));
        return 1;
    }

    // ============================

    byteString data;
    data.N = 2;
    data.ptr = calloc(data.N, sizeof(unsigned char));

    data.ptr[0] = 187;
    data.ptr[1] = 187;

    // ============================

    /* This loop will exit if the controller is unplugged. */
    while (read_event(js, &event) == 0)
    {
        // Handle button, axis, and init events
        switch (event.type)
        {
        case JS_EVENT_BUTTON:
            // On button press, print button id and if it was pressed or released
            printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");

            // Exit loop when select button is pressed
            if (event.number == 6)
            {
                printf("Loop terminated by user.\n");
                break;
            }

            if (event.value)
            {
                switch (event.number)
                {
                case 0:
                    break;
                case 1:
                    break;
                default:
                }
            }
            break;

        case JS_EVENT_AXIS:
            // On use joystick, print out X and Y values
            axis = get_axis_state(&event, axes);
            printf("Axis %zu at (%d, %d)\n", axis, axes[axis].x, axes[axis].y);

            if (axis == 0)
            {
                double foo = (double)axes[axis].y;
                foo = foo / (double)32767;
                foo = foo * (double)6;
                foo = -foo + (double)187;

                unsigned char value = (unsigned char)(foo + 0.5); // scale value

                data.ptr[0] = value;
                data.ptr[1] = value;

                printf("value=%lf   %d\n", foo, value);
                sendRec(serial_port, data);
            }

            break;

        default:
            // Ignore init events
            break;
        }

        fflush(stdout);
    }

    close(js);
    close(serial_port);

    free(data.ptr);

    return 0;
}