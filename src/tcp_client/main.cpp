
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include <set>
#include <atomic>
#include <functional>
#include <string>

// #include "tcp_client.h"

// static const char *WIFI_SSID = "HOT33";
static const char *WIFI_SSID = "OmerMarom";
// static const char* WIFI_PASSWORD = "gal0545559402";
static const char *WIFI_PASSWORD = "bruh12345";
static constexpr size_t CONNECT_TIMEOUT = 30'000;

// static const char *SERVER_ADDRESS_STR = "192.168.1.15";
static const char *SERVER_ADDRESS_STR = "192.168.158.205";
static constexpr uint16_t SERVER_PORT = 4'242;

static constexpr uint SEND_INTERVAL = 1'000;
static constexpr size_t SEND_SIZE = 10;
static constexpr size_t SEND_COUNT = 10;

static constexpr uint SEND_LED_PIN = 0;
static constexpr uint WIFI_LED_PIN = 1;
static constexpr uint ERROR_LED_PIN = 2;

static void set_led(const uint pin, const bool value) {
    static std::set<uint> init_pins;
    if (!init_pins.contains(pin)) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }

    gpio_put(pin, value);
}

struct TCPClient {
    tcp_pcb *m_tcp;
    const ip_addr_t m_server_address;
    const uint16_t m_server_port;
    std::function<void(err_t)> m_on_error;
    std::function<err_t()> m_on_connected;
    std::function<err_t()> m_on_sent;

    TCPClient(
        const ip_addr_t server_address,
        const uint16_t server_port,
        std::function<void(err_t)> error_callback
    ) : m_tcp(tcp_new_ip_type(IPADDR_TYPE_V4)),
        m_server_address(server_address),
        m_server_port(server_port),
        m_on_error(std::move(error_callback)) {
        tcp_arg(m_tcp, this);
        tcp_err(m_tcp, &TCPClient::on_error);
        tcp_sent(m_tcp, &TCPClient::on_sent);
    }

    ~TCPClient() {
        if (m_tcp) {
            const err_t error = tcp_close(m_tcp);
            if (error) {
                tcp_abort(m_tcp);
            }
        }
    }

    err_t connect(std::function<err_t()> callback) {
        m_on_connected = std::move(callback);

        const err_t error = tcp_connect(
            m_tcp,
            &m_server_address,
            m_server_port,
            &TCPClient::on_connected
        );

        return error;
    }

    err_t send(std::function<err_t()> callback) {
        m_on_sent = std::move(callback);

        static std::string MESSAGE = "Hello world!";
        const err_t error = tcp_write(
            m_tcp,
            MESSAGE.data(),
            MESSAGE.size(),
            0 // Api flags
        );

        return error;
    }

    static void on_error(void *arg, err_t error) {
        auto* self = (TCPClient*)arg;
        self->m_on_error(error);
    }

    static err_t on_connected(void* arg, tcp_pcb* _tcp, const err_t error) {
        if (error) {
            return error;
        }

        auto *self = (TCPClient*)arg;
        return self->m_on_connected();
    }

    static err_t on_sent(void* arg, tcp_pcb* _tcp, const u16_t _sent_size) {
        auto *self = (TCPClient *)arg;
        return self->m_on_sent();
    }
};

struct Cyw43Guard {
    int error;
    Cyw43Guard() { 
        error = cyw43_arch_init(); 
        if (!error) {
            cyw43_arch_enable_sta_mode();
        }
    }
    ~Cyw43Guard() {
        if (!error) {
            cyw43_arch_deinit();
        }
    }
};

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

    set_led(WIFI_LED_PIN, true);

    std::atomic<err_t> client_error = ERR_OK;
    std::atomic<bool> completed = false;

    TCPClient client(
        server_address, SERVER_PORT,
        [&](const err_t error) {
            client_error = error;
            completed = true;
        }
    );

    bool send_led_on = true;
    size_t sent_count = 0;
    // Explicit type declaration necessary for recursive call:
    std::function<err_t()> send_data = [&] {
        if (sent_count == SEND_COUNT) {
            completed = true;
            return (err_t)ERR_OK;
        }

        set_led(SEND_LED_PIN, send_led_on);

        send_led_on = !send_led_on;
        sent_count++;

        return client.send([&] {
            // sleep_ms(1);
            return send_data();
        });
    };

    // We have to copy send_data (not move) because the recursive call 
    // references the local send_data.
    const err_t conn_args_error = client.connect(send_data);
    if (conn_args_error) {
        printf("Invalid connection args. (%d)", conn_args_error);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    // TODO This is probably not the best way to do this. 
    static constexpr uint32_t CHECK_INTERVAL = 1000;
    while (!completed) sleep_ms(CHECK_INTERVAL);

    if (client_error) {
        printf("TCP client failed. (%d)", (err_t)client_error);
        set_led(ERROR_LED_PIN, true);
        return 1;
    }

    return 0;
}
