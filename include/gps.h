#pragma once

void print_array(char array[], int length);

// GPS output reading (GNRMC Format)
typedef struct gnrmc_s {
    double time;        // time of measurement
    int valid;          // is data valid?

    double latitude;    // DDmm.mm (7 decimal places)
    char lat_dir;       // EW

    double longitude;   // DDDmm.mm (7 decimal places)
    char long_dir;      // NS

    double quality;     // quality metric of measurement
} gnrmc_t;


gnrmc_t assign_property_gnrmc(gnrmc_t target, char *token, int idx);
gnrmc_t parse_gnss(char str[256]);

int initialize_uart(char port[], int baud_rate);

gnrmc_t get_position(int serial_handle);
gnrmc_t get_position_ma(int serial_handle, int n);