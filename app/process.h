#ifndef _PROCESS_H_  
#define _PROCESS_H_  

#include "stm32f10x.h"  
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
 
//standerr cmd

#define  FIX_HEAD  0xBB
#define  CMD_TYPE  0x02
#define  CMD_READ  0x22
#define  CMD_GET_RESULT  0x22
#define  CMD_CLR_RESULT  0x2A
#define MSG_CRC_INIT 0xFFFF
#define MSG_CCITT_CRC_POLY 0x1021
#define FIX_OFFSET  5 // Length of frame = PLMSB>>7+PLLSB+ 7
#define EPC_LEN_OFFSET  7

//private cmd
#define  PRI_HEAD  0xff  //private HEAD
#define  PRI_CMD_READ  0xA0
#define  PRI_CMD_GET_RESULT  0xA1
#define  PRI_CMD_CLR_RESULT  0xA2
#define  PRI_CMD_RESET_RESULT 0xAA
#define  PRI_CMD_BUSY        0xBB

//ERROR CODE
#define  ERROR_EPC_LEN      0xE1
#define  ERROR_EPC_NUMBER_OVER      0xE2

//byte index of frame 27
typedef enum FrameIndex{
FI_HEAD=0,
FI_TYPE,
FI_CMD,
FI_PLMSB,//PL MSB
FI_PLLSB, //PL LSB
FI_PARA, //parameter
FI_EPC= 8 // index of epc 	
}emFrameIndex;
//byte index of respond frame 27
typedef enum FrameRespondIndex{
    FR_HEAD=0,
    FR_LEN,
    FR_CMD,
    FR_STA1,
    FR_STA2,
    FR_DATA
}emFrameRespIndex;
typedef enum SysMode{
    SYS_FORWARD=0,//正常转发状态
    SYS_READING,  //盘点标签状态

}emSysMode;
struct  TagProcessStruct{
    u8  AllTags[20*200];
    u8  AllTagsNum;
    u8  TimerOut;
    u16 SingleTagLen;
    u16 SingleNum;
    emSysMode SysMode;
    u8  CurReadCnt;
    u8  MaxReadCnt;
    
};
struct  Uart1ProcessStruct{
    u8 RxBuf[40];
    u32 RxCmplet;
    u32 RxCnt;
        
};
struct  Uart2ProcessStruct{
    u8 RxBuf[280];
    u32 ReadCnt;
    u32 RxCmplet;
    u32 RxCnt;
    

};
struct  Tim2ProcessStruct{
    u32 run;
    u32 timeoutCnt;
    u16 val;
    u16 reserve;
    
};

extern u8 BusyCmd[6];
extern u8 TxCmd[12];
extern struct  Uart1ProcessStruct  Uart1ProcessTag;
extern struct  Uart2ProcessStruct  Uart2ProcessTag;
extern struct  Tim2ProcessStruct   Tim2ProcessTag;
extern struct  TagProcessStruct    TagProcessTag;
void RCC_Configuration(void);
void NVIC_Configuration(void);
extern void TIM2_ConfigRun(void);
extern void TIM2_IRQHandler(void);
void ClearReader(void);
void RespGetTagOverCmdToPC(void);
void Uart1Process(void);
void tagsProcess(char * pData);
extern u16 CalcCRC(u8 *msgbuf,u8 msglen);
#endif
