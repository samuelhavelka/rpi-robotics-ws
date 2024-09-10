// C library headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <time.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include <sys/ioctl.h> /* Serial Port IO Controls */

// =========================

int DEBUG = 1;

// Define special character encoding scheme
int startByte = 254;
int endByte = 255;
int specialByte = 253;

// define max message length
int maxLen = 16;

// =========================

// struct to hold an array of bytes
typedef struct byteString
{
    int N;              // length of array
    unsigned char *ptr; // array of bytes [0-255]
} byteString;

int init_serial_port(const char *device)
{
    // open connection to serial port
    int serial_port = open(device, O_RDWR | O_NOCTTY);

    if (serial_port < 0)
    {
        fprintf(stderr, "[init_serial_port]: Could not open serial device '%s' : %s\n", device, strerror(errno));
        return -1;
    }

    // set port termios settings
    struct termios tty;

    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("[init_serial_port]: Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;  // Clear all bits that set the data size
    tty.c_cflag |= CS8;     // 8 bits per byte (most common)
    // tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    // tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    // tty.c_cc[VMIN] = 0;

    // Set Baud Rate
    cfsetispeed(&tty, B57600);
    cfsetospeed(&tty, B57600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("[init_serial_port]: Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    printf("Arduino %d Init Complete.\n", serial_port);

    return serial_port;
}

void debugPrintArr(byteString input, const char *note)
{
    printf("DEBUG: %s : ", note);
    for (int i = 0; i < input.N; i++)
    {
        printf("%d ", input.ptr[i]);
    }
    printf("\n");
}

void debugPrintInt(int x)
{
    printf("DEBUG: %d\n", x);
}

byteString copyArrayToByteString(byteString data, unsigned char new_data[])
{
    // Copies an unsigned char array into a ByteString data struct
    // length of both arrays must be set by caller.

    for (int i = 0; i < data.N; i++)
    {
        data.ptr[i] = new_data[i];
    }

    return data;
}

void waitForArduino(int serial_port)
{
    // Wait for Arduino to initialize and receive 'Ready' message.

    char read_buf[32];
    int n;
    char *p = NULL;

    while (!p)
    {
        // Set read buffer to all zeros to simplify printf() call
        memset(&read_buf, '\0', sizeof(read_buf));

        n = read(serial_port, &read_buf, sizeof(read_buf));
        // n is the number of bytes read, and can also be -1 to signal an error.
        if (n < 0)
        {
            printf("Error reading: %s\n", strerror(errno));
        }

        p = strstr(read_buf, "Ready");

        usleep(0.1 * 1000000); // sleep for 0.1 sec
    }

    printf("Arduino %d is Ready.\n\n", serial_port);
}

byteString decodeSpecialBytes(byteString data)
{
    // Decodes all special characters and removes start/end characters.

    unsigned char outString[data.N];
    int n = 0;

    if (DEBUG)
        debugPrintArr(data, "  <--  Received ");

    // loop through input array to decode Special Bytes
    // Skips first byte
    for (int i = 1; i < data.N; i++)
    {
        if (data.ptr[i] == specialByte)
        {
            i++;
            outString[n] = specialByte + data.ptr[i];
        }
        else
        {
            outString[n] = data.ptr[i];
        }

        if (data.ptr[i] == endByte) // break when endByte is reached (non-inclusive)
            break;

        n++;
    }

    // re-allocate array of correct length with decoded data
    data.ptr = realloc(data.ptr, n * sizeof(unsigned char));
    data.N = n;

    // copy data from temp output string to data struct
    data = copyArrayToByteString(data, outString);

    if (DEBUG)
        debugPrintArr(data, "Decode Processed");

    return data;
}

byteString encodeSpecialBytes(byteString data)
{
    // Encodes all special characters and adds start/end characters.

    unsigned char outString[maxLen];
    int n = 1;

    if (DEBUG)
        debugPrintArr(data, " -->   Received ");

    outString[0] = startByte;

    // loop through input array to encode Special Bytes
    for (int i = 0; i < data.N; i++)
    {
        if (data.ptr[i] >= specialByte)
        {
            outString[n] = specialByte;
            n++;
            outString[n] = data.ptr[i] - specialByte;
        }
        else
        {
            outString[n] = data.ptr[i];
        }
        n++;
    }
    outString[n] = endByte;
    n++;

    // re-allocate array of correct length with encoded data
    data.ptr = realloc(data.ptr, n * sizeof(unsigned char));
    data.N = n;

    // copy data from temp output string to data struct
    data = copyArrayToByteString(data, outString);

    if (DEBUG)
        debugPrintArr(data, "Encode Processed");

    return data;
}

byteString receiveBytes(int serial_port, byteString data)
{
    // Reads message from Arduino over serial comms.
    // Decodes start/end/special characters.

    int byteCount = 0;
    unsigned char x = 0;

    // re-allocate array to max length
    data.ptr = realloc(data.ptr, maxLen * sizeof(unsigned char));
    data.N = maxLen;

    // wait for start character
    while (x != startByte)
    {
        read(serial_port, &x, sizeof(x));
        // debugPrintInt(x);
    }

    // read bytes until end character
    while (x != endByte)
    {
        data.ptr[byteCount] = x;
        read(serial_port, &x, sizeof(x));
        byteCount++;
    }

    data.ptr[byteCount] = x;
    data.N = byteCount + 1;

    data = decodeSpecialBytes(data);

    return data;
}

byteString _sendBytesToArduino(int serial_port, byteString data)
{
    // Encodes data message with start/end/special characters.
    // Writes message to Arduino over serial comms.

    data = encodeSpecialBytes(data);

    int n = write(serial_port, data.ptr, data.N);

    if (n < 0)
    {
        printf("Error writing to Arduino in _sendBytesToArduino(): %s\n", strerror(errno));
    }

    return data;
}

int sendRec(int serial_port, byteString data)
{
    // Sends data to Arduino.
    // Verifies data using Arduino confirmation message.
    // Returns 0 for success.

    int retval = 0;

    // copy input data to new array
    unsigned char *data_in;
    for (int i = 0; i < data.N; i++)
    {
        data_in[i] = data.ptr[i];
    }

    // send/receive data
    data = _sendBytesToArduino(serial_port, data);
    data = receiveBytes(serial_port, data);

    // verify Arduino confirmation
    for (int i = 0; i < data.N; i++)
    {
        if (data_in[i] != data.ptr[i])
            retval = 1;
    }

    return retval;
}

int serial_test()
{
    // Initializes serial port, sends test message, and confirms response.

    int serial_port = init_serial_port("/dev/ttyUSB0");
    if (serial_port < 0)
    {
        printf("[init_serial_port]: Error %i from init_serial_port(): %s\n", errno, strerror(errno));
        return 1;
    }

    waitForArduino(serial_port);

    byteString data;
    data.N = 2;
    data.ptr = calloc(data.N, sizeof(unsigned char));

    data.ptr[0] = 187;
    data.ptr[1] = 187;

    // 187=stop  193=forward
    int status = sendRec(serial_port, data);

    if (status != 0)
    {
        printf("Error: Send/Receive confirmation does not match.\n");
    }

    free(data.ptr);

    return 0;
}