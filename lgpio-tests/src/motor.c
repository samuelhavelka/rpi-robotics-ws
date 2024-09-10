#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <time.h>

#include <lgpio.h>

#include "motor.h"


int initialize_input(int PIN) {
    /* Open gpiochip and claim PIN for input. */
    int h;
    int CHIP = 4;

    h = lgGpiochipOpen(CHIP); 

    int status;
    status = lgGpioClaimInput(h, 0, PIN);
    if (status < 0) {
        fprintf(stderr, "can't claim GPIO (PIN %d) for output (%s)\n", PIN, lguErrorText(status));
        return -1;
    }

    return h;
}

int initialize_output(int PIN) {
    /* Open gpiochip and claim PIN for output. */
    int h;
    int CHIP = 4;

    h = lgGpiochipOpen(CHIP); 

    int status;
    status = lgGpioClaimOutput(h, 0, PIN, 0);
    if (status < 0) {
        fprintf(stderr, "can't claim GPIO (PIN %d) for output (%s)\n", PIN, lguErrorText(status));
        return -1;
    }

    return h;
}

int shutdown_PIN(int h, int PIN) {
    /* Free PIN and close gpiochip. */
    
    lgGpioFree(h, PIN);

    int status;
    status = lgGpiochipClose(h); // close gpiochip0
    if (status < 0) {
        fprintf(stderr, "gpiochip0 close failed on PIN %d.\n", PIN);  // close failed
    }
    return status;
}

void print_gpio_modes(void) {
    /* 
    Prints the mode for all GPIO pins:

        Bit	Value   Meaning
        0	1	    Kernel: In use by the kernel
        1	2	    Kernel: Output
        2	4	    Kernel: Active low
        3	8	    Kernel: Open drain
        4	16	    Kernel: Open source
        5	32	    Kernel: Pull up set
        6	64	    Kernel: Pull down set
        7	128	    Kernel: Pulls off set
        8	256	    LG: Input
        9	512	    LG: Output
        10	1024	LG: Alert
        11	2048	LG: Group
        12	4096	LG: ---
        13	8192	LG: ---
        14	16384	LG: ---
        15	32768	LG: ---
        16	65536	Kernel: Input
        17	1<<17	Kernel: Rising edge alert
        18	1<<18	Kernel: Falling edge alert
        19	1<<19	Kernel: Realtime clock alert
    */

    int status;
    int h;

    h = lgGpiochipOpen(0); // open /dev/gpiochip0
        if (h < 0) {
            // open error
            fprintf(stderr, "failed to open gpiochip.\n");
            return;
        }

    for (int i = 0; i <= 31; i++) {

        status = lgGpioGetMode(h, i); // get level of GPIO x
        if (status>0){
            printf("GPIO Mode for pin %d: %d\n", i, status); 
        } else {
            printf("Error on pin %d: %s\n", i, lguErrorText(status)); 
        }

    }

    status = lgGpiochipClose(h); // close gpiochip0
        if (status < 0){
            // close failed
            fprintf(stderr, "failed to close gpiochip.\n");
        }
    return;
}

void debug_blink_pin(int PIN) {
    /*
    Debug program which alternates output level of given pin
    PIN = output pin
    */

    int level;
    int h;
    h = initialize_output(PIN);

    while (1) {

        lgGpioWrite(h, PIN, 0); // alternate PIN level
        level = lgGpioRead(h, PIN); // get level of GPIO PIN
        printf("Level of GPIO %d: %d\n", PIN, level);

        lguSleep(0.5);

        lgGpioWrite(h, PIN, 1); // alternate PIN level
        level = lgGpioRead(h, PIN); // get level of GPIO PIN
        printf("Level of GPIO %d: %d\n", PIN, level);

        lguSleep(0.5);  
    }

    shutdown_PIN(h, PIN);
    return;
}


int run_motor(int h, int PIN, double speed, double duration) {
    /*
    h = handle
    PIN = GPIO pin number
    speed [-1, 1] where +1 is full forward, -1 is full reverse, and 0 is neutral
                  RECOMMEND to keep values < 0.2 for testing
    duration = time to run motor in seconds
    */

    int status;
    int freq = 200;

    // Duty Cycle ranges from 20-40% 
    double dutyCycle = (speed*10) + 30;
    int pwmCycles = (int)(duration * freq);

    printf("PIN %d  pwmCycles=%i  dutyCycle=%f  duration=%f\n", 
            PIN, pwmCycles, dutyCycle, duration);

    status = lgTxPwm(h, PIN, freq, dutyCycle, 0, pwmCycles);

    if (status < 0) {
        fprintf(stderr, "Error on PIN %d: (%s)\n", PIN, lguErrorText(status));
        return -1;
    }

    while (lgTxBusy(h, PIN, LG_TX_PWM)) lguSleep(0.0000000001);

    return 0;
}


void *run_motor_thread(void *arg) {
    /* Start motor running in a new thread */
    
    int h;
    motor_struct *ptr = arg;

    printf("Start motor thread %d\n", ptr->PIN);

    h = initialize_output(ptr->PIN);
    run_motor(h, ptr->PIN, ptr->speed, ptr->duration);
    shutdown_PIN(h, ptr->PIN);

    printf("End motor thread %d\n", ptr->PIN);
}

void *early_thread_termination(void *arg) {
    /* Wait for stop condition to terminate thread */

    termination_struct *ts = arg;

    pthread_t *p = ts->p;
    int *pCondition = ts->pCondition;
    
    while(1){
        if (*pCondition==1){
            lgThreadStop(p);
        }
        lguSleep(0.2);
    }
}

void *update_condition(void *arg) {
    /* Update the stop condition for a thread after 3 sec */
    int *pCondition = arg;
    lguSleep(3);
    *pCondition = 1;
}



