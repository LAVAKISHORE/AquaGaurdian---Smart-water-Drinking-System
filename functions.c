#include "functions.h"


u8 EGCNT = 0;

u8 FGCNT = DEFAULT_TARGET_GLASSES;

u8 missed_cnt = 0;

volatile u8 reminder_active = 0;

u8 arr[16]={0X11,0X11,0X11,0X11,0X11,0X11,0X1F,0X00,0X11,0X11,0XFF,0XFF,0XFF,0XFF,0XFF,0X00};
u8 alarm_hr = 0;
u8 alarm_min = 0;
u8 alarm_sec = DEFAULT_INTERVAL_SEC;

u32 interval_sec = DEFAULT_INTERVAL_SEC;

u32 next_trigger_sec = DEFAULT_INTERVAL_SEC;


u8 week[][4] =
{
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT"
};


/* ---------------- DELAYS ---------------- */

void delay_us(unsigned int tdly)
{
    tdly *= 12;

    while (tdly--);
}


void delay_ms(unsigned int tdly)
{
    tdly *= 12000;

    while (tdly--);
}


void delay_s(unsigned int tdly)
{
    tdly *= 12000000;

    while (tdly--);
}


/* ---------------- CURRENT RTC TIME ---------------- */

static u32 CurrentSeconds(void)
{
    return ((u32)HOUR * 3600) +
           ((u32)MIN * 60) +
           SEC;
}


/* ---------------- GPIO ---------------- */

void System_GPIO_Init(void)
{
    IODIR0 |= RED_LED |
              YELLOW_LED |
              GREEN_LED |
              BUZZER;

    IOCLR0 = RED_LED |
             YELLOW_LED |
             GREEN_LED |
             BUZZER;
}


/* ---------------- LCD ---------------- */

void LCD_Write2Digit(u32 val)
{
    if (val < 10)
        CharLCD('0');

    U32LCD(val);
}


void Display_LiveClock(u8 hr,
                       u8 min,
                       u8 sec,
                       u8 line,
                       u8 pos)
{
    u8 base_addr;

    if (line == 1)
        base_addr = GOTO_LINE1_POS0;
    else
        base_addr = GOTO_LINE2_POS0;

    CmdLCD(base_addr + pos);

    LCD_Write2Digit(hr);
    CharLCD(':');

    LCD_Write2Digit(min);
    CharLCD(':');

    LCD_Write2Digit(sec);
}


void Display_Error(u8 max_val)
{
    CmdLCD(CLEAR_LCD);

    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"INVALID INPUT!");

    CmdLCD(GOTO_LINE2_POS0);
    StrLCD((s8 *)"Max Allowed:");

    LCD_Write2Digit(max_val);

    delay_ms(1500);
}


/* ---------------- STATUS LED ---------------- */

void Update_Status_LEDs(void)
{
    /*
       First turn RED and GREEN OFF.
    */
    IOCLR0 = RED_LED | GREEN_LED;

    if (EGCNT >= FGCNT)
    {
        /*
           Target achieved
        */
        IOSET0 = GREEN_LED;
    }
    else if (EGCNT < missed_cnt )
    {
        /*
           Behind schedule
        */
        IOSET0 = RED_LED;
    }
}


/* ---------------- REMINDER SCHEDULING ---------------- */

void Schedule_Next_Reminder(void)
{
    u32 current_sec;

    current_sec = CurrentSeconds();

    if (interval_sec == 0)
        interval_sec = DEFAULT_INTERVAL_SEC;

    next_trigger_sec =
        (current_sec + interval_sec) % SECONDS_PER_DAY;
}


/* ---------------- NORMAL LCD MENU ---------------- */

void display_menu(void)
{
    /*
       Line 1:
       HH:MM:SS   DAY
    */
   BuildCGRAM(arr,16);
    Display_LiveClock(HOUR,
                      MIN,
                      SEC,
                      1,
                      0);

    CmdLCD(GOTO_LINE1_POS0 + 11);
    StrLCD((s8 *)week[DOW]);


    /*
       Line 2:
       D:xx M:xx T:xx
    */
    CmdLCD(GOTO_LINE2_POS0);

    //StrLCD((s8 *)"D:");
	CharLCD(0);
	CharLCD(':');
    LCD_Write2Digit(EGCNT);

    StrLCD((s8 *)" M:");
    LCD_Write2Digit(missed_cnt);
	CharLCD(' ');
	  CharLCD(1);
	  CharLCD(':');
    //StrLCD((s8 *)" T:");
    LCD_Write2Digit(FGCNT);


    /*
       Check for new day.
    */
    RTC_CheckMidnightReset();

    Update_Status_LEDs();
}


/* ================================================= */
/*                 DRINK ALARM                       */
/* ================================================= */

void Trigger_Alarm(void)
{
    u32 start_sec;
    u32 current_sec;
    u32 elapsed_sec;

    u8 remaining_sec;


    /*
       Alarm is now active.
    */
    reminder_active = 1;


    /*
       Turn Yellow LED and buzzer ON.
    */
    IOCLR0 = RED_LED | GREEN_LED;

    IOSET0 = YELLOW_LED | BUZZER;


    /*
       Display reminder.
    */
    CmdLCD(CLEAR_LCD);

    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"DRINK WATER!");


    /*
       Store RTC time at the beginning
       of the alarm.
    */
    start_sec = CurrentSeconds();


    /*
       15-second countdown.
    */
    while (reminder_active)
    {
        current_sec = CurrentSeconds();


        /*
           Calculate elapsed time.

           Handles:
           23:59:59 -> 00:00:00
        */
        if (current_sec >= start_sec)
        {
            elapsed_sec = current_sec - start_sec;
        }
        else
        {
            elapsed_sec =
                (SECONDS_PER_DAY - start_sec) +
                current_sec;
        }


        /*
           15 seconds completed?
        */
        if (elapsed_sec >= ALARM_WINDOW_SEC)
            break;


        /*
           15,14,13,...1
        */
        remaining_sec =
            (u8)(ALARM_WINDOW_SEC - elapsed_sec);


        /*
           Display countdown.
        */
        CmdLCD(GOTO_LINE2_POS0);

        StrLCD((s8 *)"IN:   ");

        LCD_Write2Digit(remaining_sec);


        /*
           Small display refresh delay.
        */
        delay_ms(100);
    }


    /*
       If reminder_active is still 1,
       the user did NOT press Drink switch.
    */
    if (reminder_active)
    {
        /*
           MISSED DRINK
        */
        reminder_active = 0;


        /*
           Turn alarm OFF.
        */
        IOCLR0 = YELLOW_LED | BUZZER;


        /*
           Increment missed counter.
        */
        if (missed_cnt < 255)
            missed_cnt++;


        /*
           Show missed message.
        */
        CmdLCD(CLEAR_LCD);

        CmdLCD(GOTO_LINE1_POS0);
        StrLCD((s8 *)"MISSED DRINK!");

        delay_ms(2000);

        CmdLCD(CLEAR_LCD);
    }


    Update_Status_LEDs();
}


/* ================================================= */
/*                 EINT0 - ADMIN                     */
/* ================================================= */

void interr0_disp(void)
{
    u32 ch;


    /*
       Display Admin Mode.
    */
    CmdLCD(CLEAR_LCD);

    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"ADMIN MODE");

    delay_ms(1000);


    /*
       Admin menu.
    */
   men: CmdLCD(CLEAR_LCD);

    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"1.RTC  2.Alarm");

    CmdLCD(GOTO_LINE2_POS0);
    StrLCD((s8 *)"3.Target 4.Exit");


    /*
       Read keypad.
    */
    ch = keyscan();
	U32LCD(ch);

    switch (ch)
    {
        case '1':
            rtctime();
			goto men;
            break;

        case '2':
            alarmtime();
			goto men;
            break;

        case '3':
            targetglass();
			goto men;
            break;

        case '4':
            break;

        default:
            break;
    }


    CmdLCD(CLEAR_LCD);

    Update_Status_LEDs();


    /*
       Clear EINT0.
    */
    EXTINT = 1 << 0;

    /*
       Clear VIC address.
    */
    VICVectAddr = 0;
}


/* ================================================= */
/*                 EINT1 - DRINK                     */
/* ================================================= */

void interr1_disp(void)
{
    /*
       Drink switch only works when
       reminder is active.
    */
    if (reminder_active)
    {
        /*
           Add one glass.
        */
        if (EGCNT < 255)
            EGCNT++;


        /*
           Stop countdown.
        */
        reminder_active = 0;


        /*
           Turn alarm OFF.
        */
        IOCLR0 = YELLOW_LED | BUZZER;


        /*
           Display successful hydration.
        */
        CmdLCD(CLEAR_LCD);

        CmdLCD(GOTO_LINE1_POS0);
        StrLCD((s8 *)"  HYDRATED!  ");

        CmdLCD(GOTO_LINE2_POS0);
        StrLCD((s8 *)"+1 Glass Added");


        delay_ms(1000);

        CmdLCD(CLEAR_LCD);


        Update_Status_LEDs();
    }


    /*
       Clear EINT1.
    */
    EXTINT = 1 << 1;

    /*
       Clear VIC address.
    */
    VICVectAddr = 0;
}


/* ================================================= */
/*                 RTC MENU                          */
/* ================================================= */

void rtctime(void)
{
    u8 key;
    u32 val;


    while (1)
    {
        CmdLCD(CLEAR_LCD);

        CmdLCD(GOTO_LINE1_POS0);
        StrLCD((s8 *)"1.HR 2.MIN 3.SEC");

        CmdLCD(GOTO_LINE2_POS0);
        StrLCD((s8 *)"4.Exit");


        key = (u8)keyscan();


        if (key == '4')
            break;


        switch (key)
        {
            case '1':

                CmdLCD(CLEAR_LCD);

                CmdLCD(GOTO_LINE1_POS0);
                StrLCD((s8 *)"Set HR:");

                val =
                    Keypad_GetNum_Interactive(2, 8, 2);

                if (val < 24)
                {
                    RTC_SetTime((u8)val,
                                MIN,
                                SEC);
                }
                else
                {
                    Display_Error(23);
                }

                break;


            case '2':

                CmdLCD(CLEAR_LCD);

                CmdLCD(GOTO_LINE1_POS0);
                StrLCD((s8 *)"Set MIN:");

                val =
                    Keypad_GetNum_Interactive(2, 8, 2);

                if (val < 60)
                {
                    RTC_SetTime(HOUR,
                                (u8)val,
                                SEC);
                }
                else
                {
                    Display_Error(59);
                }

                break;


            case '3':

                CmdLCD(CLEAR_LCD);

                CmdLCD(GOTO_LINE1_POS0);
                StrLCD((s8 *)"Set SEC:");

                val =
                    Keypad_GetNum_Interactive(2, 8, 2);

                if (val < 60)
                {
                    RTC_SetTime(HOUR,
                                MIN,
                                (u8)val);
                }
                else
                {
                    Display_Error(59);
                }

                break;
        }
    }
}


/* ================================================= */
/*                 ALARM MENU                        */
/* ================================================= */

void alarmtime(void)
{
    u8 key;
    u32 val;


    while (1)
    {
        CmdLCD(CLEAR_LCD);

        CmdLCD(GOTO_LINE1_POS0);
        StrLCD((s8 *)"1.HR 2.MIN 3.SEC");

        CmdLCD(GOTO_LINE2_POS0);
        StrLCD((s8 *)"4.Exit");


        key = (u8)keyscan();


        if (key == '4')
            break;


        switch (key)
        {
            case '1':

                CmdLCD(CLEAR_LCD);

                CmdLCD(GOTO_LINE1_POS0);
                StrLCD((s8 *)"Intvl HR:");

                val =
                    Keypad_GetNum_Interactive(2, 9, 2);

                if (val < 24)
                    alarm_hr = (u8)val;
                else
                    Display_Error(23);

                break;


            case '2':

                CmdLCD(CLEAR_LCD);

                CmdLCD(GOTO_LINE1_POS0);
                StrLCD((s8 *)"Intvl MIN:");

                val =
                    Keypad_GetNum_Interactive(2, 10, 2);

                if (val < 60)
                    alarm_min = (u8)val;
                else
                    Display_Error(59);

                break;


            case '3':

                CmdLCD(CLEAR_LCD);

                CmdLCD(GOTO_LINE1_POS0);
                StrLCD((s8 *)"Intvl SEC:");

                val =
                    Keypad_GetNum_Interactive(2, 10, 2);

                if (val < 60)
                    alarm_sec = (u8)val;
                else
                    Display_Error(59);

                break;
        }


        /*
           Convert HH:MM:SS to seconds.
        */
        interval_sec =
            ((u32)alarm_hr * 3600) +
            ((u32)alarm_min * 60) +
            alarm_sec;


        /*
           Prevent zero interval.
        */
        if (interval_sec == 0)
            interval_sec = DEFAULT_INTERVAL_SEC;


        /*
           Schedule next alarm.
        */
        Schedule_Next_Reminder();
    }
}


/* ================================================= */
/*                 TARGET MENU                       */
/* ================================================= */

void targetglass(void)
{
    u32 val;


    CmdLCD(CLEAR_LCD);

    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"Target Glass:");


    val =
        Keypad_GetNum_Interactive(2, 13, 2);


    /*
       Valid target: 1 to 99.
    */
    if (val >= 1 && val <= 99)
    {
        FGCNT = (u8)val;
    }
    else
    {
        Display_Error(99);
    }


    Update_Status_LEDs();
}