#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <LPC21xx.H>
#include "types.h"


/*
   LCD uses P0.8-P0.18.

   Therefore LEDs and buzzer are placed on
   P0.20-P0.23.
*/
#define RED_LED       (1 << 20)
#define YELLOW_LED    (1 << 21)
#define GREEN_LED     (1 << 22)
#define BUZZER        (1 << 23)


/* Keypad */
#define ROW0 16
#define ROW1 17
#define ROW2 18
#define ROW3 19

#define COL0 20
#define COL1 21
#define COL2 22
#define COL3 23


/* LCD commands */
#define CLEAR_LCD            0x01
#define RET_CUR_HOME         0x02
#define SHIFT_CUR_RIGHT      0x06
#define SHIFT_CUR_LEFT       0x07
#define DSP_OFF              0x08
#define DSP_ON_CUR_OFF       0x0C
#define DSP_ON_CUR_ON        0x0E
#define DSP_ON_CUR_BLK       0x0F
#define SHIFT_DSP_LEFT       0x10
#define SHIFT_DSP_RIGHT      0x14

#define MODE_8BIT_1LINE      0x30
#define MODE_4BIT_1LINE      0x20
#define MODE_8BIT_2LINE      0x38
#define MODE_4BIT_2LINE      0x28

#define GOTO_LINE1_POS0      0x80
#define GOTO_LINE2_POS0      0xC0
#define GOTO_LINE3_POS0      0x94
#define GOTO_LINE4_POS0      0xD4

#define GOTO_CGRAM_START     0x40


/* LCD pins */
#define LCD_DATA 8
#define LCD_RS   16
#define LCD_RW   18
#define LCD_EN   17


/* Application constants */
#define DEFAULT_INTERVAL_SEC 15
#define DEFAULT_TARGET_GLASSES 8
#define ALARM_WINDOW_SEC 15
#define SECONDS_PER_DAY 86400
#define KEYPAD_DEBOUNCE_MS 20


/* Switch is connected to GND when pressed */
#define EINT_ACTIVE_LOW 1


/* ---------------- KEYPAD ---------------- */

void Init_KPM(void);
u32 colscan(void);
u32 rowcheck(void);
u32 colcheck(void);
u32 keyscan(void);
u32 ReadNum(void);
u32 Keypad_GetNum_Interactive(u8 line, u8 pos, u8 max_digits);


/* ---------------- LCD ---------------- */

void WriteLCD(u8 byte);
void CmdLCD(u8 cmd);
void InitLCD(void);
void CharLCD(u8 asciiVal);
void StrLCD(s8 *str);
void U32LCD(u32 n);
void S32LCD(s32 n);
void F32LCD(f32 fn, u8 nDP);
void BuildCGRAM(u8 *p, u8 nBytes);


/* ---------------- RTC ---------------- */

void RTC_Init(void);
void RTC_SetTime(u8 hour, u8 min, u8 sec);
void RTC_SetDate(u8 dom, u8 month, u16 year, u8 dow);
void RTC_CheckMidnightReset(void);


/* ---------------- APPLICATION VARIABLES ---------------- */

extern u8 EGCNT;
extern u8 FGCNT;
extern u8 missed_cnt;

extern volatile u8 reminder_active;

extern u8 alarm_hr;
extern u8 alarm_min;
extern u8 alarm_sec;

extern u32 interval_sec;
extern u32 next_trigger_sec;

extern u8 week[][4];


/* ---------------- APPLICATION FUNCTIONS ---------------- */

void System_GPIO_Init(void);

void LCD_Write2Digit(u32 val);

void Display_LiveClock(u8 hr,
                       u8 min,
                       u8 sec,
                       u8 line,
                       u8 pos);

void Display_Error(u8 max_val);

void display_menu(void);

void Trigger_Alarm(void);

void interr0_disp(void);
void interr1_disp(void);

void rtctime(void);
void alarmtime(void);
void targetglass(void);

void Update_Status_LEDs(void);

void Schedule_Next_Reminder(void);


/* ---------------- DELAYS ---------------- */

void delay_us(unsigned int tdly);
void delay_ms(unsigned int tdly);
void delay_s(unsigned int tdly);


/* ---------------- REGISTER MACROS ---------------- */

#define WRITEBYTE(WORD, STARTBIT, BYTE) \
    ((WORD) = ((WORD) & ~((u32)0xFF << (STARTBIT))) | \
    (((BYTE) & 0xFF) << (STARTBIT)))

#define READBIT(WORD, BIT) \
    (((WORD) >> (BIT)) & 1)

#define WRITEBIT(WORD, BIT, BITLEVEL) \
    ((WORD) = ((WORD) & ~(1 << (BIT))) | \
    (((BITLEVEL) & 1) << (BIT)))

#define WRITENIBBLE(WORD, STARTBIT, NIBBLE) \
    ((WORD) = ((WORD) & ~((u32)0xF << (STARTBIT))) | \
    (((NIBBLE) & 0xF) << (STARTBIT)))

#define READNIBBLE(WORD, SBIT) \
    (((WORD) >> (SBIT)) & 0xF)

#define CLRBIT(WORD, BIT) \
    ((WORD) &= ~(1 << (BIT)))

#endif