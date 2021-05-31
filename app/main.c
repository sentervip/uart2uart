#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include "process.h"

// 'X'  is no Byte;
//CRC16: LEN--DATA 之间的bytes 计算CalcCRC16值
// format: CMD           SOF  LEN   CMD  DATA CRCH  CRCL
//private  ReadCmd       ff   01    a0   0    00    00
//         Respond       ff   01    a0   0    00    00
//private  GetCmd        ff   01    a1   05   00    00
//         Respond       ff   01    a1   20   00    00
//private  ClrCmd        ff   00    a2   X    00    00
//         Respond       ff   00    a2   X    00    00
//private  RESETCmd      ff   00    aa   X    00    00
//         Respond       ff   00    aa   X    00    00
u8 BusyCmd[] =        {0xFF,0x00, PRI_CMD_BUSY, 0xF4, 0x22};
u8 TxCmd[12] = {0xFF,0x00};
struct  Uart1ProcessStruct  Uart1ProcessTag;
struct  Uart2ProcessStruct  Uart2ProcessTag;
struct  Tim2ProcessStruct   Tim2ProcessTag;
struct  TagProcessStruct    TagProcessTag;


void test(void)
{
    u8  tx[12]={0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x0,0x03,0x4F, 0x7E};//{0xFF, 0x04, 0x22, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x78, 0xAB};
    //u8  tx[]={0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
		u16 val;

    val = CalcCRC(&tx[1],7);
    tx[9] = val>>8;
    tx[10] = val&0xff;
    uart1_send_buff(tx, 12);  
}
int main()
{
u32 iTmp;


RCC_Configuration();
NVIC_Configuration();
TIM2_ConfigRun();
//TIM_Cmd(TIM2, DISABLE);
uart1_init(115200);
uart2_init(115200);

memset((char*)&Uart1ProcessTag, 0, sizeof(struct  Uart1ProcessStruct ));
memset((char*)&Uart2ProcessTag, 0, sizeof(struct  Uart2ProcessStruct ));
memset((char*)&Tim2ProcessTag, 0, sizeof(struct  Tim2ProcessStruct ));
memset((char*)&TagProcessTag, 0, sizeof(struct  TagProcessStruct ));
TagProcessTag.SysMode = SYS_FORWARD; //for test
//test();
//uart1_send_char(0x12); // start flag
//uart2_send_char(0x23);
while(1){

	//uart1 process  ;to pc
   Uart1Process();
    
   //uart2 process; to reader; [get echo cmd 22 && type=2]
   if( TagProcessTag.SysMode == SYS_READING){
        if(Uart2ProcessTag.RxCmplet){       
            if(Uart2ProcessTag.RxBuf[FR_TYPE] == FR_FIX_NOTIFY_TYPE &&
						  	Uart2ProcessTag.RxBuf[FP_CMD] == FR_FIX_CMD_MULTI_READ_ECHO){  
                
                tagsProcess(Uart2ProcessTag.RxBuf);
                if(TagProcessTag.SingleNum == 0){ // get tags over
                    
                    //1 for pc                     
                   // RespGetTagOverCmdToPC();                    
                    
                    //2 for reader
                    //ClearReader();
                    TagProcessTag.SysMode = SYS_FORWARD;
                }
            }else{
                ;//throw it;
                //uart1_send_buff(Uart2ProcessTag.RxBuf, Uart2ProcessTag.RxCnt);
               // memset(Uart2ProcessTag.RxBuf, 0, Uart2ProcessTag.RxCnt);
               // Uart2ProcessTag.RxCnt = 0;
            }
            Uart2ProcessTag.RxCmplet = 0;               
            Uart2ProcessTag.RxCnt = 0;  
        }
    }
	 #if 0  // not support frame forward
		else{
        if(Uart2ProcessTag.RxCmplet){   
            if(Uart2ProcessTag.RxBuf[FR_CMD] == FR_FIX_CMD_MULTI_READ_ECHO){
                
							/*
                // 1 for reader
                TagProcessTag.CurReadCnt = 0;
                TagProcessTag.MaxReadCnt = Uart1ProcessTag.RxBuf[FR_DATA];
                //FF 03 29 00 00 00 F4 22 ;得到盘点结果
                TxCmd[FR_HEAD] = PRI_HEAD;
                TxCmd[FR_LEN] = 0x03;
                TxCmd[FR_CMD] = CMD_GET_RESULT;
                TxCmd[FR_DATA] = 0;
                TxCmd[FR_DATA+1] = 0;
                TxCmd[5] = 0;
                TxCmd[6] = 0xF4;
                TxCmd[7] = 0X22;
                uart2_send_buff(TxCmd, 8);
							*/
                TagProcessTag.SysMode = SYS_READING;
                
            //}else if(Uart2ProcessTag.RxBuf[FR_CMD] == CMD_CLR_RESULT ){
                     //|| Uart2ProcessTag.RxBuf[FR_CMD] == CMD_READ){
              //  ;//throw it
            }else{ //forward
                uart1_send_buff(Uart2ProcessTag.RxBuf, Uart2ProcessTag.RxCnt);
                memset(Uart2ProcessTag.RxBuf, 0, Uart2ProcessTag.RxCnt);
            }
            Uart2ProcessTag.RxCmplet = 0;               
            Uart2ProcessTag.RxCnt = 0;
        }
    }
		#endif
}
}


