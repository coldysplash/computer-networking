#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port> <your_message>" << '\n';
        return -1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(server_ip);
    serverAddress.sin_port = htons(server_port);

    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Error connecting to server" << std::endl;
        close(clientSocket);
        return -1;
    }

    for (size_t i = 0; i < 10; ++i)
    {
        sleep(i); // Задержка в i секунд
        char buffer[MAX_BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "%s", argv[3]);

        // Отправляем данные на сервер
        send(clientSocket, buffer, strlen(buffer), 0);
        std::cout << "Sent: " << buffer << std::endl;
    }

    close(clientSocket);
    return 0;
}