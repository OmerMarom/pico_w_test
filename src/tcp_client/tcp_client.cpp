
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include <array>

// Settings:
static const char* REMOTE_ADDRESS_STR = "192.168.1.15"; 
static constexpr uint16_t SERVER_PORT = 4242;

static const char* WIFI_SSID = "HOT33";
static const char* WIFI_PASSWORD = "gal0545559402";
static constexpr size_t CONNECT_TIMEOUT = 30'000;

static constexpr size_t SEND_SIZE = 100;
static constexpr size_t SEND_COUNT = 10;
static constexpr uint SEND_INTERVAL = 1'000;

static constexpr uint SENT_LED_PIN = 0;
static constexpr uint ERROR_LED_PIN = 1;

static void set_led(const uint pin, const bool value = true) {
    // Statically initialize pins: (Happens once a program)
    static bool initialized = false;
    if (!initialized) {
        gpio_init(SENT_LED_PIN);
        gpio_init(ERROR_LED_PIN);
        gpio_set_dir(SENT_LED_PIN, GPIO_OUT);
        gpio_set_dir(ERROR_LED_PIN, GPIO_OUT);
        initialized = true;
    }

    gpio_put(pin, value);
}

struct Client {
    ip_addr_t m_remote_address;
    tcp_pcb* m_tcp_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);

    std::array<uint8_t, SEND_SIZE> m_buffer;
    
    size_t send_count = 0;
    err_t m_error = ERR_OK;

    Client(const ip_addr_t remote_address) :
        m_remote_address(remote_address) {
        m_buffer.fill(1);

        // Event listeners:
        tcp_arg(m_tcp_pcb, this);
        tcp_sent(m_tcp_pcb, on_sent);
        tcp_err(m_tcp_pcb, on_error);
    }

    ~Client() {
        if (!m_tcp_pcb) {
            return;
        }

        // Cleanup listeners:
        tcp_arg(m_tcp_pcb, nullptr);
        tcp_sent(m_tcp_pcb, nullptr);
        tcp_err(m_tcp_pcb, nullptr);

        // Close socket:        
        const err_t close_error = tcp_close(m_tcp_pcb);
        if (close_error) {
            tcp_abort(m_tcp_pcb);
        }
    }

    // cyw43_arch_lwip_begin/end RAII class:
    struct Cyw43LockGuard {
        Cyw43LockGuard() { cyw43_arch_lwip_begin(); }
        ~Cyw43LockGuard() { cyw43_arch_lwip_end(); }
    };

    err_t connect() {
        Cyw43LockGuard g;
        const err_t connect_error = tcp_connect(
            m_tcp_pcb,
            &m_remote_address,
            SERVER_PORT,
            &Client::on_connected
        );

        return connect_error;
    }

    err_t send() {
        Cyw43LockGuard g;

        // Write to socket buffer:
        const err_t write_error = tcp_write(
            m_tcp_pcb,
            m_buffer.data(), 
            m_buffer.size(),
            0
        );
        if (write_error) {
            m_error = write_error;
            return write_error;
        }

        // Output socket buffer to server:
        const err_t output_error = tcp_output(m_tcp_pcb);
        if (output_error) {
            m_error = output_error;
            return output_error;
        }

        return ERR_OK;
    }

    static err_t on_connected(void* arg, tcp_pcb* _tcp_pcb, const err_t error) {
        if (error) {
            auto* client = (Client*)arg;
            client->m_error = error;
        }

        return ERR_OK;
    }

    static err_t on_sent(void* arg, tcp_pcb* _tcp_pcb, const u16_t _sent_size) {
        auto* client = (Client*)arg;
        client->send_count++;
        if (client->send_count >= SEND_COUNT) {
            return client->m_error = ERR_ABRT;
        }

        static bool sent_on = true; 
        set_led(SENT_LED_PIN, sent_on);
        sent_on = !sent_on;

        return ERR_OK;
    }

    static void on_error(void* arg, const err_t error) {
        auto* client = (Client*)arg;
        client->m_error = error;
    }
};

static err_t start_client() {
    // Connect to Wifi:
    const int connect_failed = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK,
        CONNECT_TIMEOUT
    );
    if (connect_failed) {
        return connect_failed;
    }

    // Create TCP client:
    ip_addr_t remote_address;
    const int address_valid = ip4addr_aton(REMOTE_ADDRESS_STR, &remote_address);
    if (!address_valid) {
        return ERR_ARG;
    }
    Client client(remote_address);

    // Connect to server:
    const err_t connect_error = client.connect();
    if (connect_error) {
        return connect_error;
    }

    // Send data to server:
    while (true) {
        if (client.m_error) {
            return client.m_error;
        }

        const err_t send_error = client.send();
        if (send_error) {
            return send_error;
        }

        sleep_ms(SEND_INTERVAL);
    };
}

int main() {
    // Init Wifi cheap:
    const int init_failed = cyw43_arch_init(); 
    if (init_failed) {
        set_led(ERROR_LED_PIN);
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    
    // TCP client:
    const err_t client_error = start_client();

    // Cleanup:
    cyw43_arch_deinit();

    if (client_error && client_error != ERR_ABRT) {
        set_led(ERROR_LED_PIN);
        return 1;
    }

    return 0;
}
