#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RTOS_MAX_TASKS 16
#define RTOS_STACK_SIZE 2048
#define RTOS_TICK_RATE_MS 1

//task

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED
} task_state_t;

typedef struct task_t task_t;
typedef void (*task_func_t)(void *arg);

struct task_t {
    void *sp;
    task_state_t state;
    uint32_t delay_ticks;
    uint8_t priority;
    task_func_t entry;
    void *arg;
    char name[16];
    uint8_t stack[RTOS_STACK_SIZE];
};

//list

typedef struct list_node {
    void *data;
    struct list_node *next;
    struct list_node *prev;
} list_node_t;

typedef struct {
    list_node_t* head; //for write
    list_node_t* tail; //for read
    uint32_t count;
} list_t;

void list_init(list_t *list);
void list_push_back(list_t *list, void *data);
void *list_pop_front(list_t *list);
void list_remove(list_t *list, void *data);
bool list_is_empty(const list_t *list);

//rtos
void my_rtos_init(void);
task_t* my_task_create(task_func_t func, uint8_t priority, void *arg, const char *name);
void my_task_delay(uint32_t ms);
void my_task_yield(void);
void my_task_exit(void);
void my_rtos_start(void);

task_t *scheduler_current(void);

//binary semaphore

typedef struct {
    uint8_t count; // 0/1.
    list_t waiters;
} semaphore_b_t;

semaphore_b_t *sem_create(uint8_t inital_count);
bool sem_take(semaphore_b_t* sem, uint8_t count);
void sem_give(semaphore_b_t* sem);

//mutex

typedef struct {
    volatile bool locked;
    task_t *owner;
    uint8_t saved_priority;
    uint32_t count;
    list_t waiters;
} mutex_t;

mutex_t* mutex_create(void);
bool mutex_lock(mutex_t* mutex, uint32_t timeout_ms);
bool mutex_unlock(mutex_t* mutex);

// queue

typedef struct {
    uint8_t *buffer;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    mutex_t lock;
    size_t count;
    semaphore_b_t not_full;
    semaphore_b_t not_empty;
} queue_t;

queue_t *queue_create(size_t capacity, size_t item_size);
bool queue_send(queue_t *q, const void* item, uint32_t timeout_ms);
bool queue_recv(queue_t *q, void* item, uint32_t timeout_ms);