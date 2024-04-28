#include <stdio.h>
#include <stdlib.h>
#include <lgpio.h>
#include "gps.h"


double random_01() {
    /* Return random from 0 to 1 */
    return (double)rand() / (double)RAND_MAX;
}

gnrmc_t get_test_position(int serial_handle) {
    /*
    Input handle from initialize_uart() function
    Returns FAKED GNRMC struct measurement from UART device
    */
    gnrmc_t m;

    m.time = 1;        // time of measurement
    m.valid = 1;          // is data valid?

    m.latitude = 3500;    // DDmm.mm (7 decimal places)
    m.lat_dir = *"N";       // EW

    m.longitude = 8950;   // DDDmm.mm (7 decimal places)
    m.long_dir = *"W";      // NS

    m.quality = 0.5;     // quality metric of measurement

    return m;
}

gnrmc_t iterate_m(gnrmc_t m) {
    m.time += 1;        // time of measurement

    double r_lat = 1 + random_01() / 100;
    double r_long = 1 + random_01() / 100;

    m.latitude += r_lat;    // DDmm.mm (7 decimal places)
    m.longitude += r_long;   // DDDmm.mm (7 decimal places)

    return m;
}


int main()
{
    printf("Running GPS test...\n\n");

    int serial_handle;
    gnrmc_t measurement;

    serial_handle = initialize_uart("/dev/ttyAMA0", 115200);
    // measurement = get_test_position(serial_handle);

    while(1) {
        clock_t tic = clock();

        measurement = get_position(serial_handle);
        // measurement = iterate_m(measurement);

        printf("Time get_position_ma elapsed in sec: %f\n", (double)((clock_t)clock() - tic) / CLOCKS_PER_SEC);

        printf("GNRMC: Time=%f   Valid=%d   LAT=%f %c   LONG=%f %c   Qual=%f \n",
                measurement.time, measurement.valid,
                measurement.latitude,measurement.lat_dir,
                measurement.longitude,measurement.long_dir,
                measurement.quality);
        break;
        if (measurement.time == 10) {break;}

        lguSleep(0.25);
    };

    lgSerialClose(serial_handle);
    printf("\nDone\n");
    return 0;
}