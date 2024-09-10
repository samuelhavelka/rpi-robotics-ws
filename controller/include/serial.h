#pragma once

typedef struct byteString
{
    int N;
    unsigned char *ptr;
} byteString;

int init_serial_port(const char *device);
void waitForArduino(int serial_port);

int sendRec(int serial_port, byteString data);
int serial_test();
