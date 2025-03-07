#ifndef __ProtocolProcessing_H
#define __ProtocolProcessing_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usart.h"
#include "spi.h"
#include "SoftwareCRC.h"

#define BUF_SIZE 128

#define DACAI_SEND(command, ...) Dacai_Send(command,##__VA_ARGS__, 0xFFFF)

#define URTSEND huart1
#define SPISEND hspi2


extern uint8_t rx_buffer[BUF_SIZE];
extern uint8_t tx_buffer[BUF_SIZE];
extern uint8_t rx_num;
extern uint8_t tx_num;


/* Host computer communication protocol format */
typedef struct{
	uint8_t mode;
	uint8_t data_len;
	uint16_t addr;
	uint16_t addr_num;
	uint16_t crc16;
}PConectTypeDef;
extern PConectTypeDef PConect;

/* Dacai screen control command */
typedef enum{
	 ReadButtonStatus = 0xB111,
	 SetButtonStatus = 0xB110,
	 SwitchScreen = 0xB100,
}CmdSlectTypeDef;
extern CmdSlectTypeDef Cmd;

/* Send link selection */
typedef enum{
	UART,
	SPI,
	IIC
}SendSlectTypeDef;
extern SendSlectTypeDef SendSlect;

/* Check method selection */
typedef enum{
	CHECKSUM8 = 1,
	MODBUSCRC16 = 2,
	ETHCRC32 = 4,
}CheckSlectTypeDef;
extern CheckSlectTypeDef CheckSlect;



void Daicai_Decode(void);
void DacaiSend(uint16_t command, ...);

void DDSend(uint8_t enable,uint32_t frequecy,uint8_t channel, float phase);
void PConectProcess(void);


#endif

#ifdef __cplusplus
}
#endif
