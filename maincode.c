#include <LPC21xx.H>
#include "functions.h"

#define EINT0_VIC_CHNO 14
#define EINT1_VIC_CHNO 15

#define EINT0_PIN_FUNC 0x0000000C
#define EINT1_PIN_FUNC 0x000000C0

void eint0_isr(void) __irq;
void eint1_isr(void) __irq;

int main(void)
{
    u32 current_sec;

    /* Hardware initialization */
    System_GPIO_Init();
    InitLCD();
    Init_KPM();
    RTC_Init();

    /*
       P0.1 -> EINT0
       P0.3 -> EINT1
    */
    PINSEL0 &= ~((3 << 2) | (3 << 6));
    PINSEL0 |= EINT0_PIN_FUNC | EINT1_PIN_FUNC;

    /*
       EINT0 and EINT1 are IRQ type.
    */
    //VICIntSelect &= ~((1 << EINT0_VIC_CHNO) |
                      //(1 << EINT1_VIC_CHNO));

    /*
       Enable EINT0 and EINT1.
    */
    VICIntEnable = (1 << EINT0_VIC_CHNO) |
                   (1 << EINT1_VIC_CHNO);

    /*
       EINT0 -> VIC vector slot 0.
    */
    VICVectCntl0 = (1 << 5) | EINT0_VIC_CHNO;
    VICVectAddr0 = (u32)eint0_isr;

    /*
       EINT1 -> VIC vector slot 1.
    */
    VICVectCntl1 = (1 << 5) | EINT1_VIC_CHNO;
    VICVectAddr1 = (u32)eint1_isr;

    /*
       Edge triggered.
    */
    EXTMODE = (1 << 0) | (1 << 1);

    /*
       Falling edge triggered.
    */
    //EXTPOLAR = 0;

    /*
       Clear pending EINT flags.
    */
    //EXTINT = (1 << 0) | (1 << 1);

    /*
       Start reminder scheduling.
    */
    Schedule_Next_Reminder();

    while (1)
    {
        /*
           Normal LCD display.
        */
        display_menu();

        /*
           Current RTC time in seconds.
        */
        current_sec = ((u32)HOUR * 3600) +
                      ((u32)MIN * 60) +
                      SEC;

        /*
           Check whether reminder time has arrived.
        */
        if (reminder_active == 0)
        {
            if (current_sec == next_trigger_sec)
            {
                Trigger_Alarm();
				Schedule_Next_Reminder();
            }
        }
    }
}


/*
   EINT0 = Admin switch
*/
void eint0_isr(void) __irq
{
    interr0_disp();
}


/*
   EINT1 = Drink switch
*/
void eint1_isr(void) __irq
{
    interr1_disp();
}