/* ###################################################################
**     Filename    : main.c
**     Project     : exp2
**     Processor   : MKL25Z128VLK4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2020-12-25, 03:05, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "ESP.h"
#include "ASerialLdd1.h"
#include "PC.h"
#include "ASerialLdd2.h"
#include "RTC.h"
#include "AD.h"
#include "AdcLdd1.h"
#include "TI.h"
#include "TimerIntLdd1.h"
#include "TU1.h"
#include "B1.h"
#include "BitIoLdd1.h"
#include "B2.h"
#include "BitIoLdd2.h"
#include "PW.h"
#include "PwmLdd1.h"
#include "TU2.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
volatile uint8_t i = 0; /**<@brief Índice para posições de arrays em geral. */
volatile uint8_t j = 0; /**<@brief Índice para posições de arrays em geral. */

volatile char pMsg[100] = {0}; /**<@brief Mensagem do PC a ser enviada ao módulo ESP, com máximo de 100 caracteres. */
volatile char eMsg[100] = {0}; /**<@brief Mensagem recebida pelo módulo ESP, com máximo de 100 caracteres. */
volatile char dMsg[30] = {0}; /**<@brief Mensagem com informações de data, hora e temperatura a ser mostrada no terminal produzida na interrupção TI_OnInterrupt(). */
volatile char tMsg[4] = {0}; /**<@brief Mensagem com informação de temperatura, a ser enviada ao tópico /temp produzida na interrupção TI_OnInterrupt(). */

volatile char pFlag = 0; /**<@brief Flag de mensagem do PC pronta. */
volatile char eFlag = 0; /**<@brief Flag de mensagem do ESP pronta. */
volatile char tFlag = 0; /**<@brief Flag de mensagem da interrupção temporária TI_OnInterrupt() pronta. */
char dFlag = 0; /**<@brief Flag que indica a direção do motor, conforme instrução recebida no tópico /dir. */
char mFlag = 0; /**<@brief Flag que indica o modo de regime do motor, conforme instrução recebida no tópico /mode. */

char cStr[20] = {0}; /**<@brief Substring de comando pertencente à mensagem recebida pelo ESP (Ex.: "MENSAGEM"). */
char tStr[50] = {0}; /**<@brief Substring de tópico pertencente à mensagem recebida pelo ESP. */
char pStr[10] = {0}; /**<@brief Substring de parâmetro pertencente à mensagem recebida pelo ESP. */

volatile uint16_t bits; /**<@brief Variável onde será alocado o valor discreto convertido pelo conversor AD entre 0 e 65535; */
volatile uint16_t temp; /**<@brief Variável onde será alocado numericamente o valor de temperatura após a conversão de bits para temperatura. */
volatile uint16_t lim; /**<@brief Variável que recebe o valor de temperatura recebido como parâmetro no tópico /limiar. */
volatile uint8_t duty; /**<@brief Variável que contém o valor do duty-cycle do PWM entre 0 e 100. */
volatile uint8_t aux; /**<@brief Variável auxiliar para o cálculo do duty-cycle. */

volatile LDD_RTC_TTime MyTime; /**<@brief Estrutura que recebe os valores do módulo RTC. */
volatile LDD_TDeviceData * MyRTC; /**<@brief Ponteiro utilizado na inicialização do módulo RTC e ao utilizar a função RTC_GetTime(). */

/**
 * @brief 
 * Envia a string colocada como parâmetro para o terminal do PC 
 * por meio de uma varredura em todos os caracteres da string
 * e enviando-os utilizando a função PC_SendChar() gerada pelo Processor Expert.
 * */
void SendP(char *s) {
	while(*s) {
		while(PC_SendChar(*s));
		s++;
	}
}

/**
 * @brief 
 * Envia a string colocada como parâmetro para o módulo ESP 
 * por meio de uma varredura em todos os caracteres da string
 * e enviando-os utilizando a função ESP_SendChar() gerada pelo Processor Expert.
 * */
void SendE(char *s) {
	while(*s) {
		while(ESP_SendChar(*s));
		s++;
	}
}

/** 
 * @brief
 * Realiza a inicialização automática utilizando a sintaxe do protocolo MQTT, ou seja,
 * envia iterativamente os seguintes comandos ao módulo ESP na sequência abaixo:
 * 
 * 1. Conecta-se ao Wi-Fi;
 * 2. Conecta-se ao MQTT;
 * 3. Inscreve-se no tópico /dir;
 * 4. Inscreve-se no tópico /mode;
 * 5. Inscreve-se no tópico /limiar;
 * 6. Inscreve-se no tópico /power;
 * 
 * Em seguida, envia para o terminal do PC uma mensagem de configurações prontas.
 * Caso aconteça falha em algum dos passos acima listados, uma tentativa é feita
 * novamente desde o início a partir da próxima iteração.
 * */
void AutoInit(void) {
	int i = 0;
	while(1) {
		if(i == 0) {
			SendE("CONNWIFI \"wifi-network\",\"wifi-password\"\r\n");
		} else if(i == 1) {
			SendE("CONNMQTT \"host-name\",1883,\"mac-address\"\r\n");
		} else if(i == 2) {
			SendE("SUBSCRIBE \"EA076/166974/dir\"\r\n");
		} else if(i == 3) {
			SendE("SUBSCRIBE \"EA076/166974/mode\"\r\n");
		} else if(i == 4) {
			SendE("SUBSCRIBE \"EA076/166974/limiar\"\r\n");
		} else if(i == 5) {
			SendE("SUBSCRIBE \"EA076/166974/power\"\r\n");
		} else if(i == 6) {
			SendP("SETTINGS READY\r\n");
			break;
		}
		
		while(eFlag == 0);
		eFlag = 0;
		SendP(eMsg);
		
		if(!strcmp(eMsg, "CONNECT WIFI\r\n") || !strcmp(eMsg, "CONNECT MQTT\r\n") || !strcmp(eMsg, "OK SUBSCRIBE\r\n")) {
			i++;
		} else if(!strcmp(eMsg, "ERROR WIFI\r\n") || !strcmp(eMsg, "NO WIFI\r\n") || !strcmp(eMsg, "NOT CONNECTED\r\n")) {
			i = 0;
		}
	}
}

/**
 * @brief
 * Realiza uma varredura na mensagem recebida pelo módulo ESP,
 * repartindo-a em três substrings:
 * 
 * 1. Substring de comando (cStr);
 * 2. Substring de tópico (tStr);
 * 3. Substring de parâmetro (pStr);
 */
void ParseMsg(void) {
	i = 0;
	j = 0;
	while(eMsg[i] != ' ') {
		cStr[i] = eMsg[i]; 
		i++;
	}
	i++;
	cStr[i] = 0;
	i++;
	while(eMsg[i] != ']') {
		tStr[j] = eMsg[i];
		i++;
		j++;
	}
	tStr[j++] = 0;
	j = 0;
	while(eMsg[i] != '[') {
		i++;
	}
	i++;
	while(eMsg[i] != ']') {
		pStr[j] = eMsg[i];
		i++;
		j++;
	}
	pStr[j++] = 0;
	i = 0;
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */

/**
 * @brief
 * A função main realiza as seguintes funções:
 * 
 * 1. Inicializa os componentes do Processor Expert;
 * 2. Inicializa o módulo RTC;
 * 3. Inicializa automaticamente a conexão MQTT (Função AutoInit());
 * 4. Entra em um loop infinito, com as seguintes funções:
 * 	- Realiza, a cada 2 segundos, a interrupção TI_OnInterrupt() que mede a temperatura, extrai o horário atualizado do módulo RTC e os converte em strings com informação de data, hora e temperatura (dMsg e tMsg);
 * 	- Publica a string de data, hora e temperatura (dStr) no terminal e a string só de temperatura (tStr) no tópico /temp;
 * 	- Se uma mensagem é recebida pelo módulo ESP (Interrupção ESP_OnRxChar()), é realizada sua separação em strings de tópico e parâmetro (Função ParseMsg());
 * 	- Define um estado de operação do motor com base na configuração mais atualizada dos três fatores (modo, direção e potência) e os aplica nos pinos referentes;  
 */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
 {
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */
	MyRTC = RTC_Init(NULL, FALSE);
	AutoInit();
	
	while(1) {
		if(pFlag) {
			pFlag = 0;
			while(PC_SendChar(0x0A));
		}
		
		if(eFlag) {
			eFlag = 0;
			SendP(eMsg);
			
			if(strcmp(eMsg, "OK PUBLISH\r\n")) {
				ParseMsg();
				
				if(strcmp(cStr, "MESSAGE")) {
				} else if(!strcmp(tStr, "EA076/166974/dir") && !strcmp(pStr, "Vent")) {
					dFlag = 0;
				} else if(!strcmp(tStr, "EA076/166974/dir") && !strcmp(pStr, "Exhaust")) {
					dFlag = 1;
				} else if(!strcmp(tStr, "EA076/166974/mode") && !strcmp(pStr, "On")) {
					mFlag = 1;
				} else if(!strcmp(tStr, "EA076/166974/mode") && !strcmp(pStr, "Off")) {
					mFlag = 0;
				} else if(!strcmp(tStr, "EA076/166974/mode") && !strcmp(pStr, "Auto")) {
					mFlag = 2;
				} else if(!strcmp(tStr, "EA076/166974/power")) {
					duty = 100 - ((pStr[0] - 0x30) * 10 + (pStr[1] - 0x30));
				} else if(!strcmp(tStr, "EA076/166974/limiar")) {
					lim = 0;
					i = 0;
					while(1) {
						if(pStr[i] >= '0' && pStr[i] <= '9') {
							lim = (lim * 10) + (pStr[i] - 0x30);
							i++;
						} else if(pStr[i] == '.') {
							i++;
							lim = (lim * 10) + (pStr[i] - 0x30);
							break;
						} else if(pStr[i] == 0) {
							lim = lim * 10;
							break;
						}
					}
					i = 0;
				}
			}
		}
		
		if(tFlag) {
			tFlag = 0;
			
			SendP(dMsg);
			
			SendE("PUBLISH \"EA076/166974/temp\",\"");
			SendE(tMsg);
			SendE("\"\r\n");
		}
		
		if (mFlag == 0) {
			PW_SetDutyUS(10000);
		} else if (mFlag == 1) {
			PW_SetDutyUS(duty * 100);
		} else if (mFlag == 2) {
			if(temp <= lim) {
				PW_SetDutyUS(10000);
			} else {
				PW_SetDutyUS(duty * 100);
			}
		}
		
		if (dFlag) {
			B1_ClrVal();
			B2_SetVal();
		} else {
			B1_SetVal();
			B2_ClrVal();
		}		
	}
  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
