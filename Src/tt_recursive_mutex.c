#include "../Inc/tt_thread.h"


/* Available in: irq, thread. */
void tt_rmutex_init (TT_RMUTEX_T *rmutex)
{
	rmutex->lock_count	= 0;
	rmutex->thread		= NULL;
	tt_mutex_init (&rmutex->mutex);
}


/* Available in: irq, thread. */
void tt_rmutex_lock (TT_RMUTEX_T *rmutex)
{
	TT_THREAD_T *thread;
	if (sysIsInIRQ ())
		thread = (TT_THREAD_T *) -1;
	else
		thread = tt_thread_self ();
	
	if (rmutex->thread != thread)
	{
		tt_mutex_lock (&rmutex->mutex);
		rmutex->thread = thread;
	}
	rmutex->lock_count++;
}


/* Available in: irq, thread. */
int tt_rmutex_try_lock (TT_RMUTEX_T *rmutex)
{
	TT_THREAD_T *thread;
	if (sysIsInIRQ ())
		thread = (TT_THREAD_T *) -1;
	else
		thread = tt_thread_self ();
	
	if (rmutex->thread != thread)
	{
		int rt = tt_mutex_try_lock (&rmutex->mutex);
		if (rt != 0)
			return rt;

		rmutex->thread = thread;
	}
	rmutex->lock_count++;
	return 0;
}


/* Available in: irq. */
int tt_rmutex_can_lock (TT_RMUTEX_T *rmutex)
{
	return tt_mutex_can_lock (&rmutex->mutex);
}


/* Available in: irq, thread. */
void tt_rmutex_unlock (TT_RMUTEX_T *rmutex)
{
	ASSERT (rmutex->lock_count > 0);
	if (--rmutex->lock_count == 0)
	{
		rmutex->thread = NULL;
		tt_mutex_unlock (&rmutex->mutex);
	}
}


