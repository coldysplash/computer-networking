#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 81

int main()
{
    int sockMain, msgLength;
    unsigned int length;
    struct sockaddr_in servAddr, clientAddr;
    char buf[BUFLEN], sendBuf[BUFLEN], firstMsgBuf[BUFLEN];
    for (size_t i = 0; i != BUFLEN; ++i)
    {
        buf[i] = 0;
        sendBuf[i] = 0;
        firstMsgBuf[i] = 0;
    }

    if ((sockMain = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Сервер не может открыть сокет для UDP");
        exit(1);
    }

    bzero((char *)&servAddr, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = 0;

    if (bind(sockMain, (struct sockaddr *)(&servAddr), sizeof(servAddr)))
    {
        perror("Связывание сервера неудачно.");
        exit(1);
    }

    length = sizeof(servAddr);
    if (getsockname(sockMain, (struct sockaddr *)(&servAddr), &length))
    {
        perror("Вызов getsockname неудачный.");
        exit(1);
    }

    printf("СЕРВЕР: номер порта - %d\n", ntohs(servAddr.sin_port));
    while (1)
    {
        length = sizeof(clientAddr);
        bzero(buf, sizeof(BUFLEN));
        msgLength = recvfrom(
            sockMain,
            buf,
            BUFLEN,
            0,
            (struct sockaddr *)(&clientAddr),
            &length);
        if (msgLength < 0)
        {
            perror("Плохой сокет клиента.");
            exit(1);
        }
        else if (msgLength <= 3)
        {
            for (int i = 0; i != BUFLEN; ++i)
            {
                sendBuf[i] = firstMsgBuf[i];
            }
            strcat(sendBuf, "\n\t\tНомер сообщения: ");
            strcat(sendBuf, buf);
            sendto(sockMain,
                   sendBuf,
                   strlen(sendBuf),
                   0,
                   (const struct sockaddr *)(&clientAddr),
                   sizeof(clientAddr));
        }
        else
        {
            for (int i = 0; i != BUFLEN; ++i)
            {
                firstMsgBuf[i] = buf[i];
            }
        }
        char *addr = malloc(sizeof(char) * BUFLEN);
        for (int i = 0; i != BUFLEN; ++i)
        {
            addr[i] = 0;
        }
        addr = inet_ntoa(clientAddr.sin_addr);

        printf("СЕРВЕР: IP адрес клиента: %s\n", addr);
        printf("СЕРВЕР: ПОРТ клиента: %d\n", ntohs(clientAddr.sin_port));
        printf("СЕРВЕР: Длина сообщения - %d\n", msgLength);
        printf("СЕРВЕР: Сообщение: %s\n\n", buf);
    }
}
