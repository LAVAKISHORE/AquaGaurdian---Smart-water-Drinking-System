#include "functions.h"

static u8 midnight_done = 0;


/*
   Initialize RTC hardware only.

   IMPORTANT:
   Do NOT set HOUR, MIN, SEC here.

   This prevents the RTC from being overwritten
   every time the program starts.
*/
void RTC_Init(void)
{
    /*
       Reset clock tick counter.
    */
    CCR = 1 << 1;

    /*
       Set 1 second tick time.
    */
    PREINT = 456;
    PREFRAC = 25024;
	HOUR  = 6;
    MIN   = 1;
    SEC   = 0;
    DOM   = 21;
    MONTH = 9;
    YEAR  = 2026;
    DOW   = 2;
    /*
       Enable RTC.
    */
    CCR = (1 << 0) | (1 << 4);
}


/*
   Set RTC time from Admin mode.
*/
void RTC_SetTime(u8 hour,
                 u8 min,
                 u8 sec)
{
    if (hour >= 24)
        return;

    if (min >= 60)
        return;

    if (sec >= 60)
        return;

    /*
       Stop RTC before changing time.
    */
    CCR &= ~(1 << 0);

    HOUR = hour;
    MIN = min;
    SEC = sec;

    /*
       Start RTC again.
    */
    CCR |= 1 << 0;

    midnight_done = 0;

    /*
       Reschedule reminder according to
       the newly entered RTC time.
    */
    Schedule_Next_Reminder();
}


/*
   Set RTC date.
*/
void RTC_SetDate(u8 dom,
                 u8 month,
                 u16 year,
                 u8 dow)
{
    if (dom < 1 || dom > 31)
        return;

    if (month < 1 || month > 12)
        return;

    if (year < 2000 || year > 2099)
        return;

    if (dow > 6)
        return;

    CCR &= ~(1 << 0);

    DOM = dom;
    MONTH = month;
    YEAR = year;
    DOW = dow;

    CCR |= 1 << 0;
}


/*
   Reset daily counters at midnight.
*/
void RTC_CheckMidnightReset(void)
{
    if ((HOUR == 0) &&
        (MIN == 0) &&
        (SEC == 0))
    {
        if (midnight_done == 0)
        {
            EGCNT = 0;
            missed_cnt = 0;

            reminder_active = 0;

            /*
               Turn OFF alarm.
            */
            IOCLR0 = YELLOW_LED | BUZZER;

            Update_Status_LEDs();

            Schedule_Next_Reminder();

            midnight_done = 1;
        }
    }
    else
    {
        midnight_done = 0;
    }
}