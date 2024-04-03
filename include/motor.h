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
    int *pCondition;
} termination_struct;


int initialize_input(int PIN);
int initialize_output(int PIN);
int shutdown_PIN(int h, int PIN);
void print_gpio_modes(void);
void debug_blink_pin(int PIN);
int run_motor(int h, int PIN, double speed, double duration);
void *run_motor_thread(motor_struct *arg);
void *early_thread_termination(termination_struct *ts);
void *update_condition(int *pCondition);
// int motor_test();