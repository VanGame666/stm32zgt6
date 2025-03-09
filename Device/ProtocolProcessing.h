
#ifndef __ProtocolProcessing_H
#define __ProtocolProcessing_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usart.h"
#include "spi.h"
#include <SoftwareCheck.h>

#define BUF_SIZE 128


#define HEAD_VERIFICATION(head) head_verification(head,sizeof(head))
#define TIAL_VERIFICATION(tail) tail_verification(tail,sizeof(tail))
#define FRAME_SEND(SendSlect,CheckSlect,head,tail) frame_send(SendSlect,CheckSlect,head,tail,sizeof(head),sizeof(tail))

#define URTSEND huart1
#define SPISEND hspi2

#define DACAI_SEND(command, ...) Dacai_Send(command,##__VA_ARGS__, 0xFFFF)


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

/* Dacai screen control command */
typedef enum{
	 ReadButtonStatus = 0xB111,
	 SetButtonStatus = 0xB110,
	 SwitchScreen = 0xB100,
}CmdSlectTypeDef;


/* Send link selection */
typedef enum{
	UART,
	SPI,
	IIC
}SendSlectTypeDef;

/* Check method selection */
typedef enum{
	NOCHECK = 0,
	CHECKSUM8 = 1,
	MODBUSCRC16 = 2,
	ETHCRC32 = 4,
}CheckSlectTypeDef;


extern CheckSlectTypeDef CheckSlect;
extern SendSlectTypeDef SendSlect;
extern CmdSlectTypeDef Cmd;
extern PConectTypeDef PConect;


void Daicai_Decode(void);
void DacaiSend(uint16_t command, ...);

void DDSend(uint8_t enable,uint32_t frequecy,uint8_t channel, float phase);
void PConectProcess(void);


#endif

#ifdef __cplusplus
}
#endif
