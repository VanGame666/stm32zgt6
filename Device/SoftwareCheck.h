#ifndef __SOFTWARECHECK_H
#define __SOFTWARECHECK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

uint32_t BitReverse32(uint32_t data);
uint16_t BitReverse16(uint16_t data);
uint32_t CharReverse32 (uint32_t data);
uint16_t CharReverse16 (uint16_t data);
uint8_t	 CheckSum8(uint8_t* data, uint16_t len);
uint16_t ModBusCRC16(uint8_t* data, uint16_t len);
uint32_t EthCRC32(uint8_t* data, uint16_t len);


#endif

#ifdef __cplusplus
}
#endif
