#include "pico/stdlib.h"
#include "hardware/timer.h"

#define BUZZER_PIN 21

#define FREQ_MAX 2000
#define FREQ_MIN  200
#define STEP      20

#define SWEEP_MS  20

volatile int freq = FREQ_MAX;
volatile bool buzzer_state = false;
alarm_id_t buzzer_alarm_id;

static inline uint32_t half_period_us(int f) {
    return 500000u / (uint32_t)f;
}

int64_t buzzer_toggle_callback(alarm_id_t id, void *user_data) {
    (void)id;
    (void)user_data;

    buzzer_state = !buzzer_state;
    gpio_put(BUZZER_PIN, buzzer_state);

    return (int64_t)half_period_us(freq);
}

bool sweep_callback(struct repeating_timer *t) {
    (void)t;

    freq -= STEP;
    if (freq < FREQ_MIN) {
        freq = FREQ_MAX;
    }

    return true;
}

int main() {
    stdio_init_all();

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    // começa a gerar o som
    buzzer_alarm_id = add_alarm_in_us(
        half_period_us(freq),
        buzzer_toggle_callback,
        NULL,
        true
    );

    // controla a queda da frequência
    static struct repeating_timer sweep_timer;
    add_repeating_timer_ms(SWEEP_MS, sweep_callback, NULL, &sweep_timer);

    while (true) {
        tight_loop_contents();
    }
}