#include <rtos.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rtos.h"
#include "esp_timer.h"
#include "esp_log.h"
static const char *TAG = "HEX_RTOS";

static task_t *tasks[RTOS_MAX_TASKS];
static uint8_t num_tasks = 0;
static task_t *current_task = NULL;
static volatile uint32_t sys_ticks = 0;
static bool rtos_running = false;

extern void port_switch_context(void **old_sp, void* new_sp);

static void task_wrapper(void *arg){
    task_t *self = scheduler_current();
    self ->entry(arg);
    my_task_exit();
}

static void *port_init_stack(void *stack_top,
                             task_func_t entry,
                             void *arg) {
    uint32_t *sp = (uint32_t *)stack_top;

    *(--sp) = 0x00000020;             // PS: INTLEVEL=0, UM=1
    *(--sp) = 0;                      // SAR
    *(--sp) = 0;                      // a15
    *(--sp) = 0;                      // a14
    *(--sp) = 0;                      // a13
    *(--sp) = 0;                      // a12
    *(--sp) = 0;                      // a11
    *(--sp) = 0;                      // a10
    *(--sp) = 0;                      // a9
    *(--sp) = 0;                      // a8
    *(--sp) = 0;                      // a7
    *(--sp) = 0;                      // a6
    *(--sp) = 0;                      // a5
    *(--sp) = 0;                      // a4
    *(--sp) = 0;                      // a3
    *(--sp) = (uint32_t)arg;          // a2 = аргумент задачи
    *(--sp) = (uint32_t)task_wrapper; // a0 = entry point

    return sp;
}

//linked_list 

void list_init(list_t* list){
    list -> head = NULL;
    list -> tail = NULL;
    list -> count = 0;
}

void list_push_back(list_t* list, void* data){
    list_node_t* node = malloc(sizeof(list_node_t));
    node->data = data;
    node->next = NULL;
    node->prev = list->tail;
    if(list->tail){
        list->tail->next = node;
    } else {
        list->head=node;
        list->count++;
    }
}
void *list_pop_front(list_t* list){
    if(!list->head){
        return NULL;
    }
    list_node_t* node = list->head;
    void* data = node->data;
    list->head = node->next;
    if(list->head) list->head->prev = NULL;
    else list->tail = NULL;
    free(node);
    list->count--;
    return data;
}

void list_remove(list_t* list, void* data){
    list_node_t* node = list->head;
    while(node){
        if(node->data == data){
            if(node->prev) node->prev->next = node->next;
            else list->head = node->next;
            if(node->next) node->next->prev = node->prev;
            else list->tail = node->prev;

            free(node);
            list->count--;
            return;
        }
        node = node->next;
    }
}

bool list_is_empty(const list_t *list){
    return list->count == 0;
}

//scheduler

static void scheduler_pick_next(void){
    if(num_tasks == 0) return;

    uint8_t start = 0;
    for(uint8_t i = 0; i < num_tasks; i++){
        if(tasks[i] == current_task) {start = i; break;}
    }
    uint8_t next = start;
    bool found = false;
    for(uint8_t i = 0; i < num_tasks;++i){
        next = (start + i + 1) % num_tasks;
        if(tasks[next]->state == TASK_READY){
            found = true;
            break;
        }
    }
    if(found){
        if(current_task && current_task->state == TASK_RUNNING){
            current_task -> state = TASK_READY;
        }
        current_task = tasks[next];
        current_task->state = TASK_RUNNING;
    }
}

static void tick_handler(void* arg){
    sys_ticks++;
    for(int i = 0; i < num_tasks;++i){
        if(tasks[i]->state == TASK_BLOCKED && tasks[i]->delay_ticks > 0){
            tasks[i] -> delay_ticks--;
            if(tasks[i] -> delay_ticks == 0){
                tasks[i] -> state = TASK_READY;
            }
        }
    }
}
task_t *scheduler_current(void){
    return current_task;
}

//tasks

void my_rtos_init(void){
    num_tasks = 0; 
    current_task = NULL;
    rtos_running = false;
}