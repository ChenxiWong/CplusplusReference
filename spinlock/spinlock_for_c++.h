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

    tail =(mcs_lock_t *) xchg_64(m, (void*)me);

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

    pred =(k42lock*) xchg_64(&l->tail, (void*)&me);
    if (pred)
    {
        me.tail = (k42lock*) 1;

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

/* 改进mcs、k42算法  */
typedef struct listlock_t listlock_t;
struct listlock_t
{
    listlock_t *next;
    int spin;
};
typedef struct listlock_t *listlock;

#define LLOCK_FLAG (void *)1

void listlock_lock(listlock *l)
{
    listlock_t me;
    listlock_t *tail;

    /* Fast path - no users */
    if (!cmpxchg(l, NULL, LLOCK_FLAG)) return;

    me.next = (listlock_t*)LLOCK_FLAG;
    me.spin = 0;

    /* Convert into a wait list */
    tail = (listlock_t *)xchg_64(l, (void*)&me);

    if (tail)
    {
        /* Add myself to the list of waiters */
        if (tail == LLOCK_FLAG) tail = NULL;
        me.next = tail;

        /* Wait for being able to go */
        while (!me.spin) cpu_relax();

        return;
    }

    /* Try to convert to an exclusive lock */
    if (cmpxchg(l, &me, LLOCK_FLAG) == &me) return;

    /* Failed - there is now a wait list */
    tail = *l;

    /* Scan to find who is after me */
    while (1)
    {
        /* Wait for them to enter their next link */
        while (tail->next == LLOCK_FLAG) cpu_relax();

        if (tail->next == &me)
        {
            /* Fix their next pointer */
            tail->next = NULL;

            return;
        }

        tail = tail->next;
    }
}

void listlock_unlock(listlock *l)
{
    listlock_t *tail;
    listlock_t *tp;

    while (1)
    {
        tail = *l;

        barrier();

        /* Fast path */
        if (tail == LLOCK_FLAG)
        {
            if (cmpxchg(l, LLOCK_FLAG, NULL) == LLOCK_FLAG) return;

            continue;
        }

        tp = NULL;

        /* Wait for partially added waiter */
        while (tail->next == LLOCK_FLAG) cpu_relax();

        /* There is a wait list */
        if (tail->next) break;

        /* Try to convert to a single-waiter lock */
        if (cmpxchg(l, tail, LLOCK_FLAG) == tail)
        {
            /* Unlock */
            tail->spin = 1;

            return;
        }

        cpu_relax();
    }

    /* A long list */
    tp = tail;
    tail = tail->next;

    /* Scan wait list */
    while (1)
    {
        /* Wait for partially added waiter */
        while (tail->next == LLOCK_FLAG) cpu_relax();

        if (!tail->next) break;

        tp = tail;
        tail = tail->next;
    }

    tp->next = NULL;

    barrier();

    /* Unlock */
    tail->spin = 1;
}

int listlock_trylock(listlock *l)
{
    /* Simple part of a spin-lock */
    if (!cmpxchg(l, NULL, LLOCK_FLAG)) return 0;

    /* Failure! */
    return EBUSY;
}

/* 进一步改进后的自旋锁  */
typedef struct bitlistlock_t bitlistlock_t;
struct bitlistlock_t
{
    bitlistlock_t *next;
    int spin;
};

typedef bitlistlock_t *bitlistlock;

#define BLL_USED ((bitlistlock_t *) -2LL)
#include <stdint.h>
static void bitlistlock_lock(bitlistlock *l)
{
    bitlistlock_t me;
    bitlistlock_t *tail;

    /* Grab control of list */
    while (atomic_bitsetandtest(l, 0)) cpu_relax();

    /* Remove locked bit */
    tail = (bitlistlock_t *) ((uintptr_t) *l & ~1LL);

    /* Fast path, no waiters */
    if (!tail)
    {
        /* Set to be a flag value */
        *l = BLL_USED;
        return;
    }

    if (tail == BLL_USED) tail = NULL;
    me.next = tail;
    me.spin = 0;

    barrier();

    /* Unlock, and add myself to the wait list */
    *l = &me;

    /* Wait for the go-ahead */
    while (!me.spin) cpu_relax();
}

static void bitlistlock_unlock(bitlistlock *l)
{
    bitlistlock_t *tail;
    bitlistlock_t *tp;

    /* Fast path - no wait list */
    if (cmpxchg(l, BLL_USED, NULL) == BLL_USED) return;

    /* Grab control of list */
    while (atomic_bitsetandtest(l, 0)) cpu_relax();

    tp = *l;

    barrier();

    /* Get end of list */
    tail = (bitlistlock_t *) ((uintptr_t) tp & ~1LL);

    /* Actually no users? */
    if (tail == BLL_USED)
    {
        barrier();
        *l = NULL;
        return;
    }

    /* Only one entry on wait list? */
    if (!tail->next)
    {
        barrier();

        /* Unlock bitlock */
        *l = BLL_USED;

        barrier();

        /* Unlock lock */
        tail->spin = 1;

        return;
    }

    barrier();

    /* Unlock bitlock */
    *l = tail;

    barrier();

    /* Scan wait list for start */
    do
    {
        tp = tail;
        tail = tail->next;
    }
    while (tail->next);

    tp->next = NULL;

    barrier();

    /* Unlock */
    tail->spin = 1;
}

static int bitlistlock_trylock(bitlistlock *l)
{
    if (!*l && (cmpxchg(l, NULL, BLL_USED) == NULL)) return 0;

    return EBUSY;
}

/*  进一步改进自旋锁  */


/* Bit-lock for editing the wait block */
#define SLOCK_LOCK 1
#define SLOCK_LOCK_BIT 0

/* Has an active user */
#define SLOCK_USED 2

#define SLOCK_BITS 3

typedef struct slock slock;
struct slock
{
    uintptr_t p;
};

typedef struct slock_wb slock_wb;
struct slock_wb
{
    /*
     * last points to the last wait block in the chain.
     * The value is only valid when read from the first wait block.
     */
    slock_wb *last;

    /* next points to the next wait block in the chain. */
    slock_wb *next;

    /* Wake up? */
    int wake;
};

/* Wait for control of wait block */
static slock_wb *slockwb(slock *s)
{
    uintptr_t p;

    /* Spin on the wait block bit lock */
    while (atomic_bitsetandtest(&s->p, SLOCK_LOCK_BIT))
    {
        cpu_relax();
    }

    p = s->p;

    if (p <= SLOCK_BITS)
    {
        /* Oops, looks like the wait block was removed. */
        atomic_dec(&s->p);
        return NULL;
    }

    return (slock_wb *)(p - SLOCK_LOCK);
}

static void slock_lock(slock *s)
{
    slock_wb swblock;

    /* Fastpath - no other readers or writers */
    if (!s->p && (cmpxchg(&s->p, 0, SLOCK_USED) == 0)) return;

    /* Initialize wait block */
    swblock.next = NULL;
    swblock.last = &swblock;
    swblock.wake = 0;

    while (1)
    {
        uintptr_t p = s->p;

        cpu_relax();

        /* Fastpath - no other readers or writers */
        if (!p)
        {
            if (cmpxchg(&s->p, 0, SLOCK_USED) == 0) return;
            continue;
        }

        if (p > SLOCK_BITS)
        {
            slock_wb *first_wb, *last;

            first_wb = slockwb(s);
            if (!first_wb) continue;

            last = first_wb->last;
            last->next = &swblock;
            first_wb->last = &swblock;

            /* Unlock */
            barrier();
            s->p &= ~SLOCK_LOCK;

            break;
        }

        /* Try to add the first wait block */
        if (cmpxchg(&s->p, p, (uintptr_t)&swblock) == p) break;
    }

    /* Wait to acquire exclusive lock */
    while (!swblock.wake) cpu_relax();
}

static void slock_unlock(slock *s)
{
    slock_wb *next;
    slock_wb *wb;
    uintptr_t np;

    while (1)
    {
        uintptr_t p = s->p;

        /* This is the fast path, we can simply clear the SRWLOCK_USED bit. */
        if (p == SLOCK_USED)
        {
            if (cmpxchg(&s->p, SLOCK_USED, 0) == SLOCK_USED) return;
            continue;
        }

        /* There's a wait block, we need to wake the next pending user */
        wb = slockwb(s);
        if (wb) break;

        cpu_relax();
    }

    next = wb->next;
    if (next)
    {
        /*
         * There's more blocks chained, we need to update the pointers
         * in the next wait block and update the wait block pointer.
         */
        np = (uintptr_t) next;

        next->last = wb->last;
    }
    else
    {
        /* Convert the lock to a simple lock. */
        np = SLOCK_USED;
    }

    barrier();
    /* Also unlocks lock bit */
    s->p = np;
    barrier();

    /* Notify the next waiter */
    wb->wake = 1;

    /* We released the lock */
}

static int slock_trylock(slock *s)
{
    /* No other readers or writers? */
    if (!s->p && (cmpxchg(&s->p, 0, SLOCK_USED) == 0)) return 0;

    return EBUSY;
}

/* 下面是另外一种实现方式，称为stack-lock算法，*/

typedef struct stlock_t stlock_t;
struct stlock_t
{
    stlock_t *next;
};

typedef struct stlock_t *stlock;

 __attribute__((noinline)) void stlock_lock(stlock *l)
{
    stlock_t *me = NULL;

    barrier();
    me = (stlock_t*)xchg_64(l, (void*)&me);

    /* Wait until we get the lock */
    while (me) cpu_relax();
}

#define MAX_STACK_SIZE (1<<12)

 __attribute__((noinline)) int on_stack(void *p)
{
    int x;

    uintptr_t u = (uintptr_t) &x;

    return ((u - (uintptr_t)p + MAX_STACK_SIZE) < MAX_STACK_SIZE * 2);
}

 __attribute__((noinline)) void stlock_unlock(stlock *l)
{
    stlock_t *tail = *l;
    barrier();

    /* Fast case */
    if (on_stack(tail))
    {
        /* Try to remove the wait list */
        if (cmpxchg(l, tail, NULL) == tail) return;

        tail = *l;
    }

    /* Scan wait list */
    while (1)
    {
        /* Wait for partially added waiter */
        while (!tail->next) cpu_relax();

        if (on_stack(tail->next)) break;

        tail = tail->next;
    }

    barrier();

    /* Unlock */
    tail->next = NULL;
}

 int stlock_trylock(stlock *l)
{
    stlock_t me;

    if (!cmpxchg(l, NULL, &me)) return 0;

    return EBUSY;
}


/*  进一步改进后的自旋锁  */
typedef struct plock_t plock_t;
struct plock_t
{
    plock_t *next;
};

typedef struct plock plock;
struct plock
{
    plock_t *next;
    plock_t *prev;
    plock_t *last;
};

void plock_lock(plock *l)
{
    plock_t *me = NULL;
    plock_t *prev;

    barrier();
    me = (plock_t*)xchg_64(l, (void*)&me);

    prev = NULL;

    /* Wait until we get the lock */
    while (me)
    {
        /* Scan wait list for my previous */
        if (l->next != (plock_t *) &me)
        {
            plock_t *t = l->next;

            while (me)
            {
                if (t->next == (plock_t *) &me)
                {
                    prev = t;

                    while (me) cpu_relax();

                    goto done;
                }

                if (t->next) t = t->next;
                cpu_relax();
            }
        }
        cpu_relax();
    }

done:
    l->prev = prev;
    l->last = (plock_t *) &me;
}

void plock_unlock(plock *l)
{
    plock_t *tail;

    /* Do I know my previous? */
    if (l->prev)
    {
        /* Unlock */
        l->prev->next = NULL;
        return;
    }

    tail = l->next;
    barrier();

    /* Fast case */
    if (tail == l->last)
    {
        /* Try to remove the wait list */
        if (cmpxchg(&l->next, tail, NULL) == tail) return;

        tail = l->next;
    }

    /* Scan wait list */
    while (1)
    {
        /* Wait for partially added waiter */
        while (!tail->next) cpu_relax();

        if (tail->next == l->last) break;

        tail = tail->next;
    }

    barrier();

    /* Unlock */
    tail->next = NULL;
}

int plock_trylock(plock *l)
{
    plock_t me;

    if (!cmpxchg(&l->next, NULL, &me))
    {
        l->last = &me;
        return 0;
    }

    return EBUSY;
}


/* 下面介绍另外一种算法，ticket lock算法，实际上，Linux内核正是采用了该算法，不过考虑到执行效率，人家是以汇编形式写的， */

typedef union ticketlock ticketlock;

union ticketlock
{
    unsigned u;
    struct
    {
        unsigned short ticket;
        unsigned short users;
    } s;
};

void ticket_lock(ticketlock *t)
{
    unsigned short me = atomic_xadd(&t->s.users, 1);

    while (t->s.ticket != me) cpu_relax();
}

void ticket_unlock(ticketlock *t)
{
    barrier();
    t->s.ticket++;
}

int ticket_trylock(ticketlock *t)
{
    unsigned short me = t->s.users;
    unsigned short menew = me + 1;
    unsigned cmp = ((unsigned) me << 16) + me;
    unsigned cmpnew = ((unsigned) menew << 16) + me;

    if (cmpxchg(&t->u, cmp, cmpnew) == cmp) return 0;

    return EBUSY;
}

int ticket_lockable(ticketlock *t)
{
    ticketlock u = *t;
    barrier();
    return (u.s.ticket == u.s.users);
}
















#endif  /*SPINLOCK_H*/
