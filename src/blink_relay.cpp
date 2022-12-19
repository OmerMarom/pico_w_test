
#include "pico/stdlib.h"

int main() {
    static constexpr uint RELAY_SET_PIN = 0;
    gpio_init(RELAY_SET_PIN);
    gpio_set_dir(RELAY_SET_PIN, GPIO_OUT);

    bool relay_set = false;

    while (true) {
        gpio_put(RELAY_SET_PIN, relay_set);
        relay_set = !relay_set;
        static constexpr uint BLINK_INTERVAL = 250; 
        sleep_ms(BLINK_INTERVAL);
    }

    return 0;
}
