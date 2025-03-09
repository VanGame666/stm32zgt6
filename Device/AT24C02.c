/************************************************** START **************************************************/

#include "AT24C02.h"


uint8_t AT24CXX_ReadBuff[AT24CXX_BUFF_SIZE];
uint8_t AT24CXX_WriteBuff[AT24CXX_BUFF_SIZE];


FaultHeaderTypeDef FaultHeader;
FaultRecordTypeDef FaultRecord;

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

void AT24Write(int position,void* pstruct)
{
	HAL_I2C_Mem_Write(&hi2c1, AT24CXX_Write_ADDR,position,I2C_MEMADD_SIZE_8BIT, (uint8_t*)pstruct,1,1000);
	HAL_Delay(10);
	HAL_I2C_Mem_Write(&hi2c1, AT24CXX_Write_ADDR,128+position,I2C_MEMADD_SIZE_8BIT, (uint8_t*)pstruct,1,1000);
	HAL_Delay(10);
}


void AT24Read(int position,void* pstruct)
{
	HAL_I2C_Mem_Read(&hi2c1, AT24CXX_Read_ADDR,position,I2C_MEMADD_SIZE_8BIT, (uint8_t*)pstruct,1,1000);
}


/************************************************** Reading and writing of error logs **************************************************/
void init_fault_header(void)
{
	FaultHeader.err_time = 0;			//RTC read-time writes will then be added
	FaultHeader.err_data = 0;
	FaultHeader.fault_count = 0;
	FaultHeader.read_addr = 0;
	FaultHeader.write_addr = 0;
	FaultHeader.CRCheck = ModBusCRC16(&FaultHeader,sizeof(FaultHeaderTypeDef)-2);

	for(int j = 0;j<sizeof(FaultHeaderTypeDef);j++)
	{
		AT24Write(FaultHeader_START + j,(uint8_t*)&FaultHeader+j);
	}
}




void read_fault_header(void)
{
	  for(int j = 0;j<sizeof(FaultHeaderTypeDef);j++)
	  {
	    	AT24Read(FaultHeader_START,(uint8_t*)&FaultHeader+j);
	  }
}


void write_fault_header(void)
{
	FaultHeader.err_time = time_code(17,31);			//RTC read-time writes will then be added
	FaultHeader.err_data = data_code(2025,3,5);
	FaultHeader.fault_count = FaultHeader.fault_count;
	FaultHeader.read_addr = FaultHeader.read_addr;
	FaultHeader.write_addr = FaultHeader.write_addr;
	FaultHeader.CRCheck = ModBusCRC16(&FaultHeader,sizeof(FaultHeaderTypeDef)-2);
	for(int j = 0;j<sizeof(FaultHeaderTypeDef);j++)
	{
		AT24Write(FaultHeader_START + j,(uint8_t*)&FaultHeader+j);
	}
}



void write_fault_record(void)
{
	uint8_t i,j;

	FaultRecord.freq = 0;
	FaultRecord.adc0_err_code = 0;
	FaultRecord.adc0_threshold_val = 0;
	/* .........To be added later */

	read_fault_header();
	/* Determine whether it exceeds the main storage area, and out of boundary backup and recovery area */
	uint16_t ring_next_write = FaultHeader.write_addr + sizeof(FaultRecordTypeDef);
	if(ring_next_write >= FaultRecord_END)
	{
		/* After writing the cross-border area, return to the starting address and continue writing */
		for(j = 0;j<sizeof(FaultRecordTypeDef)-(ring_next_write-FaultRecord_END);j++)
		{
		  AT24Write(FaultHeader.write_addr + j,(uint8_t*)&FaultRecord+j);
		}
		/* Continue to write the remaining data */
		for(i = 0;i<(ring_next_write-FaultRecord_END);j++,i++)
		{
		  AT24Write(FaultRecord_START + i,(uint8_t*)&FaultRecord+j);
		  /* If the write pointer is to overwrite the read pointer,
		   * the read pointer is moved forward which read the byte size of the error record once */
		  if(FaultRecord_START + i > FaultHeader.read_addr)
		  {
			  FaultHeader.read_addr = FaultHeader.read_addr + sizeof(FaultRecordTypeDef);
			  /* If the read pointer breaks out of the backup area, then return to the starting address and continue to start */
			  if(FaultHeader.read_addr >= FaultRecord_END)
			  {
				  FaultHeader.read_addr = FaultRecord_START+(FaultHeader.read_addr - FaultRecord_END);
			  }
		  }
		}
		/* Update error header record information */
		read_fault_header();
		FaultHeader.fault_count = FaultHeader.fault_count + 1;
		FaultHeader.write_addr = FaultRecord_START + i;
		write_fault_header();
	/* If there is no cross-border, write it directly */
	}else{
		for(j = 0;j<sizeof(FaultRecord);j++)
		{
		  AT24Write(FaultHeader.write_addr + j,(uint8_t*)&FaultRecord+j);
		}

		if(FaultHeader.write_addr + j > FaultHeader.read_addr)
		{
			FaultHeader.read_addr = FaultHeader.read_addr + sizeof(FaultRecordTypeDef);
			/* If the read pointer breaks out of the backup area, then return to the starting address and continue to start */
			if(FaultHeader.read_addr >= FaultRecord_END)
			{
				FaultHeader.read_addr = FaultRecord_START+(FaultHeader.read_addr - FaultRecord_END);
			}
		}
		/* The number of error records is increased by one */
		read_fault_header();
		FaultHeader.fault_count = FaultHeader.fault_count + 1;
		FaultHeader.write_addr = FaultHeader.write_addr + j;
		write_fault_header();
	}
}


uint8_t read_fault_record(void)
{
	read_fault_header();
	uint8_t i,j;
	/* If the read pointer and the write pointer are equal,
	 * it means that all error messages have been read and there are no pending error messages */
	if(FaultHeader.read_addr == FaultHeader.write_addr){printf("read empty\r\n");return 0;}
	/* Calculate whether the next read is out of bounds and backup area */
	uint16_t ring_next_read = FaultHeader.read_addr + sizeof(FaultRecordTypeDef);
	if(ring_next_read >= FaultRecord_END)
	{
		for(j = 0;j<sizeof(FaultRecordTypeDef)-(ring_next_read-FaultRecord_END);j++)
		{
		  AT24Read(FaultHeader.read_addr + j,(uint8_t*)&FaultRecord+j);
		}
		/* Continue reading from the error record start area */
		for(i = 0;i<(ring_next_read-FaultRecord_END);j++,i++)
		{
			AT24Read(FaultRecord_START + i,(uint8_t*)&FaultRecord+j);
		}
		/* Update error header record information */
		read_fault_header();
		FaultHeader.read_addr = FaultRecord_START + i;
		write_fault_header();
	}else{
		for(j = 0;j<sizeof(FaultRecordTypeDef);j++)
		{
			AT24Read(FaultHeader.read_addr + j,(uint8_t*)&FaultRecord+j);
		}
		read_fault_header();
		FaultHeader.read_addr = FaultHeader.read_addr + j;
		write_fault_header();
	}
}

/************************************************** Data error in the main storage area, restore from the backup area **************************************************/

void BackupOverwrite(void)
{
	uint8_t i;
	for(i = 0;i<128;i++)
	{
		AT24Read(128 + i,&AT24CXX_ReadBuff[0]+i);
	}

	for(i = 0;i<128;i++)
	{
		AT624Write(i,&AT24CXX_ReadBuff[0]+i);
	}
}


/************************************************** END **************************************************/


