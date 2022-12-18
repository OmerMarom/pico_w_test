
#include "pico/stdlib.h"

int main() {
    const uint BUTTON_PIN = 0;
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    const uint LED_PIN = 1;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    bool led_on = false;

    while (true) {
        const bool button_pressed = !gpio_get(BUTTON_PIN);

        if (button_pressed) {
            led_on = !led_on;
            gpio_put(LED_PIN, led_on);
            const uint PRESS_TIMEOUT = 180;
            sleep_ms(PRESS_TIMEOUT);
        }
    }

    return 0;
}
