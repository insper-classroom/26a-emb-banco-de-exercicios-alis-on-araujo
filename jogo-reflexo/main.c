#include <stdio.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#define BTN_G 28
#define BTN_R 20
#define BTN_Y 26

#define LED_G 5
#define LED_Y 9
#define LED_R 13

#define STEP_MS 300

typedef enum {
    COLOR_Y = 0,
    COLOR_G = 1,
    COLOR_R = 2
} color_t;

typedef enum {
    IDLE = 0,
    SHOWING,
    INPUT,
    OVER
} game_state_t;

static const color_t sequence[10] = {
    COLOR_Y, COLOR_G, COLOR_R, COLOR_Y, COLOR_G,
    COLOR_Y, COLOR_R, COLOR_Y, COLOR_G, COLOR_Y
};

volatile game_state_t state = IDLE;
volatile int round_len = 0;      // rodada atual: 1..10
volatile int pnts = 0;           // pontos finais
volatile int display_pos = 0;    // posição na sequência mostrada
volatile int display_phase = 0;   // 0 = LED ON, 1 = LED OFF
volatile int input_pos = 0;      // posição esperada da resposta
volatile bool score_printed = false;

static inline void all_leds_off(void) {
    gpio_put(LED_G, 0);
    gpio_put(LED_Y, 0);
    gpio_put(LED_R, 0);
}

static inline void show_color(color_t c) {
    all_leds_off();
    if (c == COLOR_G) gpio_put(LED_G, 1);
    if (c == COLOR_Y) gpio_put(LED_Y, 1);
    if (c == COLOR_R) gpio_put(LED_R, 1);
}

static inline color_t gpio_to_color(uint gpio) {
    if (gpio == BTN_Y) return COLOR_Y;
    if (gpio == BTN_G) return COLOR_G;
    return COLOR_R;
}

static inline bool button_matches_color(uint gpio, color_t c) {
    return (gpio == BTN_Y && c == COLOR_Y) ||
           (gpio == BTN_G && c == COLOR_G) ||
           (gpio == BTN_R && c == COLOR_R);
}

static void start_round(void) {
    input_pos = 0;
    display_pos = 0;
    display_phase = 0;
    state = SHOWING;
    show_color(sequence[0]);
}

static void finish_game(void) {
    state = OVER;
    all_leds_off();
}

static void advance_round_or_finish(void) {
    pnts = round_len;

    if (round_len >= 10) {
        finish_game();
    } else {
        round_len++;
        start_round();
    }
}

bool sequence_timer_cb(struct repeating_timer *t) {
    (void)t;

    if (state != SHOWING) {
        return true;
    }

    if (display_phase == 0) {
        all_leds_off();
        display_phase = 1;
    } else {
        display_pos++;
        if (display_pos >= round_len) {
            all_leds_off();
            state = INPUT;
        } else {
            show_color(sequence[display_pos]);
            display_phase = 0;
        }
    }

    return true;
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;

    if (state == IDLE) {
        if (gpio == BTN_G) {
            round_len = 1;
            pnts = 0;
            start_round();
        }
        return;
    }

    if (state != INPUT) return;

    color_t expected = sequence[input_pos];

    if (button_matches_color(gpio, expected)) {
        input_pos++;
        if (input_pos >= round_len) {
            advance_round_or_finish();
        }
    } else {
        pnts = round_len - 1;
        finish_game();
    }
}

int main(void) {
    stdio_init_all();

    gpio_init(LED_G); gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_Y); gpio_set_dir(LED_Y, GPIO_OUT);
    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);
    all_leds_off();

    gpio_init(BTN_G); gpio_set_dir(BTN_G, GPIO_IN); gpio_pull_up(BTN_G);
    gpio_init(BTN_R); gpio_set_dir(BTN_R, GPIO_IN); gpio_pull_up(BTN_R);
    gpio_init(BTN_Y); gpio_set_dir(BTN_Y, GPIO_IN); gpio_pull_up(BTN_Y);

    gpio_set_irq_enabled_with_callback(BTN_G, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BTN_R, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_Y, GPIO_IRQ_EDGE_FALL, true);

    struct repeating_timer timer;
    add_repeating_timer_ms(STEP_MS, sequence_timer_cb, NULL, &timer);

    while (true) {
        if (state == OVER && !score_printed) {
            printf("Points %d\n", pnts);
            fflush(stdout);
            score_printed = true;
        }
        tight_loop_contents();
    }
}