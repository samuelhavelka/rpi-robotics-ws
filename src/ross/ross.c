#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <lgpio.h>

#include "gps.h"
#include "motor.h"

/*
Test the interaction of Motor and GPS systems:
    - Run motor until robot position reached.
    - Define x and y position converted from lat/long
    - Define movement in terms of forward, left, right
        * Spin in place 90deg
        * Navigational adjustment turns
*/

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



void *test_func(void *arg) {
    /* Wait for stop condition to terminate thread */
    termination_struct *ts = arg;

    // unpack arguments
    int *serial_handle = ts->serial_handle;
    pthread_t *p0 = ts->p0;
    pthread_t *p1 = ts->p1;

    gnrmc_t measurement;
    measurement = get_test_position(*serial_handle);
    
    while(1){
        measurement = iterate_m(measurement);

        printf("GNRMC: Time=%f   Valid=%d   LAT=%f %c   LONG=%f %c   Qual=%f \n",
                measurement.time, measurement.valid,
                measurement.latitude,measurement.lat_dir,
                measurement.longitude,measurement.long_dir,
                measurement.quality);

        if (measurement.latitude > 3505){
            pthread_cancel(*p0);
            pthread_cancel(*p1);

            printf("End test func and motor threads\n");
            pthread_cancel(pthread_self()); 
        }
        lguSleep(0.2);
    }
}

void forward_until(int PIN1, int PIN2) {

    int serial_handle;
    serial_handle = initialize_uart("/dev/ttyAMA0", 115200);


    motor_struct md1;   // Define motor specs
    md1.PIN = PIN2;
    md1.speed = 0.2;
    md1.duration = 10;

    motor_struct md2;
    md2.PIN = PIN1;
    md2.speed = 0.2;
    md2.duration = 10;

    int num_threads = 2;
    pthread_t *p1[num_threads];

    p1[0] = lgThreadStart(run_motor_thread, &md1);  // DO THESE START MOTOR? 
    p1[1] = lgThreadStart(run_motor_thread, &md2);  // CHANGE TO pthread_create?


    termination_struct ts;   // Define stopping condition
    ts.p0 = p1[0];
    ts.p1 = p1[1];
    ts.serial_handle = &serial_handle;

    pthread_t tid;
    pthread_create(&tid, NULL, test_func, &ts);

    printf("pthread_join\n");
    // wait for threads to end
    for (int i = 0; i < num_threads; i++) {
        pthread_join(*p1[i], NULL);
    }

    lgSerialClose(serial_handle);
}


int main()
{
    printf("Running Ross test...\n\n");

    forward_until(5, 6);

    printf("done\n");
    return 0;
}