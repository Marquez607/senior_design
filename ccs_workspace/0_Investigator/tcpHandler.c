/*
 * Name: tcpHandler
 * Author: Marquez Jones
 * Desc: all tcp handling for clietn
 */

/*
 *    ======== udpEcho.c ========
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
#include <ti/sysbios/knl/Task.h>

/* investigator includes */
#include "shared.h"
#include "pingpong_ipc.h"

extern Display_Handle display;

/*
 * Name: camTCP
 * Desc: transmits camera data to client
 */
void camTCP(uint32_t arg0, uint32_t arg1)
{
    int                bytesSent = 0;
    int                status;
    int                server;
    int                client;
    struct sockaddr_in localAddr;
    socklen_t          addrlen;
    int                optval;
    int                optlen = sizeof(optval);

    server = socket(AF_INET,  SOCK_STREAM, 0);
    if (server == -1) {
        //Display_printf(display, 0, 0, "Error: socket not created.\n");
        goto shutdown;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        //Display_printf(display, 0, 0, "Error: bind failed.\n");
        goto shutdown;
    }

    status = listen(server, 3);
    if (status == -1) {
        //Display_printf(display, 0, 0, "Error: listen failed.\n");
        goto shutdown;
    }

    optval = 1;
    status = setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
    if (status == -1) {
        //Display_printf(display, 0, 0, "tcpHandler: setsockopt failed\n");
        goto shutdown;
    }

    ppipc_buff_t *cam_buffer;
    while(1){

        addrlen = sizeof(localAddr);
        client = accept(server,(struct sockaddr *)&localAddr,(socklen_t)&addrlen);
        //Display_printf(display, 0, 0, "Camera Client Connected.\n");

        while ( bytesSent >= 0 ) { /* while tcp connection is up */
            cam_buffer = ppipc_get_buffer(&cam_buff_tab,PPIPC_READ); /* get buffer */
            bytesSent = send(client, cam_buffer->data, cam_buffer->size, 0);
            ppipc_update_buffer(&cam_buff_tab,PPIPC_READ); /* post buffer update */
            //Task_sleep(10);
        }
        bytesSent = 0;
        //Display_printf(display, 0, 0, "Camera Client Dropped\n");
        Task_sleep(1);
    }

shutdown:
    if (server != -1) {
        close(server);
    }
}

/*
 * Name: updateTCP
 * Desc: handler for transmitting status updates
 *       to client
 */
void updateTCP(uint32_t arg0, uint32_t arg1){
    int                bytesSent = 0;
    int                status;
    int                server;
    int                client;
    struct sockaddr_in localAddr;
    socklen_t          addrlen;
    int                optval;
    int                optlen = sizeof(optval);

    pdu_t out_pdu;
    uint8_t out_buff[PDU_SIZE+2];


    server = socket(AF_INET,  SOCK_STREAM, 0);
    if (server == -1) {
        //Display_printf(display, 0, 0, "Error: socket not created.\n");
        goto shutdown;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        //Display_printf(display, 0, 0, "Error: bind failed.\n");
        goto shutdown;
    }

    status = listen(server, 3);
    if (status == -1) {
        //Display_printf(display, 0, 0, "Error: listen failed.\n");
        goto shutdown;
    }

    optval = 1;
    status = setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
    if (status == -1) {
        //Display_printf(display, 0, 0, "tcpHandler: setsockopt failed\n");
        goto shutdown;
    }

    while(1){

        addrlen = sizeof(localAddr);
        client = accept(server,(struct sockaddr *)&localAddr,(socklen_t)&addrlen);
        Display_printf(display, 0, 0, "Update Client Connected.\n");

        while ( bytesSent >= 0 ) { /* while tcp connection is up */

            /* read pdu from fifo */
            pdu_fifo_get(&tx_pdu_fifo_g,&out_pdu);

            /* copy to buffer */
            pdu_fill_buffer(out_buff,&out_pdu);

            /* send buffer */
            bytesSent = send(client, out_buff, PDU_SIZE+2, 0);

        }
        bytesSent = 0;
        Display_printf(display, 0, 0, "Update Client Dropped\n");
        Task_sleep(1);
    }

    shutdown:
    if (server != -1) {
        close(server);
    }
}

/*
 * Name: commandTCP
 * Desc: handlers for receiving commands
 *       from the client
 */
void commandTCP(uint32_t arg0, uint32_t arg1){
     int                bytesRcvd;
     int                status;
     int                server;
     int                client;
     struct sockaddr_in localAddr;
     socklen_t          addrlen;
     int                optval;
     int                optlen = sizeof(optval);

     pdu_t in_pdu;
     uint8_t in_buff[PDU_SIZE+2];

     server = socket(AF_INET,  SOCK_STREAM, 0);
     if (server == -1) {
         //Display_printf(display, 0, 0, "Error: socket not created.\n");
         goto shutdown;
     }

     memset(&localAddr, 0, sizeof(localAddr));
     localAddr.sin_family = AF_INET;
     localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
     localAddr.sin_port = htons(arg0);

     status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
     if (status == -1) {
         //Display_printf(display, 0, 0, "Error: bind failed.\n");
         goto shutdown;
     }

     status = listen(server, 3);
     if (status == -1) {
         //Display_printf(display, 0, 0, "Error: listen failed.\n");
         goto shutdown;
     }

     optval = 1;
     status = setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
     if (status == -1) {
         //Display_printf(display, 0, 0, "tcpHandler: setsockopt failed\n");
         goto shutdown;
     }

     while(1){

         addrlen = sizeof(localAddr);
         client = accept(server,(struct sockaddr *)&localAddr,(socklen_t)&addrlen);
         Display_printf(display, 0, 0, "Cmd Client Connected.\n");

         while( ( bytesRcvd = recv(client, in_buff, PDU_SIZE+2, 0) ) > 0){

             /* convert to pdu struct */
             pdu_read_buffer(in_buff,&in_pdu);

             /* write to fifo */
//             Display_printf(display, 0, 0, "FIFO PUT\n");
             pdu_fifo_put(&rx_pdu_fifo_g,&in_pdu);
//             Display_printf(display, 0, 0, "FIFO PUT FINISHED\n");

         }
         bytesRcvd = 0;
         Display_printf(display, 0, 0, "Cmd Client Dropped\n");
         Task_sleep(1);
     }

    shutdown:
     if (server != -1) {
         close(server);
     }
}


