#pragma one

#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include <array>

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

struct LwipLockGuard {
    LwipLockGuard() { cyw43_arch_lwip_begin(); }
    ~LwipLockGuard() { cyw43_arch_lwip_end(); }
};

struct TCPClient {
    TCPClient(const ip_addr_t server_address, const uint16_t server_port) :
        m_server_address(server_address),
        m_server_port(server_port) {        
    }

    ~TCPClient() {
        if (m_tcp_pcb) {
            const err_t close_error = tcp_close(m_tcp_pcb);
            if (close_error) {
                tcp_abort(m_tcp_pcb);
            }
        }
    }

    err_t connect() {
        tcp_arg(m_tcp_pcb, this);
        tcp_err(m_tcp_pcb, on_error);

        LwipLockGuard g;
        const err_t connect_error = tcp_connect(
            m_tcp_pcb,
            &m_server_address,
            m_server_port,
            &TCPClient::on_connected
        );

        return connect_error;
    }

    template <typename Tdata>
    err_t send(const Tdata& data) {
        if (data.size() > BUFFER_SIZE) {
            return ERR_ARG;
        }
        std::copy(data.begin(), data.end(), m_buffer.begin());

        LwipLockGuard g;

        const err_t write_error = tcp_write(
            m_tcp_pcb,
            m_buffer.data(), 
            data.size(),
            0
        );
        if (write_error) {
            return write_error;
        }

        const err_t output_error = tcp_output(m_tcp_pcb);
        if (output_error) {
            return output_error;
        }

        return ERR_OK;
    }

    static void on_error(void* arg, const err_t error) {
        auto* self = (TCPClient*)arg;
        self->error = error;
    }

    static err_t on_connected(void* arg, tcp_pcb* _tcp_pcb, const err_t error) {
        auto* self = (TCPClient*)arg;
        return self->error = error;
    }

    err_t error = ERR_OK;

private:
    ip_addr_t m_server_address;
    uint16_t m_server_port;
    tcp_pcb* m_tcp_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    static constexpr size_t BUFFER_SIZE = 5'000;
    std::array<uint8_t, BUFFER_SIZE> m_buffer;
};
