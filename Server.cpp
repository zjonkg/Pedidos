#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <regex>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 9100;
std::mutex file_mutex;
int order_count = 1;

void load_last_order_id() {
    std::ifstream file("pedidos.txt");
    std::string line;
    std::regex order_regex("Order-(\\d+):");
    std::smatch match;
    int max_order_id = 0;

    while (std::getline(file, line)) {
        if (std::regex_search(line, match, order_regex)) {
            int current_id = std::stoi(match[1]);
            if (current_id > max_order_id) {
                max_order_id = current_id;
            }
        }
    }
    order_count = max_order_id + 1;
}

void handle_client(SOCKET client_socket) {
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::string pedido(buffer);

        // Generar identificador único
        std::string order_id;
        {
            std::lock_guard<std::mutex> lock(file_mutex);
            order_id = "Order-" + std::to_string(order_count++);
        }

        // Guardar en el archivo
        {
            std::lock_guard<std::mutex> lock(file_mutex);
            std::ofstream file("pedidos.txt", std::ios::app);
            if (file.is_open()) {
                file << order_id << ": \"" << pedido << "\"\n";
            }
        }

        // Enviar confirmación al cliente
        send(client_socket, order_id.c_str(), order_id.length(), 0);
    }
    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    load_last_order_id();

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    std::cout << "Servicio iniciado. Escuchando en el puerto " << PORT << "..." << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket != INVALID_SOCKET) {
            std::cout << "Orden añadida perfectamente." << std::endl;
            std::thread(handle_client, client_socket).detach();
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
