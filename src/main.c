#include "debug_info.h"
#include "dns_relay_server.h"
int main() {
    remote_dns = "10.3.9.5";
    initialize_winsock();
    server_sock = create_socket();
    while (1) {
        receive_from_client();
        receive_from_server();
    }
}