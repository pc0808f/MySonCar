/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    30-September-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8l10x.h"

/**
  * @addtogroup TIM2_OCActiveMode
  * @{
  */
/* Private defines -----------------------------------------------------------*/
#define CCR1_Val  ((uint16_t)15625)
#define CCR2_Val  ((uint16_t)7812)
/* Private function prototypes -----------------------------------------------*/
static void GPIO_Config(void);
static void TIM2_Config(void);
static void TIM4_Config(void);
static void AWU_Config(void);
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
__IO u32 tim4Tick=0; 
u32 delayCount;
u32 sleepCount;
u8  keyFlag=0;
u8  keyState=0;
u32  keyDelay=0;
u8  sleepFlag=0;
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
  u8 LED1State=0;
  u32 temp;
  u16 j;
  
  /* CLK  configuration -----------------------------------------*/
  CLK_DeInit();
  CLK_MasterPrescalerConfig (CLK_MasterPrescaler_HSIDiv1);  
  
  /* GPIO configuration -----------------------------------------*/
  GPIO_Config();  

  /* TIM2 configuration -----------------------------------------*/
  TIM2_Config();  
  
  /* TIM4 configuration -----------------------------------------*/
  TIM4_Config();  

  /* AWU configuration -----------------------------------------*/
  //AWU_Config();  
  
  enableInterrupts();
  
  
  delayCount=tim4Tick;
  sleepCount=tim4Tick;
  j=11428;
  TIM2_SetAutoreload (j);  
  while (1)
  {
    if(keyState==0)
    {
      if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)==RESET)
      {
        keyDelay=tim4Tick;
        keyState=1;
      }
    }
    else if(keyState==1)
    {
      if((tim4Tick-keyDelay)>20)
      {
        if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)==RESET)
        {
          keyState=2;
          keyFlag=1;
        }
        else
          keyState=0;
      }
    }
    else if(keyState==2)
    {
      if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)!=RESET)
        keyState=0;
    }
    if(keyFlag==1)
    {
      keyFlag=0;
      if(sleepFlag==0)
      {
        sleepFlag=1;
      }
      else
      {
        sleepFlag=0;
      }
    }

     if(LED1State==0)
     {
       
       if((tim4Tick-delayCount)>1)
       {
         temp=(tim4Tick-delayCount);
         delayCount=tim4Tick;
         if(j>5333)
         {
           j=j-((11428-5333  )/230)*temp;
           TIM2_SetAutoreload (j);
            if(j<=5333)
            {
              LED1State=1;
              GPIO_SetBits(GPIOB, GPIO_Pin_5);
            }
         }
         
       }
     }
     else
     {
       if((tim4Tick-delayCount)>1)
       {
         temp=(tim4Tick-delayCount);
         delayCount=tim4Tick;
         if(j<11428)
         {
           j=j+((11428-5333  )/100)*temp;
           TIM2_SetAutoreload (j);
            if(j>=11428)
            {
              LED1State=0;
              GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            }
         }
       }       
     }
     if((tim4Tick-sleepCount)>200)
     {
       if(sleepFlag==1)
       {
        //GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        //GPIO_Init(GPIOB, GPIO_Pin_5, GPIO_Mode_In_FL_No_IT);
        //GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);
        disableInterrupts();
        TIM4_DeInit();
        TIM2_DeInit();
        GPIO_DeInit(GPIOB);
        GPIO_DeInit(GPIOC);
        GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_In_PU_IT);
        enableInterrupts();
        halt();
        GPIO_Config();  
        TIM2_Config(); 
        TIM4_Config(); 
        sleepCount=tim4Tick;
        keyState=0;
       }
     }
    
  } 
}

/**r
  * @brief  Configure PG6 to allow delay of AWU channels computation
  * @param  None
  * @retval None
  */
static void AWU_Config(void)
{

  AWU_DeInit();
  AWU_Init(AWU_Timebase_No_IT);
}


static void GPIO_Config(void)
{
  /* Set PG.6 pin */
  GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_In_PU_IT);
  GPIO_Init(GPIOB, GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Slow);
  EXTI_SetPinSensitivity(EXTI_Pin_4,EXTI_Trigger_Falling);
}

/**
  * @brief  Configure TIM2 peripheral to generate 3 different signals with 3
  *         different delays
  * @param  None
  * @retval None
  */
static void TIM2_Config(void)
{
  /* Enable TIM2 clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);

  /* Config TIM2 Channel 1 and channel 2 pins */
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_High_Fast);

  /* Time base configuration */      
  TIM2_TimeBaseInit(TIM2_Prescaler_1, TIM2_CounterMode_Up, 5333);

  /* Prescaler configuration */
  TIM2_PrescalerConfig(TIM2_Prescaler_1, TIM2_PSCReloadMode_Immediate);

  /* Output Compare Active Mode configuration: Channel1 */
  /*
    TIM2_OCMode = TIM2_OCMODE_ACTIVE
    TIM2_OutputState = TIM2_OUTPUTSTATE_ENABLE
    TIM2_Pulse = CCR1_Val
    TIM2_OCPolarity = TIM2_OCPOLARITY_HIGH
  */
  TIM2_OC1Init(TIM2_OCMode_Toggle, TIM2_OutputState_Enable, 0, TIM2_OCPolarity_High, TIM2_OCIdleState_Reset);

  /* Output Compare Active Mode configuration: Channel2 */
  
  /* Enable TIM2 outputs */
  TIM2_CtrlPWMOutputs(ENABLE);
  
  /* TIM2 enable counter */
  TIM2_Cmd(ENABLE);
}


/**
  * @brief  Configure TIM2 peripheral to generate 3 different signals with 3
  *         different delays
  * @param  None
  * @retval None
  */
static void TIM4_Config(void)
{
  /* Enable TIM2 clock */
  CLK_PeripheralClockConfig (CLK_Peripheral_TIM4 , ENABLE); 

  TIM4_DeInit(); 
  
  /* Time base configuration */      
  TIM4_TimeBaseInit(TIM4_Prescaler_128, 0x7D); // 127 and 0x7D == Interrupt 1mS 
  
  TIM4_ClearFlag(TIM4_FLAG_Update);
  
  TIM4_ITConfig(TIM4_IT_Update, ENABLE); 
  
  TIM4_Cmd(ENABLE);    // Enable TIM4  
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
