//KPM.c
#include "functions.h"
 /*u32 KPMLUT[4][4]={{1,2,3,4},
									 {5,6,7,8},
									 {9,10,11,12},
									 {13,14,15,16}};*/
 u32 KPMLUT[4][4]={{'1','2','3','A'},
									 {'4','5','6','B'},
									 {'7','8','9','C'},
									 {'=','0','=','+'}};
 
									 
void Init_KPM(void)
{
	//cfg rows(p1.16-p1.19) as ouputs
	WRITENIBBLE(IODIR1,ROW0,15);
}

u32 colscan(void)
{
	if(READNIBBLE(IOPIN1,COL0)<15)
		return 0;
	else 
		return 1;
}
u32 rowcheck(void)
{
	u32 rno;
	for(rno=0; rno<4; rno++)
	{
		WRITENIBBLE(IOPIN1,ROW0,(~(1<<rno)));
		if(colscan()==0)
			break;
	}
	//make rows as defaults
	IOCLR1=15<<ROW0;
	return rno;
}
u32 colcheck(void)
{
	u32 cno;
	for(cno=0; cno<4; cno++)
	{
		if(READBIT(IOPIN1,(cno+COL0))==0)
			break;
	}
	return cno;
}
u32 keyscan(void)
{
	u32 rno,cno,key;
	//wait for switch press
	while(colscan());
	//find rno
	rno=rowcheck();
	//find cno
	cno=colcheck();
	//collect key using kpmLut
	key=KPMLUT[rno][cno];
	//wait for switch release
	while(!colscan());
	return key;
}
u32 ReadNum(void)
{
	u8 key;
	u32 num=0;
	while(1)
	{
		key=keyscan();
		if(key>='0'&&key<='9')
		{
			num=(num*10)+(key-48);
			
		}
		else
			break;
	}
	return num;
}
u32 Keypad_GetNum_Interactive(u8 line, u8 pos, u8 max_digits) {
    u8 idx = 0, key, i;
    s8 buf[10];
    u8 base_addr = (line == 1) ? GOTO_LINE1_POS0 : GOTO_LINE2_POS0;
    u32 val = 0;

    while (1) {
        key = keyscan();

        // Single digit entry
        if (key >= '0' && key <= '9') {
            if (idx < max_digits) {
                buf[idx] = key;
                CmdLCD(base_addr + pos + idx);
                CharLCD(key);
                idx++;
            }
        }
        // Character erase ('C')
        else if (key == 'C') {
            if (idx > 0) {
                idx--;
                CmdLCD(base_addr + pos + idx);
                CharLCD(' ');
                CmdLCD(base_addr + pos + idx);
            }
        }
        // Input confirmation ('A')
        else if (key == 'A') {
            if (idx > 0) {
                val = 0;
                for (i = 0; i < idx; i++) {
                    val = (val * 10) + (buf[i] - '0');
                }
                return val;
            }
        }
    }
}
