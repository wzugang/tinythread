#ifndef INC__TT_TIME_H__
#define INC__TT_TIME_H__


/* Convert ticks to milliseconds */
uint32_t tt_ticks_to_msec (uint32_t ticks);
/* Convert milliseconds to ticks */
uint32_t tt_msec_to_ticks (uint32_t msec);

/* Sleep for some seconds */
void tt_sleep (uint32_t sec);
/* Sleep for some milliseconds */
void tt_msleep (uint32_t msec);
/* Sleep for some micorseconds */
void tt_usleep (uint32_t usec);

/* Get current tick count */
uint32_t tt_get_ticks (void);
/* Get current second time */
uint64_t tt_get_time (void);
/* set current second time */
uint64_t tt_set_time (uint64_t new_time);

#endif
