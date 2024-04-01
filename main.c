#include <stdio.h>

#include <lgpio.h>

#include "gps.h"
// #include "motor.h"

int main()
{
    printf("Running main...\n\n");

    int serial_handle;

    gnrmc_t measurement;

    serial_handle = initialize_uart("/dev/ttyAMA0", 115200);

    while(1) {
        measurement = get_position_ma(serial_handle, 3);

        printf("GNRMC: %f %d %f %c %f %c %f \n",measurement.time, measurement.valid,
                                                measurement.latitude,measurement.lat_dir,
                                                measurement.longitude,measurement.long_dir,
                                                measurement.quality);
        break;
    };

    lgSerialClose(serial_handle);
    return 0;
}