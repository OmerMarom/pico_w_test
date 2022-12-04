
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
    stdio_init_all();
    
    if (cyw43_arch_init()) {
        printf("Wifi init failed.");
        return 1;
    }

    const uint BLINK_INTERVAL = 250; 
    const uint ITERATIONS = 10;

    for (uint i = 0; i < ITERATIONS; i++) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(BLINK_INTERVAL);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
        sleep_ms(BLINK_INTERVAL);
    } 

    return 0;
}
