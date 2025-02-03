#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

const char* SERVER_IP = "127.0.0.1";
const int PORT = 9100;

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "No se pudo conectar al servicio." << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Conexion establecida con el servicio." << std::endl;
    std::cout << "Introduzca su pedido: ";
    std::string pedido;
    std::getline(std::cin, pedido);

    send(client_socket, pedido.c_str(), pedido.length(), 0);

    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Identificador recibido: " << buffer << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
