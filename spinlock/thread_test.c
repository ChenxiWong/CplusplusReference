// Last Update:2017-05-26 15:09:21
/**
 * @file thread_test.c
 * @brief 
 * @author wangchenxi
 * @version 0.1.00
 * @date 2017-05-26
 */

#include "spinlock.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

unsigned long long sign;

#ifdef M1
spinlock  s1_lock ;
#endif

#ifdef M2
int lock;
#endif

#ifdef M3
mcs_lock_t s3_lock1;
mcs_lock_t s3_lock2;
mcs_lock s3_list;
#endif
#ifdef M4
k42lock s4_list;
#endif

#ifdef M5
listlock s5_lock;
#endif

#ifdef M6
bitlistlock s6_lock;
#endif

#ifdef M7
slock s7_lock;
#endif

#ifdef M8
stlock s8_lock;
#endif
#ifdef M9
plock s9_lock;
#endif
#ifdef M10
ticketlock s10_lock;
#endif

void* add_sign(void* p)
{
    int i = 0;
    while( i != 10000)
    {
        ++i;
#ifdef M0
        __sync_fetch_and_add(&sign, 1);
#endif

#ifdef M1
        spin_lock(&s1_lock);
        ++sign;
        spin_unlock(&s1_lock);
#endif

#ifdef M2
        while(!__sync_bool_compare_and_swap(&lock, 0, 1));
        ++sign;
        while(!__sync_bool_compare_and_swap(&lock, 1, 0));
#endif

#ifdef M3
        lock_mcs(&s3_list, (mcs_lock_t*)p);
        ++sign;
        unlock_mcs(&s3_list, (mcs_lock_t*)p);
#endif
#ifdef M4
        k42_lock(&s4_list);
        ++sign;
        k42_unlock(&s4_list);
#endif
#ifdef M5
        listlock_lock(&s5_lock);
        ++sign;
        listlock_unlock(&s5_lock);
#endif
#ifdef M6
        bitlistlock_lock(&s6_lock);
        ++sign;
        bitlistlock_unlock(&s6_lock);
#endif
#ifdef M7
        slock_lock(&s7_lock);
        ++sign;
        slock_unlock(&s7_lock);
#endif
#ifdef M8
        stlock_lock(&s8_lock);
        ++sign;
        stlock_unlock(&s8_lock);
#endif
#ifdef M9
        plock_lock(&s9_lock);
        ++sign;
        plock_unlock(&s9_lock);
#endif
#ifdef M10
        ticket_lock(&s10_lock);
        ++sign;
        ticket_unlock(&s10_lock);
#endif
    }
    printf("hello one.\n");
    return (void*) NULL;
}

int main(int argc, char* argv[])
{
    pthread_t tid1, tid2;
#ifdef M3
    pthread_create(&tid1, NULL, add_sign, (void*)&s3_lock1);
    pthread_create(&tid2, NULL, add_sign, (void*)&s3_lock2);
#else
    pthread_create(&tid1, NULL, add_sign, (void*)NULL);
    pthread_create(&tid2, NULL, add_sign, (void*)NULL);
#endif
    while(1)
    {
        sleep(1);
        printf("------%lld \n", sign);
    }
    return 0;
}
