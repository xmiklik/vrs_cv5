#include <stddef.h>
#include "stm32l1xx.h"
#include "vrs_cv5.h"

GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef nvicInitStructure;
ADC_InitTypeDef ADC_InitStructure;
extern uint32_t AD_value;

void Delay (int x){
	for(int i=0;i<x;i++){}
}

void GPIO_Inicializacia()
{
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);

	/* Nastavanie ADC kanala */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_40MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

}

void ADC_Inicializacia()
{

	/* Enable ADC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	/* Enable the HSI oscillator */
	RCC_HSICmd(ENABLE);
	/* Check that HSI oscillator is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADCx regular channel8 configuration */
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_16Cycles);
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
	ADC_ITConfig(ADC1,ADC_IT_OVR,ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Nastavanie NVIC pre prerušenie na ADC1*/
	nvicInitStructure.NVIC_IRQChannel=ADC1_IRQn;
	nvicInitStructure.NVIC_IRQChannelPreemptionPriority=0;
	nvicInitStructure.NVIC_IRQChannelSubPriority=0;
	nvicInitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&nvicInitStructure);

	/* Enable the ADC */
	ADC_Cmd(ADC1,ENABLE);
	/* Wait until the ADC1 is ready */
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_ADONS)==RESET);
	{
	}
	ADC_SoftwareStartConv(ADC1);

}

void ADC1_IRQHandler()
{
	if(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC))
	{
		AD_value=ADC_GetConversionValue(ADC1);
	}

	if(ADC_GetFlagStatus(ADC1,ADC_FLAG_OVR))
	{
		ADC_ClearFlag(ADC1,ADC_FLAG_OVR);
	}
}



