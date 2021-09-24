/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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
 *    ======== buttonled.c ========
 */
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/display/Display.h>

#include <ti/drivers/utils/RingBuf.h>
#include <ti/drivers/apps/LED.h>
#include <ti/drivers/apps/Button.h>

#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* Driver Configuration */
#include "ti_drivers_config.h"

#define BLINKCOUNT            3
#define FASTBLINK             500
#define SLOWBLINK             1000
#define FIFTYMS               50000
#define EVENTBUFSIZE          10


#ifndef CONFIG_BUTTONCOUNT
#define CONFIG_BUTTONCOUNT     2
#endif

#ifndef CONFIG_LEDCOUNT
#define CONFIG_LEDCOUNT        2
#else
#define CONFIG_LED2            2
#endif

typedef struct buttonStats
{
    unsigned int pressed;
    unsigned int clicked;
    unsigned int released;
    unsigned int longPress;
    unsigned int longClicked;
    unsigned int doubleclicked;
    unsigned int lastpressedduration;
} buttonStats;

Button_Handle    buttonHandle[CONFIG_BUTTONCOUNT];
LED_Handle       ledHandle[CONFIG_LEDCOUNT];
Display_Handle   display;
buttonStats      bStats;
RingBuf_Object   ringObj;
uint8_t          eventBuf[EVENTBUFSIZE];

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    int inc;
    bool dir = true;

    GPIO_init();
    Button_init();
    LED_init();

    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if(display == NULL)
    {
        while(1);
    }

    Display_print0(display, 0, 0, "Button/LED Demo:\n"
                   "Each button controls an LED. Click to toggle, "
                   "double click to fast blink three times, "
                   "hold the button to slow blink.\n");

    while(1)
    {
        GPIO_write(CONFIG_GPIO_2, 1);
        Task_sleep(1000);
        GPIO_write(CONFIG_GPIO_2, 0);
        Task_sleep(1000);
    }
}
