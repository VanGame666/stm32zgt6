/************************************************** START **************************************************/

#include "AT24C02.h"


uint8_t AT24CXX_ReadBuff[AT24CXX_BUFF_SIZE];
uint8_t AT24CXX_WriteBuff[AT24CXX_BUFF_SIZE];

FaultHeader_t FaultHeader;
FaultRecord_t FaultRecord;


void struct_regist(void)
{
	FaultHeader_t *pStates = (FaultHeader_t*)AT24CXX_ReadBuff[1];

	FaultHeader.err_time = pStates->err_time;
	FaultHeader.err_data = pStates->err_data;
	FaultHeader.fault_count = pStates->fault_count;
	FaultHeader.read_addr = pStates->read_addr;
	FaultHeader.write_addr = pStates->write_addr;
}
/************************************************** Time encoding and decoding **************************************************/
void time_decode(void)
{
	uint8_t time_hour =  (FaultHeader.err_time & 0xFF00) >> 8;
	uint8_t time_min =  (FaultHeader.err_time & 0x00ff);
	printf("\r\nnow time:hour %d min: %d\r\n",time_hour,time_min);
}

uint16_t time_code(uint8_t hour,uint8_t min)
{
	uint16_t time_hour =  (uint16_t)hour << 8;
	uint16_t time_min =  (uint16_t)min;
	uint16_t time_code =  time_hour|time_min;
	printf("now time code :%4x \r\n",time_code);
	return time_code;
}

void data_decode(void)
{
	uint8_t data_year =  (FaultHeader.err_data & 0xFE00) >> 9;
	uint8_t data_month =  (FaultHeader.err_data & 0x01E0) >> 5;
	uint8_t data_day =  (FaultHeader.err_data & 0x001F);
	printf("\r\nnow data:year %d month: %d day: %d\r\n",data_year+1980,data_month,data_day);
}

uint16_t data_code(uint8_t year,uint8_t month,uint8_t day)
{
	year = year -1980;
	uint16_t data_year =  ((uint16_t)year << 9);
	uint16_t data_month =  (uint16_t)month << 5;
	uint16_t data_day =  (uint16_t)day & 0x001F;
	uint16_t data_code = data_year|data_month|data_day;
	printf("now data code :%4x \r\n",data_code);
	return data_code;
}


/************************************************** AT24's main storage write and backup area read and write **************************************************/

void AT24Write(int position,void* pstruct,int index)
{
	HAL_I2C_Mem_Write(&hi2c1, AT24CXX_Write_ADDR,position,I2C_MEMADD_SIZE_8BIT, (uint8_t*)pstruct+index,1,1000);
	HAL_Delay(10);
	HAL_I2C_Mem_Write(&hi2c1, AT24CXX_Write_ADDR,128+position,I2C_MEMADD_SIZE_8BIT, (uint8_t*)pstruct+index,1,1000);
	HAL_Delay(10);
}


void AT24Read(int position,void* pstruct,int index)
{
	HAL_I2C_Mem_Read(&hi2c1, AT24CXX_Read_ADDR,position,I2C_MEMADD_SIZE_8BIT, (uint8_t*)pstruct+index,1,1000);
}




/************************************************** Reading and writing of error logs **************************************************/

void write_fault_header(FaultHeader_t* header)
{
	header->err_time = time_code(17,31);
	header->err_data = data_code(2025,3,5);
	header->fault_count = header->fault_count + 1;
	header->read_addr = 0;
	header->write_addr = 0;
	header->CRCheck = ModBusCRC16(header,sizeof(FaultHeader_t)-2);
	  for(int j = 0;j<sizeof(FaultHeader_t);j++)
	  {
		  AT24Write(FaultHeader_START + j,header,j);
	  }
}

void read_fault_header(FaultHeader_t* header)
{
	  for(int j = 0;j<sizeof(FaultHeader_t);j++)
	  {
	    	AT24Read(FaultHeader_START,header,j);
	  }
}


void write_fault_record(FaultRecord_t* record)
{
	uint8_t i,j;
	uint16_t ring_next_write = FaultHeader.write_addr + sizeof(FaultRecord_t);
	if(ring_next_write >= FaultRecord_END)
	{
		for(j = 0;j<sizeof(FaultRecord_t)-(ring_next_write-FaultRecord_END);j++)
		{
		  AT24Write(FaultHeader.write_addr + j,record,j);
		}

		for(i = 0;i<(ring_next_write-FaultRecord_END);j++,i++)
		{
		  AT24Write(FaultRecord_START + i,record,j);
		  if(FaultRecord_START + i > FaultHeader.read_addr)
		  {
			  FaultHeader.read_addr = FaultHeader.read_addr + sizeof(FaultRecord_t);
			  if(FaultHeader.read_addr >= FaultRecord_END)
			  {
				  FaultHeader.read_addr = FaultRecord_START+(FaultHeader.read_addr - FaultRecord_END);
			  }
		  }
		}
		FaultHeader.write_addr = FaultRecord_START + i;
		write_fault_header(&FaultHeader);
	}else{
		for(j = 0;j<sizeof(record);j++)
		{
		  AT24Write(FaultHeader.write_addr + j,record,j);
		}
		FaultHeader.write_addr = FaultHeader.write_addr + j;
		write_fault_header(&FaultHeader);
	}
}


uint8_t read_fault_record(FaultRecord_t* record)
{
	uint8_t i,j;
	if(FaultHeader.read_addr == FaultHeader.write_addr){printf("read empty\r\n");return 0;}
	uint16_t ring_next_read = FaultHeader.read_addr + sizeof(FaultRecord_t);
	if(ring_next_read >= FaultRecord_END)
	{
		for(j = 0;j<sizeof(FaultRecord_t)-(ring_next_read-FaultRecord_END);j++)
		{
		  AT24Read(FaultHeader.read_addr + j,record,j);
		}

		for(i = 0;i<(ring_next_read-FaultRecord_END);j++,i++)
		{
			AT24Read(FaultRecord_START + i,record,j);
		}
		FaultHeader.read_addr = FaultRecord_START + i;
		write_fault_header(&FaultHeader);
	}else{
		for(j = 0;j<sizeof(FaultRecord_t);j++)
		{
			AT24Read(FaultHeader.read_addr + j,record,j);
		}
		FaultHeader.read_addr = FaultHeader.read_addr + j;
		write_fault_header(&FaultHeader);
	}
}

/************************************************** Data error in the main storage area, restore from the backup area **************************************************/

void BackupOverwrite(void)
{
	uint8_t i;
	for(i = 0;i<128;i++)
	{
		AT24Read(128 + i,&AT24CXX_ReadBuff[1],i);
	}

	for(i = 0;i<128;i++)
	{
		AT24Write(i,&AT24CXX_ReadBuff[1],i);
	}
}


/************************************************** END **************************************************/

















//
//
//void AT24C02_Func(void)
//{
//	loginfo.time = time_code(12,31);
//	loginfo.data = data_code(2025,2,26);
//	loginfo.frequency = 0x3421;
//	loginfo.voltage = 0x2222;
//	loginfo.current = 0x2222;
//	loginfo.crcheck = IEEECRC32(&loginfo,sizeof(loginfo)-4);
//
//	printf("crc %8x\r\n",loginfo.crcheck);
//
//    for(int j = 0;j<sizeof(loginfo);j++)
//    {
//    	AT24TwiceWrite(j,loginfo,j);
//    }
//
////    HAL_I2C_Mem_Write(&hi2c1, AT24CXX_Write_ADDR,128+9,I2C_MEMADD_SIZE_8BIT, (uint8_t*)&loginfo+1,1,1000);
//
//
//    HAL_I2C_Mem_Read(&hi2c1, AT24CXX_Read_ADDR,0,I2C_MEMADD_SIZE_8BIT, (uint8_t*)AT24CXX_ReadBuff,sizeof(loginfo),0xFF);
//    for(int idx = 0; idx < sizeof(loginfo); idx++) {
//    	printf("Struct Maininfo = %2x \r\n",AT24CXX_ReadBuff[idx]);
//    }
//    if(IEEECRC32(AT24CXX_ReadBuff,sizeof(loginfo)-4) == *(uint32_t*)&AT24CXX_ReadBuff[sizeof(loginfo)-4])
//    {
//    	printf("crc main check success\r\n");
//    }
//	printf("crc calc %8x\r\n",IEEECRC32(AT24CXX_ReadBuff,sizeof(loginfo)-4));
//	printf("crc read %8x\r\n",*(uint32_t*)&AT24CXX_ReadBuff[sizeof(loginfo)-4]);
//
//
//    HAL_I2C_Mem_Read(&hi2c1, AT24CXX_Read_ADDR,128,I2C_MEMADD_SIZE_8BIT, (uint8_t*)AT24CXX_ReadBuff+128,sizeof(loginfo),0xFF);
//    for(int idx = 0; idx < sizeof(loginfo); idx++) {
//    	printf("Struct Subinfo = %2x \r\n",AT24CXX_ReadBuff[128+idx]);
//    }
//    if(IEEECRC32(&AT24CXX_ReadBuff[128],sizeof(loginfo)-4) == *(uint32_t*)&AT24CXX_ReadBuff[128+sizeof(loginfo)-4])
//    {
//    	printf("crc sub check success\r\n");
//    }
//	printf("crc calc %8x\r\n",IEEECRC32(AT24CXX_ReadBuff+128,sizeof(loginfo)-4));
//	printf("crc read %8x\r\n",*(uint32_t*)&AT24CXX_ReadBuff[128+sizeof(loginfo)-4]);
//
//
//    struct_decode();
//    time_decode();
//    data_decode();
//
//}
//
//
//
//
//









