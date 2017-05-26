// Last Update:2017-05-26 15:40:15
/**
 * @file spinlock.h
 * @brief 根据一篇文档整理的自旋锁实现头文件相关（编译器相关，gcc编译器下实现)
 * @author wangchenxi
 * @version 0.1.00
 * @date 2017-05-26
 */

#ifndef SPINLOCK_H
#define SPINLOCK_H


// gcc 实现的原子操作原语
#define atomic_xadd(P, V) __sync_fetch_and_add((P), (V))
#define cmpxchg(P, O, N) __sync_val_compare_and_swap((P), (O), (N))
#define atomic_inc(P) __sync_add_and_fetch((P), 1)
#define atomic_dec(P) __sync_add_and_fetch((P), -1)
#define atomic_add(P, V) __sync_add_and_fetch((P), (V))
#define atomic_set_bit(P, V) __sync_or_and_fetch((P), 1<<(V))
#define atomic_clear_bit(P, V) __sync_and_and_fetch((P), ~(1<<(V)))



/* Compile read-write barrier 读写栅栏 */
#define barrier() asm volatile("": : :"memory")

/* Pause instruction to prevent excess processor bus usage */
/* 暂停指令以防止多余的处理器总线使用 */
#define cpu_relax() asm volatile("pause\n": : :"memory")

/* Atomic exchange (of various sizes) */
/*  原子交换操作  */
static inline void *xchg_64(void *ptr, void *x)
{
    __asm__ __volatile__("xchgq %0,%1"
            :"=r" ((unsigned long long) x)
            :"m" (*(volatile long long *)ptr), "0" ((unsigned long long) x)
            :"memory");

    return x;
}

static inline unsigned xchg_32(void *ptr, unsigned x)
{
    __asm__ __volatile__("xchgl %0,%1"
            :"=r" ((unsigned) x)
            :"m" (*(volatile unsigned *)ptr), "0" (x)
            :"memory");

    return x;
}

static inline unsigned short xchg_16(void *ptr, unsigned short x)
{
    __asm__ __volatile__("xchgw %0,%1"
            :"=r" ((unsigned short) x)
            :"m" (*(volatile unsigned short *)ptr), "0" (x)
            :"memory");

    return x;
}

/* Test and set a bit */
static inline char atomic_bitsetandtest(void *ptr, int x)
{
    char out;
    __asm__ __volatile__("lock; bts %2,%1\n"
            "sbb %0,%0\n"
            :"=r" (out), "=m" (*(volatile long long *)ptr)
            :"Ir" (x)
            :"memory");

    return out;
}




// 最简单
#define EBUSY 1
typedef unsigned spinlock;

static void spin_lock(spinlock *lock)
{
    while (1)
    {
        if (!xchg_32(lock, EBUSY)) return;

        while (*lock) cpu_relax();
    }
}

static void spin_unlock(spinlock *lock)
{
    barrier();
    *lock = 0;
}

static int spin_trylock(spinlock *lock)
{
    return xchg_32(lock, EBUSY);
}

/* MCS 自旋锁 */
typedef struct mcs_lock_t mcs_lock_t;
struct mcs_lock_t
{
    mcs_lock_t *next;
    int spin;
};
typedef struct mcs_lock_t *mcs_lock;
#ifndef NULL
#define NULL 0
#endif

static void lock_mcs(mcs_lock *m, mcs_lock_t *me)
{
    mcs_lock_t *tail;

    me->next = NULL;
    me->spin = 0;

    tail = xchg_64(m, me);

    /* No one there? */
    if (!tail) return;

    /* Someone there, need to link in */
    tail->next = me;

    /* Make sure we do the above setting of next. */
    barrier();

    /* Spin on my spin variable */
    while (!me->spin) cpu_relax();

    return;
}

static void unlock_mcs(mcs_lock *m, mcs_lock_t *me)
{
    /* No successor yet? */
    if (!me->next)
    {
        /* Try to atomically unlock */
        if (cmpxchg(m, me, NULL) == me) return;

        /* Wait for successor to appear */
        while (!me->next) cpu_relax();
    }

    /* Unlock next one */
    me->next->spin = 1;
}

static int trylock_mcs(mcs_lock *m, mcs_lock_t *me)
{
    mcs_lock_t *tail;

    me->next = NULL;
    me->spin = 0;

    /* Try to lock */
    tail = cmpxchg(m, NULL, &me);

    /* No one was there - can quickly return */
    if (!tail) return 0;

    return EBUSY;
}

/* 自旋锁算法，K42锁算法  */
typedef struct k42lock k42lock;
struct k42lock
{
    k42lock *next;
    k42lock *tail;
};

static void k42_lock(k42lock *l)
{
    k42lock me;
    k42lock *pred, *succ;
    me.next = NULL;

    barrier();

    pred = xchg_64(&l->tail, &me);
    if (pred)
    {
        me.tail = (void *) 1;

        barrier();
        pred->next = &me;
        barrier();

        while (me.tail) cpu_relax();
    }

    succ = me.next;

    if (!succ)
    {
        barrier();
        l->next = NULL;

        if (cmpxchg(&l->tail, &me, &l->next) != &me)
        {
            while (!me.next) cpu_relax();

            l->next = me.next;
        }
    }
    else
    {
        l->next = succ;
    }
}

static void k42_unlock(k42lock *l)
{
    k42lock *succ = l->next;

    barrier();

    if (!succ)
    {
        if (cmpxchg(&l->tail, &l->next, NULL) == (void *) &l->next) return;

        while (!l->next) cpu_relax();
        succ = l->next;
    }

    succ->tail = NULL;
}

static int k42_trylock(k42lock *l)
{
    if (!cmpxchg(&l->tail, NULL, &l->next)) return 0;

    return EBUSY;
}




#endif  /*SPINLOCK_H*/
