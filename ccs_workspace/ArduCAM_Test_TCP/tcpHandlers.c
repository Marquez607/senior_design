/*
 * Name: tcpHandlers
 * Desc: this file contains all tcpHandlers used in
 *       application
 *
 * NOTE: this application assumes only one client per thread
 *
 * HANDLERS LIST:
 * tcpCamera
 * tcpLocation
 * tcpCommands
 */

/*
 *    ======== tcpHandlers.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
/* BSD support */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <ti/net/slnetutils.h>

#include <ti/display/Display.h>

#define TX_SIZE 64
#define RX_SIZE 64

extern Display_Handle display;

extern void *TaskCreate(void (*pFun)(), char *Name, int Priority,
        uint32_t StackSize, uintptr_t Arg1, uintptr_t Arg2, uintptr_t Arg3);


/***********************************************
 * Name: tcpCamera
 * Desc: this thread will be assigned
 *       to the camera port and deal
 *       with sending camera feed to the
 *       client
 ***********************************************/
void tcpCamera(uint32_t arg0, uint32_t arg1)
{
    int                bytesRcvd;
    int                bytesSent;
    int                status;
    int                server;
    int                client;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    int                optval;
    int                optlen = sizeof(optval);
    char               tx_buffer[TX_SIZE];
    char               rx_buffer[RX_SIZE];
    uint32_t           max_ticks = 40E3; //wait 40 thousand cpu ticks


    server = socket(AF_INET,  SOCK_STREAM, 0);
    if (server == -1) {

        goto shutdown;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {

        goto shutdown;
    }

    status = listen(server, 1);
    if (status == -1) {

        goto shutdown;
    }

    optval = 1;
    status = setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
    if (status == -1) {

        goto shutdown;
    }

//wait_client:
    while(1){
        addrlen = sizeof(localAddr);
        client = accept(server,(struct sockaddr *)&localAddr,(socklen_t)&addrlen);


        /* if bytes can't be sent then client probably dropped */
        while ( bytesSent >=0 ) {

            sprintf(tx_buffer,"Hello From Server");
            bytesSent = send(client, tx_buffer, TX_SIZE, 0);

        }

    }

shutdown:
    if (server != -1) {
        close(server);
    }
}
