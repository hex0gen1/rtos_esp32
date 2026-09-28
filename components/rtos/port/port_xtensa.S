/*
 * port_xtensa.S — Context switch для ESP32-S3 (Xtensa LX7)
 *
 * Используется CALL0 ABI (без register windows).
 * Все функции RTOS скомпилированы с -mcall0.
 *
 * void port_switch_context(void **old_sp, void *new_sp)
 *   a2 = указатель на old_sp (void**)
 *   a3 = new_sp (void*)
 */

    .section .iram1, "ax"
    .global port_switch_context
    .type   port_switch_context, @function
    .align  4

port_switch_context:
    /* ── Выделяем место на стеке ──
     * 16 регистров × 4 + SAR + PS = 72 байта */
    addi    a1, a1, -72

    /* ── Сохраняем a0 (return address) ── */
    s32i    a0,  a1,  0

    /* ── Сохраняем a2–a15 ── */
    s32i    a2,  a1,  4
    s32i    a3,  a1,  8
    s32i    a4,  a1,  12
    s32i    a5,  a1,  16
    s32i    a6,  a1,  20
    s32i    a7,  a1,  24
    s32i    a8,  a1,  28
    s32i    a9,  a1,  32
    s32i    a10, a1,  36
    s32i    a11, a1,  40
    s32i    a12, a1,  44
    s32i    a13, a1,  48
    s32i    a14, a1,  52
    s32i    a15, a1,  56

    /* ── Сохраняем SAR (shift amount register) ── */
    rsr.sar a4
    s32i    a4,  a1,  60

    /* ── Сохраняем PS (processor status) ── */
    rsr.ps  a4
    s32i    a4,  a1,  64

    /* ── Сохраняем текущий SP в *old_sp ── */
    s32i    a1,  a2,  0

    /* ── Загружаем новый SP ── */
    mov     a1,  a3

    /* ── Восстанавливаем PS ── */
    l32i    a4,  a1,  64
    wsr.ps  a4
    rsync

    /* ── Восстанавливаем SAR ── */
    l32i    a4,  a1,  60
    wsr.sar a4

    /* ── Восстанавливаем a2–a15 ── */
    l32i    a15, a1,  56
    l32i    a14, a1,  52
    l32i    a13, a1,  48
    l32i    a12, a1,  44
    l32i    a11, a1,  40
    l32i    a10, a1,  36
    l32i    a9,  a1,  32
    l32i    a8,  a1,  28
    l32i    a7,  a1,  24
    l32i    a6,  a1,  20
    l32i    a5,  a1,  16
    l32i    a4,  a1,  12
    l32i    a3,  a1,  8
    l32i    a2,  a1,  4

    /* ── Восстанавливаем a0 ── */
    l32i    a0,  a1,  0

    /* ── Освобождаем стек ── */
    addi    a1,  a1,  72

    /* ── Возврат (jx a0) ── */
    ret