#include "SoftwareCRC.h"

uint32_t BitReverse32(uint32_t data)
{
	uint32_t revers = 0;
    for (int i = 0; i < 32; i++) {
    	revers = revers << 1;
    	revers = revers | (data & 0x00000001);
        data >>= 1;
    }
    return revers;
}

uint16_t BitReverse16(uint16_t data)
{
	uint16_t revers = 0;
    for (int i = 0; i < 16; i++) {
    	revers = revers << 1;
    	revers = revers | (data & 0x0001);
        data >>= 1;
    }
    return revers;
}

uint32_t CharReverse32 (uint32_t data)
{
	return ((data & 0x000000ff) << 24 | (data & 0x0000ff00) << 8 | (data & 0x00ff0000) >> 8 | (data & 0xff000000) >> 24);
}

uint16_t CharReverse16 (uint16_t data)
{
	return (data & 0x00ff) << 8 | (data & 0xff00) >> 8;
}

uint32_t EthCRC32(uint8_t* data, uint16_t len)
{
	uint32_t CRC32 = 0xFFFFFFFF;
	uint32_t POLY = BitReverse32(0x04C11DB7);

	for(int i = 0; i < len; i++)
	{
		CRC32 = CRC32 ^ (data[i]);

		for(int j = 0; j < 8; j++)  /*此处的8 -- 指每一个char类型有8bit，每bit都要处理*/
		{
			if(CRC32 & 0x00000001)
			{
				CRC32 = CRC32 >> 1;
				CRC32 = CRC32 ^ POLY;   //多项式0x04c11db7的倒序后是0xedb88320
			}else{
				CRC32 = CRC32 >> 1;
			}
    }
  }
  return CRC32 ^ 0xFFFFFFFF;
}


uint16_t ModBusCRC16(uint8_t *data, uint16_t len)
{
    uint16_t CRC16 = 0xFFFF;
    uint16_t POLY = BitReverse16(0x8005);

    for (int i = 0; i < len; i++)
    {
        CRC16 = CRC16 ^ (data[i]);

        for (int j = 0; j < 8; j++)
        {
            if (CRC16 & 0x0001)
            {
                CRC16 = CRC16 >> 1;
                CRC16 = CRC16 ^ POLY;
            }else{
				CRC16 = CRC16 >> 1;
			}
        }
    }

    return CRC16;
}


uint8_t CheckSum8(uint8_t *data, uint16_t len)
{
    uint32_t SUM = 0;

    for (int i = 0; i < len; i++)
    {
    	SUM = SUM + data[i];
    }

    return SUM & 0x000000FF;
}

