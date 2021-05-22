 
#include "stm32f10x_gpio.h"
#include <string.h>
#include <stdio.h>  
#include "process.h"
#include "uart.h"   


int fputc(int ch,FILE *p)       //在使用printf时系统自动条用此函数    
{    
    USART_SendData(USART2,(u8)ch);      
    while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);    
    return ch;    

}


void USART1_IRQHandler(void)  
{  
    USART_ClearFlag(USART1,USART_FLAG_TC);  
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=Bit_RESET)//检查指定的USART中断发生与否  
    {   
        
        //rx data
        Uart1ProcessTag.RxBuf[Uart1ProcessTag.RxCnt++] = USART_ReceiveData(USART1);  
        
         //is head
        if(Uart1ProcessTag.RxBuf[FI_HEAD] != FIX_HEAD){
            Uart1ProcessTag.RxCnt = 0; //reset and throw it     
         //rx complement?
        }else if(Uart1ProcessTag.RxCnt >=FI_CMD &&
                  Uart1ProcessTag.RxCnt == Uart1ProcessTag.RxBuf[FI_LEN] +FIX_OFFSET){
            Uart1ProcessTag.RxCmplet = 1;
            return;
        }else{
            return;
        }
    
    }  
}  

void USART2_IRQHandler(void)  
{  
    USART_ClearFlag(USART2,USART_FLAG_TC );  
    if(USART_GetITStatus(USART2,USART_IT_RXNE)!=Bit_RESET)
    {    
            //rx data
            Uart2ProcessTag.RxBuf[Uart2ProcessTag.RxCnt++] = USART_ReceiveData(USART2);  
            
            
            //is head
            if(Uart2ProcessTag.RxBuf[FR_HEAD] != FIX_HEAD){
                Uart2ProcessTag.RxCnt = 0; //reset and throw it     
                
            //rx complement?
            }else if(Uart2ProcessTag.RxCnt >FR_CMD && Uart2ProcessTag.RxCnt == (Uart2ProcessTag.RxBuf[FR_LEN] +FIX_OFFSET+2)){
               // if(Uart2ProcessTag.RxBuf[FR_STA1] + Uart2ProcessTag.RxBuf[FR_STA2+1] != 0){                 
               //     memset(Uart2ProcessTag.RxBuf, 0, Uart2ProcessTag.RxCnt);
               //     Uart1ProcessTag.RxCnt = 0;
               // }else{
                    Uart2ProcessTag.RxCmplet = 1;
                //}
                return;
            }else{
                
                return;
            }                    
    }    
}  
/*******************************************************************************  
* 函 数 名         : uart_init  
* 函数功能         : IO端口及串口1，时钟初始化函数    A9,A10    
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/    
void uart1_init(u32 bt)    
{    
    GPIO_InitTypeDef GPIO_InitStructure;    //声明一个结构体变量，用来初始化GPIO    
    NVIC_InitTypeDef NVIC_InitStructure;     //中断结构体定义    
    USART_InitTypeDef  USART_InitStructure;   //串口结构体定义    

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO,ENABLE);    

    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;//TX    
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;    
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;    
    GPIO_Init(GPIOA,&GPIO_InitStructure);    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//RX    
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;    
    GPIO_Init(GPIOA,&GPIO_InitStructure);    


    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);     
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;     
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;     
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;     
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     
    NVIC_Init(&NVIC_InitStructure);    


    USART_InitStructure.USART_BaudRate=bt;   //波特率设置为bt    
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;    
    USART_InitStructure.USART_StopBits=USART_StopBits_1;    
    USART_InitStructure.USART_Parity=USART_Parity_No;    
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;    
    USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;    
    USART_Init(USART1,&USART_InitStructure);    
    USART_Cmd(USART1, ENABLE);    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断    
    USART_ClearFlag(USART1,USART_FLAG_TC);//清除USARTx的待处理标志位     
}    

/*******************************************************************************  
* 函 数 名         : uart2_init  
* 函数功能         : IO端口及串口2，时钟初始化函数     A2,A3   
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/    
void uart2_init(u32 bt)    
{    
     USART_InitTypeDef USART_InitStructure;    
  NVIC_InitTypeDef NVIC_InitStructure;     
    GPIO_InitTypeDef GPIO_InitStructure;    //声明一个结构体变量，用来初始化GPIO    
   //使能串口的RCC时钟    
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE); //使能UART3所在GPIOB的时钟    
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);    

   //串口使用的GPIO口配置    
   // Configure USART2 Rx (PB.11) as input floating      
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    
   GPIO_Init(GPIOA, &GPIO_InitStructure);    

   // Configure USART2 Tx (PB.10) as alternate function push-pull    
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;    
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    
   GPIO_Init(GPIOA, &GPIO_InitStructure);    

   //配置串口    
   USART_InitStructure.USART_BaudRate = bt;    
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;    
   USART_InitStructure.USART_StopBits = USART_StopBits_1;    
   USART_InitStructure.USART_Parity = USART_Parity_No;    
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    


   // Configure USART3     
   USART_Init(USART2, &USART_InitStructure);//配置串口3    

  // Enable USART1 Receive interrupts 使能串口接收中断    
   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);    
   //串口发送中断在发送数据时开启    
   //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);    

   // Enable the USART3     
   USART_Cmd(USART2, ENABLE);//使能串口3    

   //串口中断配置    
   //Configure the NVIC Preemption Priority Bits       
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);    

   // Enable the USART3 Interrupt     
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;    
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
   NVIC_Init(&NVIC_InitStructure);    
}    



/*******************************************************************************  
* 函 数 名         : uart1_send_char  
* 函数功能         : 串口1发送一字节        
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/    
void uart1_send_char(u8 temp)      
{     
    USART_SendData(USART1,(u8)temp);        
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);          
}    

/*******************************************************************************  
* 函 数 名         : uart1_send_buff  
* 函数功能         : 串口1发送一字符串       
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/    
void uart1_send_buff(u8 buf[],u32 len)         
{    
    u32 i;    
    for(i=0;i<len;i++)    
    uart1_send_char(buf[i]);      
}    

/*******************************************************************************  
* 函 数 名         : uart2_send_char  
* 函数功能         : 串口2发送一字节        
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/    
void uart2_send_char(u8 temp)      
{        
    USART_SendData(USART2,(u8)temp);        
    while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);         
}    



/*******************************************************************************  
* 函 数 名         : uart2_send_buff  
* 函数功能         : 串口2发送一字符串       
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/    
void uart2_send_buff(u8 buf[],u32 len)     
{    
    u32 i;    
    for(i=0;i<len;i++)    
    uart2_send_char(buf[i]);         
}    
