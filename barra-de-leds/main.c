#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

#define BTN_PIN 22
#define SW_PIN  28

const uint8_t BAR_PINS[5] = {2, 3, 4, 5, 6};

volatile int contador = 0;

void bar_init(void) {
    for (int i = 0; i < 5; i++) {
        gpio_init(BAR_PINS[i]);
        gpio_set_dir(BAR_PINS[i], GPIO_OUT);
        gpio_put(BAR_PINS[i], 0);
    }
}

void bar_display(int val) {
    if (val < 0) val = 0;
    if (val > 5) val = 5;

    for (int i = 0; i < 5; i++) {
        gpio_put(BAR_PINS[i], i < val);
    }
}

static inline int read_sw_level(void) {
    return (sio_hw->gpio_in >> SW_PIN) & 1u;
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    if (gpio != BTN_PIN) return;
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;

    int sw = read_sw_level();

    if (sw == 0) {
        if (contador < 5) contador++;
    } else {
        if (contador > 0) contador--;
    }

    bar_display(contador);
}

int main(void) {
    stdio_init_all();
    bar_init();

    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    bar_display(contador);

    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        tight_loop_contents();
    }
}