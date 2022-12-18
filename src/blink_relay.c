
#include "pico/stdlib.h"

int main() {
    const uint RELAY_SET_PIN = 0;
    gpio_init(RELAY_SET_PIN);
    gpio_set_dir(RELAY_SET_PIN, GPIO_OUT);
    gpio_put(RELAY_SET_PIN, true);

    const uint LED_PIN = 1;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);

    const uint RELAY_POWER_PIN = 0;
    gpio_init(RELAY_POWER_PIN);
    gpio_set_dir(RELAY_POWER_PIN, GPIO_OUT);

    bool relay_set = false;

    while (true) {
        gpio_put(RELAY_POWER_PIN, relay_set);
        relay_set = !relay_set;
        const uint BLINK_INTERVAL = 250; 
        sleep_ms(BLINK_INTERVAL);
    }

    return 0;
}
