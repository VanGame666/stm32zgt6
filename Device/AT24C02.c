/************************************************** START **************************************************/

#include "AT24C02.h"


uint8_t AT24CXX_ReadBuff[AT24CXX_BUFF_SIZE];
uint8_t AT24CXX_WriteBuff[AT24CXX_BUFF_SIZE];

FaultHeaderTypeDef FaultHeader;
FaultRecordTypeDef FaultRecord;


void struct_regist(void)
{
	FaultHeaderTypeDef *pStates = (FaultHeaderTypeDef*)AT24CXX_ReadBuff[1];

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
void init_fault_header(FaultHeaderTypeDef* header)
{
	header->err_time = 0;			//RTC read-time writes will then be added
	header->err_data = 0;
	header->fault_count = 0;
	header->read_addr = 0;
	header->write_addr = 0;
	header->CRCheck = ModBusCRC16(header,sizeof(FaultHeaderTypeDef)-2);

	for(int j = 0;j<sizeof(FaultHeaderTypeDef);j++)
	{
		AT24Write(FaultHeader_START + j,header,j);
	}
}




void read_fault_header(FaultHeaderTypeDef* header)
{
	  for(int j = 0;j<sizeof(FaultHeaderTypeDef);j++)
	  {
	    	AT24Read(FaultHeader_START,header,j);
	  }
}


void write_fault_header(FaultHeaderTypeDef* header)
{
	header->err_time = time_code(17,31);			//RTC read-time writes will then be added
	header->err_data = data_code(2025,3,5);
	header->fault_count = header->fault_count;
	header->read_addr = header->read_addr;
	header->write_addr = header->write_addr;
	header->CRCheck = ModBusCRC16(header,sizeof(FaultHeaderTypeDef)-2);
	for(int j = 0;j<sizeof(FaultHeaderTypeDef);j++)
	{
		AT24Write(FaultHeader_START + j,header,j);
	}
}



void write_fault_record(void)
{
	uint8_t i,j;
	read_fault_header(&FaultHeader);
	uint16_t ring_next_write = FaultHeader.write_addr + sizeof(FaultRecordTypeDef);
	if(ring_next_write >= FaultRecord_END)
	{
		for(j = 0;j<sizeof(FaultRecordTypeDef)-(ring_next_write-FaultRecord_END);j++)
		{
		  AT24Write(FaultHeader.write_addr + j,&FaultRecord,j);
		}

		for(i = 0;i<(ring_next_write-FaultRecord_END);j++,i++)
		{
		  AT24Write(FaultRecord_START + i,&FaultRecord,j);
		  if(FaultRecord_START + i > FaultHeader.read_addr)
		  {
			  FaultHeader.read_addr = FaultHeader.read_addr + sizeof(FaultRecordTypeDef);
			  if(FaultHeader.read_addr >= FaultRecord_END)
			  {
				  FaultHeader.read_addr = FaultRecord_START+(FaultHeader.read_addr - FaultRecord_END);
			  }
		  }
		}
		FaultHeader.fault_count = FaultHeader.fault_count + 1;
		FaultHeader.write_addr = FaultRecord_START + i;
		write_fault_header(&FaultHeader);
	}else{
		for(j = 0;j<sizeof(FaultRecord);j++)
		{
		  AT24Write(FaultHeader.write_addr + j,&FaultRecord,j);
		}

		FaultHeader.fault_count = FaultHeader.fault_count + 1;
		FaultHeader.write_addr = FaultHeader.write_addr + j;
		write_fault_header(&FaultHeader);
	}
}


uint8_t read_fault_record(void)
{
	read_fault_header(&FaultHeader);
	uint8_t i,j;
	if(FaultHeader.read_addr == FaultHeader.write_addr){printf("read empty\r\n");return 0;}
	uint16_t ring_next_read = FaultHeader.read_addr + sizeof(FaultRecordTypeDef);
	if(ring_next_read >= FaultRecord_END)
	{
		for(j = 0;j<sizeof(FaultRecordTypeDef)-(ring_next_read-FaultRecord_END);j++)
		{
		  AT24Read(FaultHeader.read_addr + j,&FaultRecord,j);
		}

		for(i = 0;i<(ring_next_read-FaultRecord_END);j++,i++)
		{
			AT24Read(FaultRecord_START + i,&FaultRecord,j);
		}
		FaultHeader.read_addr = FaultRecord_START + i;
		write_fault_header(&FaultHeader);
	}else{
		for(j = 0;j<sizeof(FaultRecordTypeDef);j++)
		{
			AT24Read(FaultHeader.read_addr + j,&FaultRecord,j);
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
		AT24Read(128 + i,&AT24CXX_ReadBuff[0],i);
	}

	for(i = 0;i<128;i++)
	{
		AT24Write(i,&AT24CXX_ReadBuff[0],i);
	}
}


/************************************************** END **************************************************/


