#ifndef __SOFTWARECRC_H
#define __SOFTWARECRC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

extern uint32_t BitReverse32(uint32_t data);
extern uint16_t BitReverse16(uint16_t data);
extern uint32_t CharReverse32 (uint32_t data);
extern uint16_t CharReverse16 (uint16_t data);
extern uint32_t EthCRC32(uint8_t* data, uint16_t len);
extern uint16_t ModBusCRC16(uint8_t *data, uint16_t len);
extern uint8_t 	CheckSum8(uint8_t *data, uint16_t len);


#endif

#ifdef __cplusplus
}
#endif
