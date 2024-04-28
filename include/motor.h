#pragma once

// define struct for passing motor control parameters
typedef struct {
    int PIN;
    double speed;
    double duration;
} motor_struct;

// define struct for passing early termination args to thread 
typedef struct {
    pthread_t *p;
    pthread_t *p0;
    pthread_t *p1;
    int *serial_handle;
    int *pCondition;
} termination_struct;


int initialize_input(int PIN);      // Open gpiochip and claim PIN for input.
int initialize_output(int PIN);     // Open gpiochip and claim PIN for output.
int shutdown_PIN(int h, int PIN);   // Free PIN and close gpiochip.

void print_gpio_modes(void);        // Prints the mode for all GPIO pins
void debug_blink_pin(int PIN);

int run_motor(int h, int PIN, double speed, double duration);
void *run_motor_thread(void *arg);
void *early_thread_termination(void *arg);
void *update_condition(void *arg);
