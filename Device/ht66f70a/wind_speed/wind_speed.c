#include "HT66F70A.h"
//#include "MyType.h.h"
#include "build-in.h"

#define EedNumber	1

unsigned short ADR;
unsigned short WindSpeed_Class;

// IO setting
#define PortD_AllBitDirection		_pdc
#define PortD_AllBitStatus			_pd
#define PortD_Bit0Direction			_pdc0
#define PortD_Bit0DStatus			_pd0
#define PortD_Bit1Direction			_pdc1
#define PortD_Bit1DStatus			_pd1
#define PortD_Bit2Direction			_pdc2
#define PortD_Bit2DStatus			_pd2
#define PortD_Bit3Direction			_pdc3
#define PortD_Bit3DStatus			_pd3
#define PortD_Bit4Direction			_pdc4
#define PortD_Bit4DStatus			_pd4
#define PortD_Bit5Direction			_pdc5
#define PortD_Bit5DStatus			_pd5
#define PortD_Bit6Direction			_pdc6
#define PortD_Bit6DStatus			_pd6
#define PortD_Bit7Direction			_pdc7
#define PortD_Bit7DStatus			_pd7

// uart setting
#define TX_data					_pc2
#define TX_PC2Direction			_pcc2
#define RX_data					_pc3
#define RX_PC3Direction			_pcc3

// adc setting
unsigned int ADC_Ch4_HByte, ADC_Ch4_LByte;

// timer count
volatile unsigned int CTM_Count1;

// uart setting
unsigned char crc;
unsigned int i;
volatile unsigned char BaudRate;
volatile unsigned char BaudRate_Const;
volatile unsigned char UartBitCount;
volatile unsigned char TXREG;
volatile unsigned char RCREG;
volatile unsigned char TransmitData[25];
volatile unsigned char ReceiveData[23];
volatile unsigned char ZigbeeREG_Const;

// Declare Function
void TRANSMIT(void);
void RECEIVE(void);
void crc_cal(void);


void main()
{
	//close watch dog
	_tm0c0=0b00100000;
	_tm0c1=0b11000001;
	_tm0al=1000&0xff;
	_tm0ah=1000>>8;
	
	_mf0e=1;
	_t0ae=1;
	_emi=1;
	_t0af=0;
	_mf0f=0;
	CTM_Count1=500;
	
	PortD_AllBitDirection=0x00;
	PortD_AllBitStatus=0x00;
	
	_pas2=0x03;
	_adcr0=0b00000100;
	_adcr1=0b00100110;
	
	_ade=1;
	_emi=1;
	_start=1;
	_start=0;
	
	_t0on=1;
	_wdtc=0b01010111;
	
	TX_PC2Direction=0;
	RX_PC3Direction=1;
	TX_data=1;
	BaudRate=39;
	
	ZigbeeREG_Const=0;

	TransmitData[0]=0x7E;
	TransmitData[1]=0x00;
	TransmitData[2]=0x10;
	TransmitData[3]=0x10;
	TransmitData[4]=0x01;
	TransmitData[5]=0x00;
	TransmitData[6]=0x7D;
	TransmitData[7]=0x33;
	TransmitData[8]=0xA2;
	TransmitData[9]=0x00;
	TransmitData[10]=0x40;
	TransmitData[11]=0xE6;
	TransmitData[12]=0x6E;
	TransmitData[13]=0x6A;
	TransmitData[14]=0xFF;
	TransmitData[15]=0xFE;
	TransmitData[16]=0x00;
	TransmitData[17]=0x00;
	TransmitData[18]=EedNumber;
	TransmitData[19]=WindSpeed_Class;
//	TransmitData[]=ADC_Ch4_LByte;
//	TransmitData[]=ADC_Ch4_HByte;
	crc_cal();
	TransmitData[20]=crc;			//CRC
	
	while(1)
	{
		
	}
	
}

DEFINE_ISR(ISR_TM0, 0x14)
{
	CTM_Count1--;
	_clrwdt1();
	while(CTM_Count1==0)
	{
		CTM_Count1 = 500;
		
		_clrwdt1();
		_start=1;
		GCC_DELAY(200000);
		
/*		if(ADR>(1861.818/2))*/
		if(ADR>1100)
		{
			PortD_AllBitStatus=0xff;
			WindSpeed_Class=9;
		}
/*		else if(ADR>(1650.812/2))*/
		else if(ADR>1000)
		{
			PortD_AllBitStatus=0xff;
			WindSpeed_Class=8;			
		}
/*		else if(ADR>(1452.218/2))*/
		else if(ADR>900)
		{
			PortD_AllBitStatus=0x7f;
			WindSpeed_Class=7;			
		}	
/*		else if(ADR>(1241.212/2))*/
		else if(ADR>800)
		{
			PortD_AllBitStatus=0x3f;
			WindSpeed_Class=6;			
		}
/*		else if(ADR>(1030.206/2))*/
		else if(ADR>700)
		{
			PortD_AllBitStatus=0x1f;
			WindSpeed_Class=5;			
		}
/*		else if(ADR>(831.612/2))*/
		else if(ADR>600)
		{
			PortD_AllBitStatus=0x0f;
			WindSpeed_Class=4;			
		}
/*		else if(ADR>(620.606/2))*/
		else if(ADR>500)
		{
			PortD_AllBitStatus=0x07;
			WindSpeed_Class=3;			
		}
/*		else if(ADR>(409.6/2))*/
		else if(ADR>430)
		{
			PortD_AllBitStatus=0x03;
			WindSpeed_Class=2;			
		}
/*		else if(ADR>(211.006/2))*/
		else if(ADR>400)
		{
			PortD_AllBitStatus=0x01;
			WindSpeed_Class=1;			
		}
/*		else if(ADR<(211.006/2))*/
		else if(ADR<400)
		{
			PortD_AllBitStatus=0x00;
			WindSpeed_Class=0;			
		}
		
		TransmitData[18]=EedNumber;
		TransmitData[19]=WindSpeed_Class;
//		TransmitData[]=ADC_Ch4_LByte;
//		TransmitData[]=ADC_Ch4_HByte;
		crc_cal();
		TransmitData[20]=crc;		
		
		for(i=0;i<21;i++)
		{
			TXREG=TransmitData[i];
			TRANSMIT();
		}
		
		_clrwdt1();
		_start=0;
		GCC_DELAY(200000);		
	}
	_t0af=0;
	
	
}
DEFINE_ISR(ISR_ADC, 0x1C)
{
	ADR=(((unsigned short)_adrh<<8) + _adrl);
	ADC_Ch4_LByte=(unsigned char)ADR;
	ADC_Ch4_HByte=ADR>>8;
	_start=1;
	_start=0;
}

void TRANSMIT(void)
{
	BaudRate_Const=BaudRate;
	TX_data=0;
	UartBitCount=8;
	while(BaudRate_Const!=0)
	{
		BaudRate_Const--;
	}
	BaudRate_Const=BaudRate;
	while(UartBitCount!=0)
	{
		UartBitCount--;
		GCC_RRC(TXREG);	
		if(_c==0)
		{
			TX_data=0;
			while(BaudRate_Const!=0)
			{
				BaudRate_Const--;
			}
			BaudRate_Const=BaudRate;
		}
		else
		{
			TX_data=1;
			while(BaudRate_Const!=0)
			{
				BaudRate_Const--;
			}
			BaudRate_Const=BaudRate;
		}	
	}
	_nop();
	_nop();
	TX_data=1;
	while(BaudRate_Const!=0)
	{
		BaudRate_Const--;
	}
	BaudRate_Const=BaudRate;
	while(BaudRate_Const!=0)
	{
		BaudRate_Const--;
	}
	BaudRate_Const!=BaudRate;
	
	GCC_DELAY(10000);
}

void RECEIVE(void)
{
	BaudRate_Const=BaudRate+1;
	UartBitCount=8;
	if(RX_data==0)
	{
		while(BaudRate_Const!=0)
		{
			BaudRate_Const--;
		}
		BaudRate_Const=BaudRate+1;
		while(UartBitCount!=0)
		{
			UartBitCount--;
			if(RX_data==1)
			{
				_c=1;
			}
			else
			{
				_c=0;
			}
			GCC_RRC(RCREG);
			
			while(BaudRate_Const!=0)
			{
				BaudRate_Const--;
			}
			BaudRate_Const=BaudRate+1;
		}
		_nop();
		ReceiveData[ZigbeeREG_Const]=RCREG;
		if(ZigbeeREG_Const==23)
		{
			ZigbeeREG_Const=0;
		}
		
	}
}

void crc_cal(void)
{
	int temp=0;
	//API 2 need escape char, 
	//0x11=XON, control char to start the serial data
	//0x13=XOFF, control char to stop the serial data
	//若Byte符合0x11或0x13, 則Byte前面加上0x7D，原有的Byte需與0x20做XOR運算
	//0x11 ==> 0x7D, (0x11^0x20)0x31
	
	temp+=TransmitData[3];
	temp+=TransmitData[4];
	temp+=TransmitData[5];
	temp+=0x13;
	temp+=TransmitData[8];
	temp+=TransmitData[9];
		
	for(i=10;i<20;i++)
	{
		temp+=TransmitData[i];
	}
	crc=0xff-temp;
}