// ================================================
//
// blink_rt.c
// Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
// Mar 27, 2017
//
// ================================================

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <wiringPi.h>

#define MY_PRIORITY (49) /* we use 49 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

#define LED 27 // BCM.GPIO pin 17 = wiringPi pin 0

void stack_prefault(void) {

        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);
        return;
}

int main(int argc, char* argv[])
{	

        struct timespec t;
        struct sched_param param;
        int interval = 50000; /* 50us*/

  	if (argc>=2 && atoi(argv[1])>0 ) {  // if we have a positive input value
     		interval = atoi(argv[1]);
  	}
	else {
		perror("invalid argument\n");
		exit(0);
	}		

        /* Declare ourself as a real time task */

        param.sched_priority = MY_PRIORITY;
        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler failed");
                exit(-1);
        }

        /* Lock memory */

        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
                perror("mlockall failed");
                exit(-2);
        }

        /* Pre-fault our stack */

        stack_prefault();

        clock_gettime(CLOCK_MONOTONIC ,&t);
        /* start after one second */
        //t.tv_sec = 0;
	t.tv_nsec = interval;

	/* setup the good jazz to blink*/
	wiringPiSetup();
        pinMode(LED, OUTPUT);

	int state = 0;	
	int ts = t.tv_sec;
	

        while(1) {
                /* wait until next shot */
                //clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);
		delayMicroseconds(interval);
                /* do the stuff */
		digitalWrite(LED, state^=1);

		clock_gettime(CLOCK_MONOTONIC, &t);
		if ((t.tv_sec - ts) > 30){
			printf("done\n");
			exit(0);
		}	 
               	/* calculate next shot */
                //t.tv_nsec += interval;

                /*while (t.tv_nsec >= NSEC_PER_SEC) {
                       t.tv_nsec -= NSEC_PER_SEC;
                        t.tv_sec++;
                }*/
   }
}
