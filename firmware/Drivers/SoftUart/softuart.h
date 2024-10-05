/*
 Software Uart For Stm32
 By Liyanboy74
 https://github.com/liyanboy74
*/

#include "main.h"

#define 	Number_Of_SoftUarts 	1 	// Max 8

#define 	SoftUartTxBufferSize	256
#define 	SoftUartRxBufferSize	64

#define 	SoftUart_DATA_LEN   	8 	// Max 8 Bit
#define 	SoftUart_PARITY     	0   // 0=None 1=odd 2=even
#define 	SoftUart_STOP_Bit   	1   // Number of stop bits

typedef enum {
	SoftUart_OK,
	SoftUart_Error
}SoftUartState_E;

typedef struct{
	uint8_t			Tx[SoftUartTxBufferSize];
	uint8_t			Rx[SoftUartRxBufferSize];
}SoftUartBuffer_S;

typedef struct {
	__IO uint8_t 		TxNComplated;
	uint8_t			TxEnable;
	uint8_t			RxEnable;
	uint8_t 		TxBitShift,TxBitCounter;
	uint8_t 		RxBitShift,RxBitCounter;
	uint8_t			TxIndex,TxSize;
	uint8_t			RxIndex;
	SoftUartBuffer_S	*Buffer;
	GPIO_TypeDef  		*TxPort;
	uint16_t 		TxPin;
	GPIO_TypeDef  		*RxPort;
	uint16_t 		RxPin;
	uint8_t 		RxTimingFlag;
	uint8_t 		RxBitOffset;
} SoftUart_S;

//Call Every (0.2)*(1/9600) = 20.83 uS
void 		SoftUartHandler(void);

void 		SoftUartWaitUntilTxComplate(uint8_t SoftUartNumber);
uint8_t 	SoftUartRxAlavailable(uint8_t SoftUartNumber);
SoftUartState_E SoftUartPuts(uint8_t SoftUartNumber,uint8_t *Data,uint8_t Len);
SoftUartState_E SoftUartEnableRx(uint8_t SoftUartNumber);
SoftUartState_E SoftUartDisableRx(uint8_t SoftUartNumber);
SoftUartState_E SoftUartInit(uint8_t SoftUartNumber,GPIO_TypeDef *TxPort,uint16_t TxPin,GPIO_TypeDef *RxPort,uint16_t RxPin);
SoftUartState_E SoftUartReadRxBuffer(uint8_t SoftUartNumber,uint8_t *Buffer,uint8_t Len);
