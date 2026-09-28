#include "rtos.h"

static const char *TAG = "MAIN";

void test_task(void *arg) {
    while (1) {

        my_task_delay(1000);
    }
}

// ВОТ ЭТА ФУНКЦИЯ ДОЛЖНА БЫТЬ ТОЧНО ТАКОЙ:
void app_main(void){
    my_rtos_init();
    my_task_create(test_task, 1, NULL, "test");
    my_rtos_start();
}