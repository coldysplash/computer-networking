#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 1024

struct Info
{
    int client_socket;
    char *ip;
    uint16_t port;
};

pthread_mutex_t lock;

void *handleClient(void *arg)
{
    Info *info = static_cast<Info *>(arg);
    int clientSocket = info->client_socket;
    std::string client_info = "Client Port: ";
    client_info.append(std::to_string(info->port));
    delete info;

    char buffer[MAX_BUFFER_SIZE];
    std::ofstream outputFile("sharedData.txt", std::ios::app);

    while (true)
    {
        pthread_mutex_init(&lock, NULL);

        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            break;
        }
        buffer[bytesRead] = '\0';
        outputFile << client_info << " Recv MSG: " << buffer << '\n';

        std::cout << "Received : " << buffer << std::endl;

        pthread_mutex_destroy(&lock);
    }

    close(clientSocket);
    outputFile.close();
    return nullptr;
}

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = 0; // 0 для автоматического выбора свободного порта

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    // Получаем номер свободного порта
    socklen_t addressLength = sizeof(serverAddress);
    getsockname(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), &addressLength);
    std::cout << "Server listening on port: " << ntohs(serverAddress.sin_port) << std::endl;

    if (listen(serverSocket, 10) == -1)
    {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    while (true)
    {
        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress),
                                  &clientAddressLength);

        if (clientSocket == -1)
        {
            std::cerr << "Error accepting connection" << std::endl;
            close(serverSocket);
            return -1;
        }

        // Создаем новый поток для обработки клиента
        pthread_t thread;
        Info *data = new Info{sizeof(Info)};
        data->ip = inet_ntoa(clientAddress.sin_addr);
        data->port = ntohs(clientAddress.sin_port);
        data->client_socket = clientSocket;
        if (pthread_create(&thread, nullptr, handleClient, data) != 0)
        {
            std::cerr << "Error creating thread" << std::endl;
            close(clientSocket);
            delete data;
        }
    }

    close(serverSocket);
    return 0;
}