#include "jtimer.h"


/**************************************************************************************************
* function 	: timer_t jset_timer(timer_t start_sec, long start_nsec, timer_t interval_sec, long interval_nsec, void* func, int sival);
* param		: timer_t start_sec => how many secs + start_nsec to start this timer
*			  long start_nsec => how manay secs + start_nsec to start this timer
*			  timer_t interval_sec => interval of this timer is interval_sec + interval_nsec	
*			  long interval_nsec => interval of this timer is interval_sec + interval_nsec
*			  void* func => timer callback function
*			  int sival => sival_int of timer evp 
* return    : timer_t, timer_id
****************************************************************************************************/
timer_t jset_timer(timer_t start_sec, long start_nsec, timer_t interval_sec, long interval_nsec, void* func, int sival){
	timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent));     
    
    evp.sigev_value.sival_int = sival;            
	evp.sigev_notify = SIGEV_THREAD;            
    evp.sigev_notify_function = func;       
     
    if (timer_create(CLOCKID, &evp, &timerid) == -1){
		log_error("fail to timer_create!\n");
        perror("fail to timer_create");
        return -1;
    }
	struct itimerspec it; 
    it.it_interval.tv_sec = interval_sec;
    it.it_interval.tv_nsec = interval_nsec;
    it.it_value.tv_sec = start_sec;
    it.it_value.tv_nsec = start_nsec;
 
    if (timer_settime(timerid, 0, &it, NULL) == -1){
		log_error("fail to timer_settime!\n");
        perror("fail to timer_settime");
        return -1;
    }
	return timerid;
}
