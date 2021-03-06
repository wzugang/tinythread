/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "../../../Inc/tt_thread.h"


#define LED0_GPIO_GRP	P3
#define LED0_GPIO_BIT	4
#define LED1_GPIO_GRP	P3
#define LED1_GPIO_BIT	5
#define LED2_GPIO_GRP	P3
#define LED2_GPIO_BIT	6
#define LED3_GPIO_GRP	P3
#define LED3_GPIO_BIT	7



/* MY_SEM_T is implemented by a condition and a mutex */
typedef struct
{
	TT_MUTEX_T		mutex;
	TT_COND_T		cond;
	unsigned int	value;
} MY_SEM_T;


void my_sem_init (MY_SEM_T *sem, unsigned int value)
{
	tt_mutex_init (&sem->mutex);
	tt_cond_init (&sem->cond);

	sem->value = value;
}


void my_sem_down (MY_SEM_T *sem)
{
	tt_mutex_lock (&sem->mutex);
	while (sem->value == 0)
	{
		tt_cond_wait (&sem->cond, &sem->mutex);
	}
	--sem->value;
	tt_mutex_unlock (&sem->mutex);
}

void my_sem_up (MY_SEM_T *sem)
{
	/* The 4 piece of code should all work! */
#if 0
	tt_mutex_lock (&sem->mutex);
	++sem->value;
	tt_cond_signal (&sem->cond);
	tt_mutex_unlock (&sem->mutex);

#elif 1
	tt_mutex_lock (&sem->mutex);
	++sem->value;
	tt_cond_broadcast (&sem->cond);
	tt_mutex_unlock (&sem->mutex);

#elif 1
	tt_mutex_lock (&sem->mutex);
	++sem->value;
	tt_mutex_unlock (&sem->mutex);

	tt_cond_signal (&sem->cond);
#else

	tt_mutex_lock (&sem->mutex);
	++sem->value;
	tt_mutex_unlock (&sem->mutex);

	tt_cond_broadcast (&sem->cond);
#endif
	
}



#define THREAD_NUM	4
typedef struct
{
	GPIO_T		*gpio_group;
	int			gpio_bit;
	MY_SEM_T	*sem;
} THREAD_ARG_T;

static TT_THREAD_T	*thread[THREAD_NUM];
static unsigned int	stack[THREAD_NUM][TT_THREAD_BUFFER_SIZE(256)/sizeof(unsigned int)];


void thread_entry(void *arg)
{
	THREAD_ARG_T *the_arg = (THREAD_ARG_T *)arg;

	my_sem_down (the_arg->sem);
	while (1)
	{
		int cur_ticks = tt_get_ticks ();

		if (cur_ticks % 1024 < 4)
		{
			my_sem_up (the_arg->sem);
			//tt_thread_yield ();
			//tt_sleep(1);
			tt_msleep(rand()%64);
			my_sem_down (the_arg->sem);
		}

		
		if(cur_ticks % 64 < 32)
			the_arg->gpio_group->DOUT &= ~(1 << the_arg->gpio_bit);	/* Turn off LED */
		else
			the_arg->gpio_group->DOUT |= (1 << the_arg->gpio_bit);	/* Turn on LED */
	}
}


int main()
{
	int i;
	THREAD_ARG_T	arg[THREAD_NUM];
	MY_SEM_T		sem;

	/* The clock frequency of systick may be changed by user's application.
	   Please change the value of SYSTICK_CLOCK according to real situration */
#define TT_SYSTICK_CLOCK		22118400
	/* Initialize TinyThread */
	tt_init(TT_SYSTICK_CLOCK);
	
	/* Open LED GPIO for testing */
	_GPIO_SET_PIN_MODE(LED0_GPIO_GRP, LED0_GPIO_BIT, GPIO_PMD_OUTPUT);
	_GPIO_SET_PIN_MODE(LED1_GPIO_GRP, LED1_GPIO_BIT, GPIO_PMD_OUTPUT);
	_GPIO_SET_PIN_MODE(LED2_GPIO_GRP, LED2_GPIO_BIT, GPIO_PMD_OUTPUT);
	_GPIO_SET_PIN_MODE(LED3_GPIO_GRP, LED3_GPIO_BIT, GPIO_PMD_OUTPUT);

	/* Set semaphore value to 3, so that always 3 threads' LED is blinking */
	my_sem_init (&sem, 3);

	/* Create and run thread */
	arg[0].gpio_group = LED0_GPIO_GRP;
	arg[1].gpio_group = LED1_GPIO_GRP;
	arg[2].gpio_group = LED2_GPIO_GRP;
	arg[3].gpio_group = LED3_GPIO_GRP;
	arg[0].gpio_bit = LED0_GPIO_BIT;
	arg[1].gpio_bit = LED1_GPIO_BIT;
	arg[2].gpio_bit = LED2_GPIO_BIT;
	arg[3].gpio_bit = LED3_GPIO_BIT;

	for (i = 0; i < THREAD_NUM; ++i)
	{
		arg[i].sem = &sem;
		thread[i] = tt_thread_create("thread",			/* thread Name */
									 0,					/* thread priority */
									 stack[i],			/* stack pointer */
									 sizeof(stack[i]),	/* stack size */
									 thread_entry,		/* thread entry function */
									 (void *)&arg[i]	/* argument for thread entry function */
									 );	
	}
	

	tt_thread_exit ();
	return 0;
}


