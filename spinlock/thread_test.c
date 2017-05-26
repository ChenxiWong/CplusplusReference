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


void* add_sign(void* p)
{
    int i = 0;
    while( i != 10000)
    {
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
        ++i;
    }
    printf("hello one.\n");
}

int main(int argc, char* argv[])
{
    pthread_t tid1, tid2;
#ifndef M3
    pthread_create(&tid1, NULL, add_sign, (void*)NULL);
    pthread_create(&tid2, NULL, add_sign, (void*)NULL);
#else
    pthread_create(&tid1, NULL, add_sign, (void*)&s3_lock1);
    pthread_create(&tid2, NULL, add_sign, (void*)&s3_lock2);
#endif
    sleep(3);
    printf("------%lld \n", sign);
}
