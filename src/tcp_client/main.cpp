
#include <set>

#include "tcp_client.h"

static const char* WIFI_SSID = "HOT33";
static const char* WIFI_PASSWORD = "gal0545559402";
static constexpr size_t CONNECT_TIMEOUT = 30'000;

static const char* SERVER_ADDRESS_STR = "192.168.1.15"; 
static constexpr uint16_t SERVER_PORT = 4'242;

static constexpr uint SEND_INTERVAL = 1'000;
static constexpr size_t SEND_SIZE = 100;
static constexpr size_t SEND_COUNT = 10;

static constexpr uint SEND_LED_PIN = 0;
static constexpr uint ERROR_LED_PIN = 1;

void set_led(const uint pin, const bool value) {
    static std::set<uint> init_pins;
    if (!init_pins.contains(pin)) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }

    gpio_put(pin, value);
}

int main() {
    ip_addr_t server_address;
    const int address_valid = ip4addr_aton(SERVER_ADDRESS_STR, &server_address);
    if (!address_valid) {
        printf("Server address %s is not valid.", SERVER_ADDRESS_STR);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    Cyw43Guard cyw43;
    if (cyw43.error) {
        printf("Cyw43 init failed. (%d)", cyw43.error);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    const int wifi_conn_error = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK,
        CONNECT_TIMEOUT
    );
    if (wifi_conn_error) {
        printf("Connection to Wifi failed. (%d)", wifi_conn_error);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    TCPClient client(server_address, SERVER_PORT);

    const err_t server_conn_error = client.connect();
    if (server_conn_error) {
        printf("Connection to server failed. (%d)", server_conn_error);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    err_t error = ERR_OK;
    bool send_led_on = false;
    std::array<uint8_t, SEND_SIZE> send_buffer;
    send_buffer.fill(1);

    for (uint i = 0; i < SEND_COUNT; i++) {
        const err_t send_error = client.send(send_buffer);
        if (send_error) {
            error = send_error;
            break;
        }

        if (client.error) {
            error = client.error;
            break;
        }

        send_led_on = !send_led_on;
        set_led(SEND_LED_PIN, send_led_on);

        sleep_ms(SEND_INTERVAL);
    };

    set_led(SEND_LED_PIN, false);
    
    if (error) {
        printf("TCP client failed. (%d)", server_conn_error);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    return 0;
}
