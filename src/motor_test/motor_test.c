#include <stdio.h>
#include <lgpio.h>
#include "motor.h"


int main()
{
    printf("Running motor test...\n\n");
    int status;
    int h1;
    int h2;
    int C1 = 0;

    motor_struct md1;
    md1.PIN = 5;
    md1.speed = 0.3;
    md1.duration = 3;

    motor_struct md2;
    md2.PIN = 6;
    md2.speed = 0.5;
    md2.duration = 5;

    int num_threads = 2;
    pthread_t *p1[num_threads];

    p1[0] = lgThreadStart(run_motor_thread, &md1);
    p1[1] = lgThreadStart(run_motor_thread, &md2);

    termination_struct ts;
    ts.p = p1[0];
    ts.pCondition = &C1;

    pthread_t tid;
    pthread_create(&tid, NULL, early_thread_termination, &ts);

    pthread_t tid2;
    pthread_create(&tid2, NULL, update_condition, ts.pCondition);

    for (int i = 0; i < num_threads; i++) {
        status = pthread_join(*p1[i], NULL);
    }

    printf("done\n");
    return 0;
}