 
#include "stm32f10x_gpio.h"
#include <string.h>
#include <stdio.h>  
#include "process.h"
#include "uart.h"   


int fputc(int ch,FILE *p)       //��ʹ��printfʱϵͳ�Զ����ô˺���    
{    
    USART_SendData(USART2,(u8)ch);      
    while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);    
    return ch;    

}


void USART1_IRQHandler(void)  
{  
    USART_ClearFlag(USART1,USART_FLAG_TC);  
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=Bit_RESET)//���ָ����USART�жϷ������  
    {           
        //1rx data
        Uart1ProcessTag.RxBuf[Uart1ProcessTag.RxCnt++] = USART_ReceiveData(USART1);  
			  //2 is head ?
			  if(Uart1ProcessTag.RxBuf[FP_HEAD] != PRI_FIX_HEAD ){
				    Uart1ProcessTag.RxCnt = 0;
					  return;
				}			
         //3 time out
        if(Uart1ProcessTag.RxCnt == 1){
            Uart1ProcessTag.TimerOut = Tim2ProcessTag.val; 							
         //rx complement?
        }else if(Uart1ProcessTag.RxCnt >= FP_CMD &&
            Uart1ProcessTag.RxCnt == Uart1ProcessTag.RxBuf[FP_LEN] +FIX_PRI_OFFSET){
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
            
            
			       //3 time out
             if(Uart1ProcessTag.RxCnt == 1){
                Uart2ProcessTag.TimerOut = Tim2ProcessTag.val; 							
                
             }
            //is head
            if(Uart2ProcessTag.RxBuf[FR_HEAD] != FR_FIX_HEAD){
                Uart2ProcessTag.RxCnt = 0; 
							  return;
                
            //rx complement?
            }else if(Uart2ProcessTag.RxCnt > FR_PLLSB &&
  						Uart2ProcessTag.RxCnt == (Uart2ProcessTag.RxBuf[FR_PLLSB] + FIX_FR_OFFSET)
						  && Uart2ProcessTag.RxBuf[FR_CMD] == 0x22
						  && Uart2ProcessTag.RxBuf[FR_TYPE] == 0x02
						  ){
               // if(Uart2ProcessTag.RxBuf[FR_STA1] + Uart2ProcessTag.RxBuf[FR_STA2+1] != 0){                 
               //     memset(Uart2ProcessTag.RxBuf, 0, Uart2ProcessTag.RxCnt);
               //     Uart1ProcessTag.RxCnt = 0;
               // }else{
                    Uart2ProcessTag.RxCmplet = 1;
								    TagProcessTag.SysMode = SYS_READING ;
                //}
                return;
            }else{
                
                return;
            }                    
    }    
}  
/*******************************************************************************  
* �� �� ��         : uart_init  
* ��������         : IO�˿ڼ�����1��ʱ�ӳ�ʼ������    A9,A10    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/    
void uart1_init(u32 bt)    
{    
    GPIO_InitTypeDef GPIO_InitStructure;    //����һ���ṹ�������������ʼ��GPIO    
    NVIC_InitTypeDef NVIC_InitStructure;     //�жϽṹ�嶨��    
    USART_InitTypeDef  USART_InitStructure;   //���ڽṹ�嶨��    

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


    USART_InitStructure.USART_BaudRate=bt;   //����������Ϊbt    
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;    
    USART_InitStructure.USART_StopBits=USART_StopBits_1;    
    USART_InitStructure.USART_Parity=USART_Parity_No;    
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;    
    USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;    
    USART_Init(USART1,&USART_InitStructure);    
    USART_Cmd(USART1, ENABLE);    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//ʹ�ܻ���ʧ��ָ����USART�ж� �����ж�    
    USART_ClearFlag(USART1,USART_FLAG_TC);//���USARTx�Ĵ������־λ     
}    

/*******************************************************************************  
* �� �� ��         : uart2_init  
* ��������         : IO�˿ڼ�����2��ʱ�ӳ�ʼ������     A2,A3   
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/    
void uart2_init(u32 bt)    
{    
     USART_InitTypeDef USART_InitStructure;    
  NVIC_InitTypeDef NVIC_InitStructure;     
    GPIO_InitTypeDef GPIO_InitStructure;    //����һ���ṹ�������������ʼ��GPIO    
   //ʹ�ܴ��ڵ�RCCʱ��    
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE); //ʹ��UART3����GPIOB��ʱ��    
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);    

   //����ʹ�õ�GPIO������    
   // Configure USART2 Rx (PB.11) as input floating      
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    
   GPIO_Init(GPIOA, &GPIO_InitStructure);    

   // Configure USART2 Tx (PB.10) as alternate function push-pull    
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;    
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    
   GPIO_Init(GPIOA, &GPIO_InitStructure);    

   //���ô���    
   USART_InitStructure.USART_BaudRate = bt;    
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;    
   USART_InitStructure.USART_StopBits = USART_StopBits_1;    
   USART_InitStructure.USART_Parity = USART_Parity_No;    
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    


   // Configure USART3     
   USART_Init(USART2, &USART_InitStructure);//���ô���3    

  // Enable USART1 Receive interrupts ʹ�ܴ��ڽ����ж�    
   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);    
   //���ڷ����ж��ڷ�������ʱ����    
   //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);    

   // Enable the USART3     
   USART_Cmd(USART2, ENABLE);//ʹ�ܴ���3    

   //�����ж�����    
   //Configure the NVIC Preemption Priority Bits       
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);    

   // Enable the USART3 Interrupt     
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;    
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
   NVIC_Init(&NVIC_InitStructure);    
}    



/*******************************************************************************  
* �� �� ��         : uart1_send_char  
* ��������         : ����1����һ�ֽ�        
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/    
void uart1_send_char(u8 temp)      
{     
    USART_SendData(USART1,(u8)temp);        
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);          
}    

/*******************************************************************************  
* �� �� ��         : uart1_send_buff  
* ��������         : ����1����һ�ַ���       
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/    
void uart1_send_buff(u8 buf[],u32 len)         
{    
    u32 i;    
    for(i=0;i<len;i++)    
    uart1_send_char(buf[i]);      
}    

/*******************************************************************************  
* �� �� ��         : uart2_send_char  
* ��������         : ����2����һ�ֽ�        
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/    
void uart2_send_char(u8 temp)      
{        
    USART_SendData(USART2,(u8)temp);        
    while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);         
}    



/*******************************************************************************  
* �� �� ��         : uart2_send_buff  
* ��������         : ����2����һ�ַ���       
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/    
void uart2_send_buff(u8 buf[],u32 len)     
{    
    u32 i;    
    for(i=0;i<len;i++)    
    uart2_send_char(buf[i]);         
}    
