#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <lgpio.h>

#include "gps.h"

void print_array(char array[], int length) {
    for (int i=0; i<length; i++) {
        printf("%c", array[i]);
    }
    printf("\n\n");
    // printf("\nExiting print_array\n");
}


gnrmc_t assign_property_gnrmc(gnrmc_t target ,char *token, int idx) {

    switch (idx)
    {
    case 1:
        target.time = atof(token);
        break;
    case 2:
        if (!strncmp(token,"A",1)) {target.valid=1;} else{target.valid=-1;};
        break;
    case 3:
        target.latitude = atof(token);
        break;
    case 4:
        target.lat_dir = *token;
        break;
    case 5:
        target.longitude = atof(token);
        break;
    case 6:
        target.long_dir = *token;
        break;
    case 7:
        target.quality = atof(token);
        break;
    default:
        break;
    }
    return target;
}


gnrmc_t parse_gnss(char str[256]) {

    const char outer_delimiters[] = "\r\n";
    const char inner_delimiters[] = ",";
 
    char* token;
    char* outer_saveptr = NULL;
    char* inner_saveptr = NULL;

    int token_count = 0;
    gnrmc_t measurement;
    measurement = (gnrmc_t) {-1,-1,-1,-1,-1,-1,-1};  // initialize measurement as all -1
    
    // split serial buffer into tokens using \r\n as delimiter
    token = strtok_r(str, outer_delimiters, &outer_saveptr);
 
    while (token != NULL) {
        // printf("\n\n_%s_\n", token);

        // split serial buffer into tokens using , as delimiter
        char* inner_token = strtok_r(token, inner_delimiters, &inner_saveptr);

        // only check the GNRMC formatted output from GPS
        if (!strncmp(inner_token, "$GNRMC", 7)) {

            while (inner_token != NULL) {
                // printf("_%s_\t", inner_token);
                measurement = assign_property_gnrmc(measurement, inner_token, token_count);
                inner_token = strtok_r(NULL, inner_delimiters, &inner_saveptr);
                token_count++;
            }
        }
        token = strtok_r(NULL, outer_delimiters, &outer_saveptr);
    }

    if (token_count<8) {fprintf(stderr, "DATA ERROR: gps.parse_gnss() : Missing token, data will be invalid.\n");};

    return measurement;
}

int initialize_uart(char port[], int baud_rate) {
    /* Open serial port with given baudrate. Returns handle (>=0). */

    int serial_handle;

    if ((serial_handle = lgSerialOpen(port, baud_rate, 0)) < 0) {	/* open serial port */
        fprintf (stderr, "ERROR: Unable to open serial device: %s\n", strerror (errno)) ;
        return -1;
    }

    return serial_handle;
}

gnrmc_t get_position(int serial_handle) {
    /*
    Input handle from initialize_uart() function
    Returns GNRMC struct measurement from UART device
    */

    int err_count=0;
    int byte_count;
    int max_len = 256;
    char buffer[256];
    gnrmc_t measurement;

    while(1){

        if(lgSerialDataAvailable(serial_handle)) { 
            byte_count = lgSerialRead(serial_handle, buffer, max_len);	/* receive character serially*/	

            if (byte_count>230) {   // required number of characters to get full GNRMC 
                // print_array(buffer,max_len);     // print out serial buffer for debugging 
                measurement = parse_gnss(buffer);
                if (err_count>2) {fprintf(stderr, "WARNING: gps.request_position() : (%d) GPS serial byte count less than minimum.\n", err_count);}
                break;
            } 
            else {err_count++;}
        }  
    } 
    // printf("Error count: %d\n", err_count);     // print out number of truncated readings 
    return measurement;
}

gnrmc_t get_position_ma(int serial_handle, int n) {
    /*
    Returns average position of n measurements
    Takes approx 0.1 sec per measurement.
    [Potentially increase buffer size for better performance]
    */

    int i=1;
    gnrmc_t m_temp;
    gnrmc_t measurement;

    measurement = get_position(serial_handle);

    while (i<n) {

        m_temp = get_position(serial_handle);

        if (m_temp.valid) {
            measurement.latitude += m_temp.latitude;
            measurement.longitude += m_temp.longitude;

            i++;
        }
    }
    measurement.latitude /= n;
    measurement.longitude /= n;

    return measurement;
}

int main_gps()
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