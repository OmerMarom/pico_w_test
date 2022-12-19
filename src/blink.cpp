
#include "pico/cyw43_arch.h"

int main() {
    if (cyw43_arch_init()) {
        return 1;
    }

    bool led_on = false;

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
        led_on = !led_on;
        static constexpr uint BLINK_INTERVAL = 250; 
        sleep_ms(BLINK_INTERVAL);
    } 

    return 0;
}
