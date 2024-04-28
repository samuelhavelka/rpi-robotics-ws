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

// // define macro for default buffer size
// #define GET_MACRO_get_position(_1,_2,get_position2,...) get_position2
// #define get_position(...) GET_MACRO_get_position(__VA_ARGS__, get_position2,get_position1)(__VA_ARGS__)
// #define get_position1(a) get_position_func(a)

gnrmc_t get_position(int serial_handle);

gnrmc_t get_position_ma(int serial_handle, int n);
