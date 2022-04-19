/* ###################################################################
**     Filename    : Events.c
**     Project     : exp2
**     Processor   : MKL25Z128VLK4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2020-12-25, 03:05, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         Cpu_OnNMIINT - void Cpu_OnNMIINT(void);
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */

/*
** ===================================================================
**     Event       :  Cpu_OnNMIINT (module Events)
**
**     Component   :  Cpu [MKL25Z128LK4]
/* ===================================================================*/
/** @brief Interrupção não utilizada. */
void Cpu_OnNMIINT(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  PC_OnError (module Events)
**
**     Component   :  PC [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/** @brief Interrupção não utilizada. */
void PC_OnError(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  PC_OnRxChar (module Events)
**
**     Component   :  PC [AsynchroSerial]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/**
 * @brief
 * Esta interrupção é acionada toda vez que um caracter é digitado,
 * realizando as seguintes funções:
 * 
 * 1. Envia o caracter para o terminal do PC;
 * 2. Se o caracter for numérico, será o novo dígitomenos significativo do valor de PWM até que a tecla Enter for pressionada;
 * 3. Se o caracter for 'R' ou 'L', define que o sentido de rotação (Horário ou anti-horário) do motor assim que a tecla Enter for pressionada . 
*/
void PC_OnRxChar(void)
{
  /* Write your code here ... */
	uint8_t c;
	extern volatile uint8_t aux;
	extern volatile uint8_t duty;
	extern volatile char pFlag;
	
	PC_RecvChar(&c);
	while(PC_SendChar(c));
	
	if(c >= '0' && c <= '9') {
		aux = (aux * 10) + (c - 0x30);
	} else if(c == 'R') {
		aux = 101;
	} else if(c == 'L') {
		aux = 102;
	} else if(c == 0x0D) {
		if(aux <= 100) {
			duty = 100 - aux;
			PW_SetDutyUS(duty * 100);
		} else if(aux == 101) {
			B1_SetVal();
			B2_ClrVal();
		} else if(aux == 102) {
			B1_ClrVal();
			B2_SetVal();
		}
		aux = 0;
		pFlag = 1;
	}
}

/*
** ===================================================================
**     Event       :  PC_OnTxChar (module Events)
**
**     Component   :  PC [AsynchroSerial]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/** @brief Interrupção não utilizada. */
void PC_OnTxChar(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  ESP_OnError (module Events)
**
**     Component   :  ESP [AsynchroSerial]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/** @brief Interrupção não utilizada. */
void ESP_OnError(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  ESP_OnRxChar (module Events)
**
**     Component   :  ESP [AsynchroSerial]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/**
 * @brief
 * Esta interrupção é acionada toda vez que um caracter é digitado, realizando as seguintes funções:
 * 
 * 1. Coloca o caracter na próxima posição vazia do buffer de mensagem do ESP (eMsg). 
 * Se o caracter recebido for um Carriage Return, adiciona o Line Feed na próxima posição, e finaliza a string com valor nulo;
 * 2. Aciona a flag de mensagem pronta (eFlag).
*/
void ESP_OnRxChar(void)
{
  /* Write your code here ... */
	extern volatile uint8_t i;
	extern volatile char eMsg[100];
	extern volatile char eFlag;
	uint8_t c;
	
	ESP_RecvChar(&c);
	eMsg[i] = c;
	i++;
	if(c == 0x0A){
		eMsg[i] = 0;
		eFlag = 1;
		i = 0;
	}
}

/*
** ===================================================================
**     Event       :  ESP_OnTxChar (module Events)
**
**     Component   :  ESP [AsynchroSerial]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/** @brief Interrupção não utilizada. */
void ESP_OnTxChar(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  TI_OnInterrupt (module Events)
**
**     Component   :  TI [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
/**
 * @brief
 * Esta interrupção é acionada a cada 2 segundos, realizando as seguintes funções:
 * 
 * 1. Realiza uma amostragem do valor de temperatura pelo módulo AD e converte-a em um valor de temperatura;
 * 2. Extrai a data e hora mais atualizada do módulo RTC;
 * 3. Cria a string dStr a partir dos valores medidos de temperatura e de data e hora para ser publicada no terminal ao fim da interrupção;
 * 4. Cria a string tStr a partir do valor de temperatura para ser publicada no tópico /temp ao fim da interrupção;
 * 5. Aciona a flag tFlag, que atua no chamamento das publicações de dStr e tStr na função main.
*/
void TI_OnInterrupt(void)
{
  /* Write your code here ... */
	extern volatile LDD_RTC_TTime MyTime;
	extern volatile LDD_TDeviceData * MyRTC;
	extern volatile uint16_t bits;
	extern volatile uint16_t temp;
	extern volatile char dMsg[30];
	extern volatile char tMsg[5];
	extern volatile char tFlag;
	
	RTC_GetTime(MyRTC, &MyTime);
	
	AD_Measure(1);
	AD_GetValue16(&bits);
	
	temp = ((3300 * bits)/65535) - 600;
	
	dMsg[0] = (MyTime.Day)/10 + 0x30;
	dMsg[1] = (MyTime.Day)%10 + 0x30;
	dMsg[2] = '/';
	dMsg[3] = (MyTime.Month)/10 + 0x30;
	dMsg[4] = (MyTime.Month)%10 + 0x30;
	dMsg[5] = '/';
	dMsg[6] = (MyTime.Year)/1000 + 0x30;
	dMsg[7] = ((MyTime.Year)%1000)/100 + 0x30;
	dMsg[8] = ((MyTime.Year)%100)/10 + 0x30;
	dMsg[9] = (MyTime.Year)%10 + 0x30;
	dMsg[10] = ' ';
	dMsg[11] = (MyTime.Hour)/10 + 0x30;
	dMsg[12] = (MyTime.Hour)%10 + 0x30;
	dMsg[13] = ':';
	dMsg[14] = (MyTime.Minute)/10 + 0x30;
	dMsg[15] = (MyTime.Minute)%10 + 0x30;
	dMsg[16] = ':';
	dMsg[17] = (MyTime.Second)/10 + 0x30;
	dMsg[18] = (MyTime.Second)%10 + 0x30;
	dMsg[19] = ',';
	dMsg[20] = ' ';
	dMsg[21] = temp/100 + 0x30;
	dMsg[22] = (temp%100)/10 + 0x30;
	dMsg[23] = '.';
	dMsg[24] = temp%10 + 0x30;
	dMsg[25] = ' ';
	dMsg[26] = 'C';
	dMsg[27] = 0x0D;
	dMsg[28] = 0x0A;
	
	tMsg[0] = temp/100 + 0x30;
	tMsg[1] = (temp%100)/10 + 0x30;
	tMsg[2] = '.';
	tMsg[3] = temp%10 + 0x30;
	
	tFlag = 1;
}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

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
