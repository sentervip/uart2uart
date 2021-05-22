#ifndef _PROCESS_H_  
#define _PROCESS_H_  

#include "stm32f10x.h"  
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
 
//standerr cmd
#define  FIX_HEAD  0xff
#define  FIX_OFFSET 0x05
#define  CMD_READ  0x22
#define  CMD_GET_RESULT  0x29
#define  CMD_CLR_RESULT  0x2A
#define MSG_CRC_INIT 0xFFFF
#define MSG_CCITT_CRC_POLY 0x1021

//private cmd
#define  PRI_CMD_READ  0xA0
#define  PRI_CMD_GET_RESULT  0xA1
#define  PRI_CMD_CLR_RESULT  0xA2
#define  PRI_CMD_RESET_RESULT 0xAA
#define  PRI_CMD_BUSY        0xBB

//ERROR CODE
#define  ERROR_EPC_LEN      0xE1
#define  ERROR_EPC_NUMBER_OVER      0xE2


typedef enum FrameIndex{
FI_HEAD=0,
FI_LEN,
FI_CMD,
FI_DATA,
}emFrameIndex;
typedef enum FrameRespondIndex{
    FR_HEAD=0,
    FR_LEN,
    FR_CMD,
    FR_STA1,
    FR_STA2,
    FR_DATA,
    FR_NUBS=7,//tags numbers 2byte
    FR_TAG_LEN=9, //single tag data length 2byte
    FR_TAG_DATA=13, //tag1, tag2=tag1+6(crc2+len2+2[34 00])
    
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
