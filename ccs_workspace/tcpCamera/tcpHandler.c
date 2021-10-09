/*
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#define TX_SIZE 64
#define RX_SIZE 64

extern Display_Handle display;

/*
 *  ======== echoFxn ========
 *  Echoes UDP messages.
 *
 */
void camTCP(uint32_t arg0, uint32_t arg1)
{
//    int                bytesRcvd;
    int                bytesSent = 0;
    int                status;
    int                server;
    int                client;
    struct sockaddr_in localAddr;
//    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    int                optval;
    int                optlen = sizeof(optval);
//    char               rx_buffer[RX_SIZE];

    Display_printf(display, 0, 0, "TCP Hello Test Started\n");

    server = socket(AF_INET,  SOCK_STREAM, 0);
    if (server == -1) {
        Display_printf(display, 0, 0, "Error: socket not created.\n");
        goto shutdown;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        Display_printf(display, 0, 0, "Error: bind failed.\n");
        goto shutdown;
    }

    status = listen(server, 3);
    if (status == -1) {
        Display_printf(display, 0, 0, "Error: listen failed.\n");
        goto shutdown;
    }

    optval = 1;
    status = setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
    if (status == -1) {
        Display_printf(display, 0, 0, "tcpHandler: setsockopt failed\n");
        goto shutdown;
    }

    ppipc_buff_t *cam_buffer;
    while(1){

        addrlen = sizeof(localAddr);
        client = accept(server,(struct sockaddr *)&localAddr,(socklen_t)&addrlen);
        Display_printf(display, 0, 0, "Client Connected.\n");

        while ( bytesSent >= 0 ) { /* while tcp connection is up */
            cam_buffer = ppipc_get_buffer(&cam_buff_tab,PPIPC_READ); /* get buffer */
            bytesSent = send(client, cam_buffer->data, cam_buffer->size, 0);
            ppipc_update_buffer(&cam_buff_tab,PPIPC_READ); /* post buffer update */
            Task_sleep(10);
        }
        bytesSent = 0;
        Display_printf(display, 0, 0, "Client Dropped\n");
        Task_sleep(1);
    }

shutdown:
    if (server != -1) {
        close(server);
    }
}
