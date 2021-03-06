#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include "process.h"

#define QM100
//static const u8 ReadCmd0[10] = {0xFF,0x05,0x22,0x00,0x00,0x03,0x03,0x20,0x3B,0xFC};  
static const u8 ReadCmd0[10] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x0,0x03,0x4F, 0x7E};  
static const u8 ReadCmd1[10] = {0xff,0x05,0x22,0x00,0x00,0x03,0x02,0x58,0x3a,0x84}; 
static const u8 ReadCmd2[10] = {0xFF,0x05,0x22,0x00,0x00,0x03,0x01,0xF4,0x39,0x28};
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
        //uart1_send_char(0xA5);  // debug
        Tim2ProcessTag.val++;  
        //TIM_Cmd(TIM2, DISABLE);    			
    }
}
void TIM2_ConfigRun(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    //重新将Timer设置为缺省值
    TIM_DeInit(TIM2);
    //采用内部时钟给TIM2提供时钟源
    TIM_InternalClockConfig(TIM2);
    //预分频系数为36000-1，这样计数器时钟为72MHz/36000 = 2kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 36000 - 1;
    //设置时钟分割
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    //设置计数器模式为向上计数模式
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    //设置计数溢出大小，每计2000个数就产生一个更新事件
    TIM_TimeBaseStructure.TIM_Period = 2000;
    //将配置应用到TIM2中
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    //清除溢出中断标志
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    //禁止ARR预装载缓冲器
    TIM_ARRPreloadConfig(TIM2, DISABLE);  //预装载寄存器的内容被立即传送到影子寄存器 
    //开启TIM2的中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE); //run tim2
}
void RCC_Configuration(void)
{
    
    ErrorStatus HSEStartUpStatus; 
    //RCC_DeInit();
    //RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if(HSEStartUpStatus == SUCCESS)
    {
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        FLASH_SetLatency(FLASH_Latency_2);
        RCC_HCLKConfig(RCC_SYSCLK_Div1); 
        RCC_PCLK2Config(RCC_HCLK_Div1); 
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while(RCC_GetSYSCLKSource() != 0x08) {}
    }
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
#ifdef  VECT_TAB_RAM  
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn ;//TIM2_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/* CRC16   */
void CRC_calcCrc8(unsigned short int *crcReg, unsigned short int poly, unsigned short int u8Data)
{
    unsigned short int i;
    unsigned short int xorFlag;
    unsigned short int bit;
    unsigned short int dcdBitMask = 0x80;
    for(i=0; i<8; i++)
    {
        xorFlag = *crcReg & 0x8000;
        *crcReg <<= 1;
        bit = ((u8Data & dcdBitMask) == dcdBitMask);
        *crcReg |= bit;
        if(xorFlag)
        {
            *crcReg = *crcReg ^ poly;
        }
        dcdBitMask >>= 1;
    }
}

/* CRC16 string */
unsigned short int CalcCRC16(unsigned char *buf, unsigned char len)
{
    unsigned short calcCrc = MSG_CRC_INIT;
    unsigned char  i;

    for (i = 0; i< len; i++)
    {
        CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY,  buf[i]);
    }
    return calcCrc;
}

/**
* for QM100/M100 reader
* checksum 为type到最后一个指令参数parameter累加和，并只取累加和最低字节LSB
*/
unsigned short int CalcCRC(unsigned char *uBuff,unsigned char uBuffLen)
{
	unsigned short int uSum = 0;
	unsigned char i = 0;
	for(i=0;i<uBuffLen;i++)
	{
		uSum += uBuff[i];
 		uSum &= 0xFFu;// must &0xffu,否则容易死机
  }
	return uSum &=0x00ff;
}


//clr for reader
void ClearReader(void)
{
	  TagProcessTag.SingleNum  = 0;
    //TagProcessTag.CurReadCnt = 0;
    //TagProcessTag.MaxReadCnt = 0;
    TagProcessTag.AllTagsNum = 0;
}
//respond to pc
void RespGetTagOverCmdToPC(void)
{
    u8  tx[8];
    u32 val;
    
    //ff   01    a0   amount   00    00  盘点结束   
    tx[FR_HEAD] = PRI_FIX_HEAD;
    tx[1] = 1;
    tx[FP_CMD] = PRI_CMD_READ;
    tx[FP_DATA] = TagProcessTag.AllTagsNum; 
    val = CalcCRC((char*)&tx[FP_LEN],5);
    tx[6] = val>>8;
    tx[7] = val&0xff;
    uart1_send_buff(tx, 8);  
}
void Uart1Process(void)
{
    //u8 SendResCmd[10];
   u16 val = 0;
    
   
	   //3 time out
     if( abs(Uart1ProcessTag.TimerOut - Tim2ProcessTag.val) > 2){		
            Uart1ProcessTag.RxCnt = 0;	
		 }
    if(Uart1ProcessTag.RxCmplet ){  
        
        //1 is busy?
        if( TagProcessTag.SysMode == SYS_READING){
            uart1_send_buff(BusyCmd,sizeof(BusyCmd)); //busy
            memset(Uart1ProcessTag.RxBuf, 0, Uart1ProcessTag.RxCnt);
            Uart1ProcessTag.RxCmplet = 0;
            return;
        }
        
        
        //2 is CMD_READ ?
        if(Uart1ProcessTag.RxBuf[FP_CMD] == PRI_CMD_READ){
            
            // 1 read EPC
            if(Uart1ProcessTag.RxBuf[FP_DATA] == 0x01){   
                uart2_send_buff((u8*)ReadCmd1,sizeof(ReadCmd1)) ;  
                             
            }else{
                u8 SendResCmd[6] = {0xff,0x01,0xa0,0x0,0x1d,0xae};
							  uart1_send_buff((u8*)SendResCmd,sizeof(SendResCmd)) ;
                uart2_send_buff((u8*)ReadCmd0,sizeof(ReadCmd0)) ;                 
            }
              
                              
        //is PRI_CMD_CLR_RESULT ?
        }else if(Uart1ProcessTag.RxBuf[FP_CMD] == PRI_CMD_CLR_RESULT){  
            
            //1 for pc
             u8 ClrRespCmd[]={0xff, 0x00,0xa2,0x11,0x68};
            uart1_send_buff(ClrRespCmd, sizeof(ClrRespCmd));  
            
            //2 clear buf action
            memset((char*)&TagProcessTag, 0, sizeof(struct  TagProcessStruct ));
            //TagProcessTag.SingleTagLen = 0;  
                
            
        // PRI_CMD_GET_RESULT ?
        }else if(Uart1ProcessTag.RxBuf[FP_CMD] == PRI_CMD_GET_RESULT){
            u8 SendResCmd[6] = {0xff,0x01,0xa1,0x0,0x1d,0xae};
            //1 respond for pc
            SendResCmd[FP_DATA] = TagProcessTag.AllTagsNum;        
            val = CalcCRC16((char*)&SendResCmd[FP_LEN],3);
						SendResCmd[4] = val >>8;
						SendResCmd[5] = val & 0xff;
            uart1_send_buff(SendResCmd,SendResCmd[FP_LEN]+ 5); 
            
        //reset cmd 
        }else if(Uart1ProcessTag.RxBuf[FP_CMD] == PRI_CMD_RESET_RESULT){
            
            //respond for pc  
            u8 ResetRespCmd[5]={0xff, 0x00,0xaa,0x1d,0xa5};
            uart1_send_buff(ResetRespCmd,sizeof(ResetRespCmd)); 
            for(u32 i=0;i<9000;i++){
                ;;
            }
            __disable_fault_irq();
            NVIC_SystemReset();
            
        //forward data
        }else{
            uart2_send_buff(Uart1ProcessTag.RxBuf, Uart1ProcessTag.RxCnt);
        }
        memset(Uart1ProcessTag.RxBuf, 0, Uart1ProcessTag.RxCnt);
        Uart1ProcessTag.RxCmplet = 0;
        Uart1ProcessTag.RxCnt = 0;          
    }
}
void Uart2Process(void)
{
	//uart2 process; to reader; [get echo cmd 22 && type=2]
	
	//time out
	if( abs(Uart2ProcessTag.TimerOut - Tim2ProcessTag.val) > 2){		
		Uart2ProcessTag.RxCnt = 0;	
	}
	
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
}
/**
*  single tags and append to tagsBuf
*
*/
void tagsAppend(char * pData)
{    
    int i=0;
    int j=0;
    
    
    if(TagProcessTag.AllTagsNum == 0){      //first time all of valid
        memcpy((char*)(&TagProcessTag.AllTags[TagProcessTag.AllTagsNum * TagProcessTag.SingleTagLen]), 
                       pData,
                       TagProcessTag.SingleTagLen);
       TagProcessTag.AllTagsNum++;
    }else{ // after first times 
          while(i < TagProcessTag.AllTagsNum){
              j = memcmp((char*)(&TagProcessTag.AllTags[i*TagProcessTag.SingleTagLen]),
                          pData, TagProcessTag.SingleTagLen);
              if(j == 0){ 
                  j = 0xa0;
                  break;
              }
             i++;
         }
         if( 0xa0 != j){
             memcpy((char*)(&TagProcessTag.AllTags[TagProcessTag.AllTagsNum * TagProcessTag.SingleTagLen]),
                    pData, TagProcessTag.SingleTagLen);
             TagProcessTag.AllTagsNum++;
         }
    }
         
    

}
/**
*  get uart2 all tags and append to tagsBuf
*
*/
void tagsProcess(char * pData)
{
    int i;
    
    if(CMD_GET_RESULT == pData[FR_CMD] ){
        
        if(  pData[FP_HEAD] <= 0x0a ){ // read over？ lenth of epc > 4(10-6) bytes
            TagProcessTag.SingleNum = 0;
            return;
        }
        //start only calculate one times
        if(TagProcessTag.SingleTagLen == 0){ 
            TagProcessTag.SingleTagLen = (pData[FR_PLLSB] + FIX_FR_OFFSET) - 12;         
        }
        
        if(TagProcessTag.SingleNum > 100){
            uart1_send_char(ERROR_EPC_NUMBER_OVER);
          return;
        }
        tagsAppend( &pData[FR_EPC]);       
    }

}

