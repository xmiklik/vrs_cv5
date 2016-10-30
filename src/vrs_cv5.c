
#include <stddef.h>
#include "stm32l1xx.h"
#include "vrs_cv5.h"

GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
ADC_InitTypeDef ADC_InitStructure;
USART_InitTypeDef USART_InitStructure;

uint32_t AD_value;
uint8_t Send;
char* TX_Buffer;

void Delay (int x){
	for(int i=0;i<x;i++){}
}

void GPIO_Inicializacia()
{
	/* Spusti hodiny na GPIOA periferii */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);

	/* Nastavanie ADC  */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_40MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/* Nastavenie USART2 */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/* Nastavenie AF pre obi dva GPIO porty */
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

}

void ADC_Inicializacia()
{

	/* Spusti hodiny na APB2 periferii */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	/* Spusti HSI oscilator */
	RCC_HSICmd(ENABLE);
	/* Zisti ci je HSI oscilator pripraveny */
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	/* Inicializacia ADC struktury */
	ADC_StructInit(&ADC_InitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* Nastavenie vybraneho ADC kanala */
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_16Cycles);
	/* Aktivuje prerusenie EOC pre AD prevodnik */
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
	/* Aktivuje prerusenie OVR pre ADC*/
	ADC_ITConfig(ADC1,ADC_IT_OVR,ENABLE);

	/* Nastavenie skupiny priorit pre prerusenie*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Nastavanie NVIC pre prerusenie na ADC1*/
	NVIC_InitStructure.NVIC_IRQChannel=ADC1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Spusti ADC */
	ADC_Cmd(ADC1,ENABLE);
	/* Cakaj kym ADC1 nie je pripraveny */
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_ADONS)==RESET);
	{
	}
	ADC_SoftwareStartConv(ADC1);	// Spusti konverziu

}

void USART_Inicializacia()
{
	/* Nastavenie hodin pre USART2*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

	/* Nastavenie USART struktury podla zadania*/
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* Nastavi USART2 podla nadefinovanych parametrov v &USART_InitStructure*/
	USART_Init(USART2,&USART_InitStructure);
	/* Zapne prerusenie RXNE pre USART2*/
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;
	/* Zvolena nizsia priorita ako pri ADC*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART2,ENABLE);
}

/* Funkcia PosliSlovo sluzi na posielanie hodnoty "text" cez USART2.
 * Znaky sa posielaju kym sa nenastavi flag TC(transmission complete).
 * Po odoslani vsetkych znakov sa nastavi TC a odriadkuje. */
void PosliSlovo(char *text)
{
	uint8_t i=0;
	while(text[i])
	{
		USART_SendData(USART2,text[i]);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);
		i++;
	}
	USART_SendData(USART2,'\n');
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);
	USART_SendData(USART2,'\r');
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);
}

/* Obsluzna funkcia AD prevodnika. Po skonceni konverzie sa hodnota
 * z ADC1 ulozi do AD_value. V pripade, ze sa aktivuje flag Overrun
 * bude nasledne vymazany.*/
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

/* Obsluzna funkcia pre USART2. Ak datovy register pre prijimanie
 * nie je prazdny t.j. nieco bolo prijate cez USART2 spusti sa flag RXNE.
 * Ak prijaty znak je "m" zneguje sa premenna "Send" => zmeni sa format
 * odosielania dat z ADC cez UART (vid main.c)*/
void USART2_IRQHandler()
{
	if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE))
	{
		if(USART_ReceiveData(USART2)=='m')
		{
			Send=!Send;
		}
	}
	/* Ak sa nastavi flag TXE zacnu sa odosielat znaky "*TX_Buffer"
	 * Ak narazi na koniec pola skonci odosielanie a vypne sa flag
	 * TXE.*/
	if(USART_GetFlagStatus(USART2,USART_FLAG_TXE))
		{
			if((*TX_Buffer)!=0)
			{
				USART_SendData(USART2,*TX_Buffer);
				TX_Buffer++;
			}
			else USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
	}
}
