#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"

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

typedef struct {
    volatile game_state_t state;
    volatile int round_len;
    volatile int pnts;
    volatile int display_pos;
    volatile int display_phase;
    volatile int input_pos;
    volatile bool score_printed;
} game_t;

static game_t *game(void) {
    static game_t g = {
        .state = IDLE,
        .round_len = 0,
        .pnts = 0,
        .display_pos = 0,
        .display_phase = 0,
        .input_pos = 0,
        .score_printed = false
    };
    return &g;
}

static const color_t *sequence(void) {
    static const color_t seq[10] = {
        COLOR_Y, COLOR_G, COLOR_R, COLOR_Y, COLOR_G,
        COLOR_Y, COLOR_R, COLOR_Y, COLOR_G, COLOR_Y
    };
    return seq;
}

static inline void all_leds_off(void) {
    gpio_put(LED_G, 0);
    gpio_put(LED_Y, 0);
    gpio_put(LED_R, 0);
}

static inline void show_color(color_t c) {
    all_leds_off();
    if (c == COLOR_G) gpio_put(LED_G, 1);
    else if (c == COLOR_Y) gpio_put(LED_Y, 1);
    else if (c == COLOR_R) gpio_put(LED_R, 1);
}

static inline bool button_matches_color(uint gpio, color_t c) {
    return (gpio == BTN_Y && c == COLOR_Y) ||
           (gpio == BTN_G && c == COLOR_G) ||
           (gpio == BTN_R && c == COLOR_R);
}

static inline color_t gpio_to_color(uint gpio) {
    if (gpio == BTN_Y) return COLOR_Y;
    if (gpio == BTN_G) return COLOR_G;
    return COLOR_R;
}

static void start_round(void) {
    game_t *g = game();
    const color_t *seq = sequence();

    g->input_pos = 0;
    g->display_pos = 0;
    g->display_phase = 1;
    g->state = SHOWING;

    show_color(seq[0]);
}

static void finish_game(void) {
    game_t *g = game();
    g->state = OVER;
    all_leds_off();
}

static void complete_round(void) {
    game_t *g = game();

    g->pnts = g->round_len;

    if (g->round_len >= 10) {
        finish_game();
        return;
    }

    g->round_len++;
    start_round();
}

bool sequence_timer_cb(struct repeating_timer *t) {
    (void)t;

    game_t *g = game();
    const color_t *seq = sequence();

    if (g->state != SHOWING) {
        return true;
    }

    if (g->display_phase == 1) {
        all_leds_off();
        g->display_phase = 0;

        if (g->display_pos + 1 >= g->round_len) {
            g->state = INPUT;
        } else {
            g->display_pos++;
        }
    } else {
        show_color(seq[g->display_pos]);
        g->display_phase = 1;
    }

    return true;
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;

    game_t *g = game();

    if (g->state == IDLE) {
        if (gpio == BTN_G) {
            g->round_len = 1;
            g->pnts = 0;
            g->score_printed = false;
            start_round();
        }
        return;
    }

    if (g->state != INPUT) return;

    const color_t *seq = sequence();
    color_t expected = seq[g->input_pos];

    if (button_matches_color(gpio, expected)) {
        g->input_pos++;

        if (g->input_pos >= g->round_len) {
            complete_round();
        }
    } else {
        g->pnts = (g->round_len > 0) ? (g->round_len - 1) : 0;
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
        game_t *g = game();

        if (g->state == OVER && !g->score_printed) {
            printf("Points %d\n", g->pnts);
            fflush(stdout);
            g->score_printed = true;
        }

        tight_loop_contents();
    }
}