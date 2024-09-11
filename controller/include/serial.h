#pragma once

// struct to hold an array of bytes
typedef struct byteString
{
    int N;              // length of array
    unsigned char *ptr; // array of bytes [0-255]
} byteString;

int init_serial_port(const char *device); // Opens serial port and wait for setup confirmation

int sendRec(int serial_port, byteString data); // Sends data to Arduino and confirms response

int serial_test();
