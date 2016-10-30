#include <stddef.h>
#include "stm32l1xx.h"
#include "vrs_cv5.h"


extern uint32_t AD_value;
extern uint8_t Send;
extern char* TX_Buffer;

int main(void)
{
	float pom;
	char txt[10];
	Send=1;
	GPIO_Inicializacia();
	ADC_Inicializacia();
	USART_Inicializacia();
	while (1)
	{
		if(Send==0)
				{
					pom=AD_value*330/4096;
					sprintf(txt,"%d.%dV",(int)pom/100,(int)pom%100);
				}
		else
				{
					sprintf(txt,"%d",AD_value);
				}


				TX_Buffer=txt;
				USART_ITConfig(USART2,USART_IT_TXE,ENABLE);
				Delay(1000000);

	}
	return 0;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*
 * Minimal __assert_func used by the assert() macro
 * */
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  while(1)
  {}
}

/*
 * Minimal __assert() uses __assert__func()
 * */
void __assert(const char *file, int line, const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
}
