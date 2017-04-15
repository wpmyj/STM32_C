/*
程序说明:  	
            MAX7219控制的数码管显示程序 --- STM32F107VCT 
						
注意事项：						
						1、MAX7219初始化完成后需要给予一定的延时，否则会出现无法正常显示的情况，最好采用不断
          刷新程序的方法，防止出现不显示情况。						
						2、每次读写，最好对MAX7219的一些常规配置进行重复初始化，否则也会出现不显示情况。
						3、每次刷新数码管显示的时候需要都刷新1遍，否则会造成显示不稳当。
						
端口使用:		见Doc文件夹内电路设计说明
创建者:     FangYIkaii   
创建时间:   2017-01-21 
修改时间:   2017-01-21
*/
/*
SPI:Serial Peripheral Interface串行外围设备接口--触摸屏，音频DA芯片PCM1770，W25X16,2.4G无线
1.点对点通信中，SPI接口不需要寻址操作，且为全双工通信，在多个从器件的系统中，每个从器件需要独立的使能信号
2.I2C数据输入输出用的是一根线，SPI则分别为SDO和SDI两根信号线，速度快过I2C,但输入输出是不同的管脚，占用CPU的管脚也多过I2C(全双工通信)
3.4条信号线：串行时钟SCLK,串行数据输出SDO/MOSI,串行数据输入SDI/MISO,片选信号线CS
MISO:主设备数据输入，从设备数据输出引脚，主机从这条信号线读入数据，从机的数据由这条线输出，从机-->主机
MOSI:主设备数据输出，从设备数据输入引脚，主机的数据从这条线输出，从机从这条线读入，主机-->从机
SCK：时钟信号线
CS:片选信号线，当SPI多个设备与MCU相连的时候，通过该引脚选择设备与MCU通信

*/

//========================================
//头文件
#include "main.h"

//========================================
//变量定义
//码表定义：                   0   			1    		2    	3    		4    	5    	6     	7    		8    9
//码表定义：                   F  		R/A  		H/N    	P   		-    	E   不显示                  
const uint8_t code_table[17]= {0x7e,  0x30,  0x6d,  0x79,  0x33,  0x5b,  0x5f,  0x70,  0x7f,  0x7b,
                               0x47,  0x77,  0x37,  0x67,  0x01,  0X4F,  0x00,
														};


//========================================
//函数声明
void LoaderSystemDisplay(void);												//装载机系统运行显示
void GearDisplay(uint8_t gear, uint8_t no);						//档位信息显示
void ErrorDisplay(uint8_t Error);											//错误信息显示
														
														
														
void MAX7219_Configuration(void);											//完成MAX7219的配置
void Init_MAX7219(void);															//MAX7219初始化程序
void Send1Byte_MAX7219(uint8_t dat);										//向MAX7219发送一个字节数据程序
void Clear1stChip_MAX7219(void);											//清第1片MAX7219的显示程序
void Clear2ndChip_MAX7219(void);											//清第2片MAX7219的显示程序
void Clear3rdChip_MAX7219(void);											//清第2片MAX7219的显示程序
void Write1stChip_MAX7219(uint16_t addr,uint16_t data);				//MAX7219往第1个芯片内写数据程序
void Write2ndChip_MAX7219(uint16_t addr,uint16_t data);				//MAX7219往第2个芯片内写数据程序
void Write3rdChip_MAX7219(uint16_t addr,uint16_t data);				//MAX7219往第3个芯片内写数据程序
void DisplayChar_MAX7219 (uint8_t addr,uint8_t data,uint8_t chipno);		//MAX7219显示数据程序
void DisplayDigitalTube1_4Num_MAX7219 (uint16_t data,uint8_t no,uint8_t point);				//第1、2号4位数码管显示程序，显示低4位
void DisplayDigitalTube2_2Num_MAX7219 (uint8_t data,uint8_t no,uint8_t point);				//第1、2、3、4号2位数码管显示程序，显示低4位
void DisplayDigitalTube3_4Num_MAX7219 (uint16_t data,uint8_t no,uint8_t point);	      //第1、2号4位数码管显示程序，显示低4位
//========================================
//子模块函数

/***********************************************************************
函数名称：void LoaderSystemDisplay(void)
功    能：装载机系统运行显示
输入参数：无
输出参数：无
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：
***********************************************************************/
void LoaderSystemDisplay(void)
{
		DisplayDigitalTube3_4Num_MAX7219 (XG.PressureShow/10,1,3);							//动臂大腔压力__第1、2号4位数码管显示程序，显示低4位
		DisplayDigitalTube3_4Num_MAX7219 (XG.Function,2,0);											//通道信息显示：按键值__第1、2号4位数码管显示程序，显示低4位			
		DisplayDigitalTube1_4Num_MAX7219 (XG.PumpSpeed,1,0);										//泵轮转速__第1、2号4位数码管显示程序，显示低4位
		DisplayDigitalTube1_4Num_MAX7219 (XG.SpeedShow,2,0);										//涡轮转速__第1、2号4位数码管显示程序，显示低4位
//		DisplayDigitalTube2_2Num_MAX7219 (ControllerInfo.SuggestGear,1,0);		//推荐挡位__第1、2、3、4号2位数码管显示程序，显示低4位
		GearDisplay(ControllerInfo.SuggestGear,2);		
	//			DisplayDigitalTube2_2Num_MAX7219 (Timer5.minute,2,0);							//当前挡位__第1、2、3、4号2位数码管显示程序，显示低4位
		GearDisplay(ControllerInfo.ProcessGear,1);															//当前挡位信息显示
		if(XG.Throttle>=100)
		{
			DisplayDigitalTube2_2Num_MAX7219(99,3,0);															//油门开度__第1、2、3、4号2位数码管显示程序，显示低4位
		}
		else
		{
			DisplayDigitalTube2_2Num_MAX7219 (XG.Throttle,3,0);											//油门开度__第1、2、3、4号2位数码管显示程序，显示低4位			
		}
//		DisplayDigitalTube2_2Num_MAX7219(ErrorMessage,4,0);											//错误信息显示__第1、2、3、4号2位数码管显示程序，显示低4位
		ErrorDisplay(ErrorMessage);
}	

/***********************************************************************
函数名称：void ErrorDisplay(uint8_t Error)
功    能：错误信息显示
输入参数：错误值
输出参数：无
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：
***********************************************************************/
void ErrorDisplay(uint8_t Error)
{
	uint8_t code;

	Init_MAX7219();					//需要重复设置，否则出现不显示现象
	code=code_table[Error%0x10];	
	Write2ndChip_MAX7219(DIG_7,code);					
	code=code_table[Error/0x10];							
	Write2ndChip_MAX7219(DIG_8,code);		
}


/***********************************************************************
函数名称：void GearDisplay(uint8_t gear, uint8_t no)
功    能：档位信息显示
输入参数：档位值
输出参数：无
编写时间：2014.05.29
编 写 人：masterjay
注    意：
***********************************************************************/
void GearDisplay(uint8_t gear, uint8_t no)
{
	uint8_t code;
	Init_MAX7219();					//需要重复设置，否则出现不显示现象
	if(no==1)		//当前挡位显示
	{
		if(flagctrl.Bits.KDchange)								//KD
		{
			code=code_table[14];		
			Write2ndChip_MAX7219(DIG_3,code);					
			code=code_table[14];						
			Write2ndChip_MAX7219(DIG_4,code);		
		}
		else
		{					
			if(gear/0x10==0)   //F
			{
				if(gear%0x10!=0)
				{
					code=code_table[10];		
					Write2ndChip_MAX7219(DIG_3,code);					
					code=code_table[gear%0x10];						
					Write2ndChip_MAX7219(DIG_4,code);		
				}	
				else
				{
					code=code_table[10];		
					Write2ndChip_MAX7219(DIG_3,code);					
					code=code_table[13];						
					Write2ndChip_MAX7219(DIG_4,code);					
				}
			}
			else if(gear/0x10==1)	//R
			{
				if(gear%0x10!=0)
				{
					code=code_table[11];		
					Write2ndChip_MAX7219(DIG_3,code);					
					code=code_table[gear%0x10];						
					Write2ndChip_MAX7219(DIG_4,code);		
				}
				else
				{
					code=code_table[11];		
					Write2ndChip_MAX7219(DIG_3,code);					
					code=code_table[13];						
					Write2ndChip_MAX7219(DIG_4,code);	
				}	
			}
			else if(gear/0x10==2)	//N
			{
				if(gear%0x10!=0)
				{
					code=code_table[12];		
					Write2ndChip_MAX7219(DIG_3,code);					
					code=code_table[gear%0x10];						
					Write2ndChip_MAX7219(DIG_4,code);		
				}
				else
				{
					code=code_table[12];		
					Write2ndChip_MAX7219(DIG_3,code);					
					code=code_table[13];		   				
					Write2ndChip_MAX7219(DIG_4,code);		
				}				
			}			
		}					
	}
	else if(no==2)	//推荐挡位显示	
	{
		if(flagctrl.Bits.KDchange)								//KD
		{
			code=code_table[14];		
			Write2ndChip_MAX7219(DIG_1,code);					
			code=code_table[14];						
			Write2ndChip_MAX7219(DIG_2,code);		

			
		}
		else
		{					
			if(gear/0x10==0)   //F
			{
				if(gear%0x10!=0)
				{
					code=code_table[10];		
					Write2ndChip_MAX7219(DIG_1,code);					
					code=code_table[gear%0x10];						
					Write2ndChip_MAX7219(DIG_2,code);		
				}	
				else
				{
					code=code_table[10];		
					Write2ndChip_MAX7219(DIG_1,code);					
					code=code_table[13];						
					Write2ndChip_MAX7219(DIG_2,code);					
				}
			}
			else if(gear/0x10==1)	//R
			{
				if(gear%0x10!=0)
				{
					code=code_table[11];		
					Write2ndChip_MAX7219(DIG_1,code);					
					code=code_table[gear%0x10];						
					Write2ndChip_MAX7219(DIG_2,code);		
				}
				else
				{
					code=code_table[11];		
					Write2ndChip_MAX7219(DIG_1,code);					
					code=code_table[13];						
					Write2ndChip_MAX7219(DIG_2,code);	
				}	
			}
			else if(gear/0x10==2)	//N
			{
				if(gear%0x10!=0)
				{
					code=code_table[12];		
					Write2ndChip_MAX7219(DIG_1,code);					
					code=code_table[gear%0x10];						
					Write2ndChip_MAX7219(DIG_2,code);		
				}
				else
				{
					code=code_table[12];		
					Write2ndChip_MAX7219(DIG_1,code);					
					code=code_table[13];		   				
					Write2ndChip_MAX7219(DIG_2,code);		
				}				
			}			
		}			
	}
	
	
	
}

/***********************************************************************
函数名称：void MAX7219_Configuration(void)
功    能：完成MAX7219的配置
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：
***********************************************************************/
void MAX7219_Configuration(void)
{
	//GPIO结构体初始化
	GPIO_InitTypeDef GPIO_InitStructure;	
	//时钟：DIN;LOAD;CLK
	RCC_APB2PeriphClockCmd(DIN_MAX7219_RCC_APB2Periph_GPIOX, ENABLE);
	RCC_APB2PeriphClockCmd(LOAD_MAX7219_RCC_APB2Periph_GPIOX, ENABLE);	
	RCC_APB2PeriphClockCmd(CLK_MAX7219_RCC_APB2Periph_GPIOX, ENABLE);	
	
	//GPIO定义：管脚，速度--50MHz，模式--PP--推挽输出，端口
	//DIN
	GPIO_InitStructure.GPIO_Pin = DIN_MAX7219_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(DIN_MAX7219_GPIOX, &GPIO_InitStructure);
	//LOAD
	GPIO_InitStructure.GPIO_Pin = LOAD_MAX7219_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(LOAD_MAX7219_GPIOX, &GPIO_InitStructure);	
	//CLK
	GPIO_InitStructure.GPIO_Pin = CLK_MAX7219_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(CLK_MAX7219_GPIOX, &GPIO_InitStructure);
	
	//DIN写低电平
	DIN_MAX7219(0);
	//LOAD写高电平
	LOAD_MAX7219(1);	
	//CLK写低电平
	CLK_MAX7219(0); 
	//MAX7219初始化程序
	Init_MAX7219();	
	Clear1stChip_MAX7219();														//清第1片MAX7219的显示程序	
	Clear2ndChip_MAX7219();														//清第2片MAX7219的显示程序
	Clear3rdChip_MAX7219();														//清第3片MAX7219的显示程序
}

/***********************************************************************
函数名称：void Init_MAX7219(void)
功    能：MAX7219初始化程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：
					#define 			REG_DECODE 				0x09   	//译码控制寄存器   
					#define 			REG_INTENSITY 		0x0a   	//亮度控制寄存器 
					#define 			REG_SCAN_LIMIT 		0x0b   	//扫描界限寄存器 
					#define 			REG_SHUTDOWN 			0x0c   	//关断模式寄存器
					#define 			REG_DISPLAY_TEST 	0x0f  	//测试控制寄存器
***********************************************************************/
void Init_MAX7219(void)
{
	Write1stChip_MAX7219(REG_DISPLAY_TEST,0x00);      //选择工作模式（0xX0）			0x01 --test模式
	Write2ndChip_MAX7219(REG_DISPLAY_TEST,0x00);      //选择工作模式（0xX0）			0x01 --test模式
	Write3rdChip_MAX7219(REG_DISPLAY_TEST,0x00);      //选择工作模式（0xX0）			0x01 --test模式	
	Write1stChip_MAX7219(REG_INTENSITY,0x02);        	//设置初始亮度  
	Write2ndChip_MAX7219(REG_INTENSITY,0x02);        	//设置初始亮度 
	Write3rdChip_MAX7219(REG_INTENSITY,0x02);        	//设置初始亮度 	
	Write1stChip_MAX7219(REG_SCAN_LIMIT,0x07);        //设置为全显示
	Write2ndChip_MAX7219(REG_SCAN_LIMIT,0x07);        //设置为全显示
	Write3rdChip_MAX7219(REG_SCAN_LIMIT,0x07);        //设置为全显示	
	Write1stChip_MAX7219(REG_DECODE,0x00);       			//选用B解码模式--0xFF,非解码模式--0x00
	Write2ndChip_MAX7219(REG_DECODE,0x00);       			//选用B解码模式--0xFF,非解码模式--0x00
	Write3rdChip_MAX7219(REG_DECODE,0x00);       			//选用B解码模式--0xFF,非解码模式--0x00	
	Write1stChip_MAX7219(REG_SHUTDOWN,0x01);         	//开启正常工作模式（0xX1）	0x00 --关闭显示
	Write2ndChip_MAX7219(REG_SHUTDOWN,0x01);         	//开启正常工作模式（0xX1）	0x00 --关闭显示	
	Write3rdChip_MAX7219(REG_SHUTDOWN,0x01);         	//开启正常工作模式（0xX1）	0x00 --关闭显示			
}

/***********************************************************************
函数名称：void Send1Byte_MAX7219(uint8_t dat)
功    能：向MAX7219发送一个字节数据程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：for i= 0：8
             1.CLK置0
             2.判断dat：DIN写0或1
             3.CLK置1
             4.dat左移一位
***********************************************************************/
void Send1Byte_MAX7219(uint8_t dat)
{
   uint8_t i,temp;
   for (i=0;i<8;i++)
	 {   
			CLK_MAX7219(0);       				//CLK置零
		 	delay(DelayTime_MAX7219);
			temp=dat&0x80; 								//每次取高字节      
			if(temp==0x80)
			{
				DIN_MAX7219(1);
			}
			else
			{
				DIN_MAX7219(0);
			}
			delay(DelayTime_MAX7219);
			dat<<=1;										//数据左移1位
			CLK_MAX7219(1);							//CLK置高
			delay(DelayTime_MAX7219);
	}
}

/***********************************************************************
函数名称：void Clear1stChip_MAX7219(void);
功    能：清第1片MAX7219的显示程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：寄存器1-8：digit0-digit7，清空8位数
***********************************************************************/
void Clear1stChip_MAX7219(void)
{ 
  uint8_t i; 
  for (i=1; i <=8; i++) 
	{
		Write1stChip_MAX7219(i, 0x00);
	}
} 

/***********************************************************************
函数名称：void Clear2ndChip_MAX7219(void);
功    能：清第2片MAX7219的显示程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：寄存器1-8：digit0-digit7，清空8位数
***********************************************************************/
void Clear2ndChip_MAX7219(void)
{
  uint8_t i; 
  for (i=1; i <=8; i++) 
	{
		Write2ndChip_MAX7219(i, 0x00);
	}
}

/***********************************************************************
函数名称：void Clear3rdChip_MAX7219(void);
功    能：清第2片MAX7219的显示程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：寄存器1-8：digit0-digit7，清空8位数
***********************************************************************/
void Clear3rdChip_MAX7219(void)
{
  uint8_t i; 
  for (i=1; i <=8; i++) 
	{
		Write3rdChip_MAX7219(i, 0x00);
	}
}

/***********************************************************************
函数名称：void Write1stChip_MAX7219(uint16_t addr,uint16_t data);
功    能：MAX7219往第1个芯片内写数据程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：LOAD高电平
          1.写第三个芯片(空) 2.写第二个芯片(空) 3.写第一个芯片(数据,地址)
          LOAD低电平
***********************************************************************/
void Write1stChip_MAX7219(uint16_t addr,uint16_t data)
{ 
	LOAD_MAX7219(0); 							//LOAD置低,选通MAX7219芯片
	
	delay(DelayTime_MAX7219);			//对第三个芯片进行空操作	
	Send1Byte_MAX7219(REG_NO_OP);          
	Send1Byte_MAX7219(0x00);
// 	CLK_MAX7219(1);								//时钟上升沿，发送地址
	
	delay(DelayTime_MAX7219);			//对第二个芯片进行空操作
	Send1Byte_MAX7219(REG_NO_OP);	
	Send1Byte_MAX7219(0x00);	
// 	CLK_MAX7219(1);								//时钟上升沿，发送地址
	
	delay(DelayTime_MAX7219);
	Send1Byte_MAX7219(addr);			//对第一个芯片进行写操作
	Send1Byte_MAX7219(data);
	
	LOAD_MAX7219(1);  						//LOAD置高禁用MAX7219芯片 
	delay(DelayTime_MAX7219);
}

/***********************************************************************
函数名称：void Write2ndChip_MAX7219(uint16_t addr,uint16_t data);
功    能：MAX7219往第2个芯片内写数据程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意： LOAD高电平
           1.写第三个芯片(空) 2.写第二个芯片(数据,地址) 3.写第一个芯片(空)
           LOAD低电平
***********************************************************************/
void Write2ndChip_MAX7219(uint16_t addr,uint16_t data)
{
	LOAD_MAX7219(0); 							//LOAD置低选通MAX7219芯片
	
	delay(DelayTime_MAX7219);      //对第三个芯片进行空操作  
	Send1Byte_MAX7219(REG_NO_OP);        
	Send1Byte_MAX7219(0x00);
//	CLK_MAX7219(1);								//时钟上升沿，发送地址		
	
	delay(DelayTime_MAX7219);
	Send1Byte_MAX7219(addr);     	//对第二个芯片进行写操作         
	Send1Byte_MAX7219(data);
//	CLK_MAX7219(1);								//时钟上升沿，发送地址
	
	delay(DelayTime_MAX7219);
	Send1Byte_MAX7219(REG_NO_OP);	//对第一个芯片进行空操作
	Send1Byte_MAX7219(0x00);
	
	LOAD_MAX7219(1);  						//LOAD置高禁用MAX7219芯片	
	delay(DelayTime_MAX7219);
}

/***********************************************************************
函数名称：void Write3rdChip_MAX7219(uint16_t addr,uint16_t data);
功    能：MAX7219往第3个芯片内写数据程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：LOAD高电平
          1.写第三个芯片(数据,地址) 2.写第二个芯片(空) 3.写第一个芯片(空)
          LOAD低电平
***********************************************************************/
void Write3rdChip_MAX7219(uint16_t addr,uint16_t data)
{
	LOAD_MAX7219(0); 							//LOAD置低选通MAX7219芯片
	
	delay(DelayTime_MAX7219); 	    //对第三个芯片进行写操作        
	Send1Byte_MAX7219(addr);     
	Send1Byte_MAX7219(data);
//	CLK_MAX7219(1);								//时钟上升沿，发送地址	
	
	delay(DelayTime_MAX7219);        //对第二个芯片进行空操作         
	Send1Byte_MAX7219(REG_NO_OP);
	Send1Byte_MAX7219(0x00);
//	CLK_MAX7219(1);								//时钟上升沿，发送地址
	
	delay(DelayTime_MAX7219);
	Send1Byte_MAX7219(REG_NO_OP);	//对第一个芯片进行空操作
	Send1Byte_MAX7219(0x00);
	
	LOAD_MAX7219(1);  						//LOAD置高禁用MAX7219芯片	
	delay(DelayTime_MAX7219);
}


/***********************************************************************
函数名称：void DisplayChar_MAX7219 (uint8_t addr,uint8_t data,uint8_t chipno) 
功    能：MAX7219显示数据程序
输入参数：
输出参数：
编写时间：2017-01-21
编 写 人：FangYikaii
注    意：
***********************************************************************/
void DisplayChar_MAX7219(uint8_t addr,uint8_t data,uint8_t chipno) 
{
	if(chipno==1)
	{
		Write1stChip_MAX7219(addr,data);
	}
	else if(chipno==2)
	{
		Write2ndChip_MAX7219(addr,data);
	}		
	else if(chipno==3)
	{
		Write3rdChip_MAX7219(addr,data);
	}	
} 

/***********************************************************************
函数名称：void DisplayDigitalTube1_4Num_MAX7219 (uint16_t data,uint8_t no,uint8_t point) 
功    能：第1、2号4位数码管显示程序，显示低4位
输入参数：数据值，数码管编号，小数点位置
输出参数：
编写时间：2017-01-21
编 写 人：FangYikaii
注    意：
***********************************************************************/
void DisplayDigitalTube1_4Num_MAX7219(uint16_t data,uint8_t no,uint8_t point) 
{
	uint8_t code;
	/*重要*/
	Init_MAX7219();					//需要重复设置，否则出现不显示现象
	data=data%10000;				//显示低4位
	if(no==1)								//第1号4位数码管
	{
		//千位
		code=data/1000;
		//表中对应字符
 		code=code_table[code];
		if(point==4)
		{
			code=code | 0x80;
		}			 
		//第一个数
		Write1stChip_MAX7219(DIG_1,code);		
		
		//百位
		code=(data%1000)/100;
		//表中对应的字符
 		code=code_table[code];		
		if(point==3)
		{
			code=code | 0x80;
		}
		//第二个数
		Write1stChip_MAX7219(DIG_2,code);					

		//十位
		code=(data%100)/10;
		//表中对应字符
 		code=code_table[code];			
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第三个数
		Write1stChip_MAX7219(DIG_3,code);					

		//个位
		code=data%10;
		//表中对应字符
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}				
		//第四个数
		Write1stChip_MAX7219(DIG_4,code);						
	}
	else if(no==2)					//第2号4位数码管
	{
		//千位
		code=data/1000;
		//表中对应的数
 		code=code_table[code];
		if(point==4)
		{
			code=code | 0x80;
		}
    //第五个数
		Write1stChip_MAX7219(DIG_5,code);					

		//百位
		code=(data%1000)/100;
		//表中对应的数
 		code=code_table[code];		
		if(point==3)
		{
			code=code | 0x80;
		}				
		//第六个数
		Write1stChip_MAX7219(DIG_6,code);					

		//十位
		code=(data%100)/10;
		//表中对应的数
 		code=code_table[code];			
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第七个数
		Write1stChip_MAX7219(DIG_7,code);					

		//个位
		code=data%10;
		//表中对应的数
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//第八个数
		Write1stChip_MAX7219(DIG_8,code);					
	}
}

/***********************************************************************
函数名称：void DisplayDigitalTube2_2Num_MAX7219 (uint8_t data,uint8_t no,uint8_t point)
功    能：第1、2、3、4号2位数码管显示程序，显示低2位
输入参数：数值，数码管编号，小数点位置
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：
***********************************************************************/
void DisplayDigitalTube2_2Num_MAX7219(uint8_t data,uint8_t no,uint8_t point)
{
	uint8_t code;
	/*重要*/
	Init_MAX7219();					//需要重复设置，否则出现不显示现象
	data=data%100;					//显示低2位
	if(no==1)								//第1号2位数码管
	{
		//十位
		code=data/10;
 		code=code_table[code];	
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第一个数
		Write2ndChip_MAX7219(DIG_1,code);				

		//个位
		code=data%10;
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//第二个数
		Write2ndChip_MAX7219(DIG_2,code);									
	}
	else if(no==2)					//第2号2位数码管
	{
		//十位
		code=data/10;
 		code=code_table[code];	
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第三个数
		Write2ndChip_MAX7219(DIG_3,code);					

		//个位
		code=data%10;
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//第四个数
		Write2ndChip_MAX7219(DIG_4,code);					
	
				
	}	
	else if(no==3)					//第3号2位数码管
	{
		//十位
		code=data/10;
 		code=code_table[code];	
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第五个数
		Write2ndChip_MAX7219(DIG_5,code);					

		//个位
		code=data%10;
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//第六个数
		Write2ndChip_MAX7219(DIG_6,code);						
	}		
	else if(no==4)					//第4号2位数码管
	{
		//十位
		code=data/10;
 		code=code_table[code];	
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第七个数
		Write2ndChip_MAX7219(DIG_7,code);					
	
		//个位
		code=data%10;
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//个位
		Write2ndChip_MAX7219(DIG_8,code);					
	}	
}


/***********************************************************************
函数名称：void DisplayDigitalTube3_4Num_MAX7219 (uint16_t data,uint8_t no,uint8_t point) 
功    能：第1、2号4位数码管显示程序，显示低4位
输入参数：数据值，数码管编号，小数点位置
输出参数：
编写时间：2017-01-21
编 写 人：FangYIkaii
注    意：
***********************************************************************/
void DisplayDigitalTube3_4Num_MAX7219 (uint16_t data,uint8_t no,uint8_t point) 
{
	uint8_t code;
	/*重要*/
	Init_MAX7219();					//需要重复设置，否则出现不显示现象
	data=data%10000;				//显示低4位
	if(no==1)								//第1号4位数码管
	{
		//千位
		code=data/1000;
 		code=code_table[code];
		if(point==4)
		{
			code=code | 0x80;
		}			
		//第一个数
		Write3rdChip_MAX7219(DIG_1,code);

		//百位
		code=(data%1000)/100;
 		code=code_table[code];		
		if(point==3)
		{
			code=code | 0x80;
		}			
		//第二个数
		Write3rdChip_MAX7219(DIG_2,code);					

		//十位
		code=(data%100)/10;
 		code=code_table[code];			
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第三个数
		Write3rdChip_MAX7219(DIG_3,code);					

		//个位
		code=data%10;
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//第四个数
		Write3rdChip_MAX7219(DIG_4,code);							
	}
	else if(no==2)					//第2号4位数码管
	{
		//千位
		code=data/1000;
 		code=code_table[code];
		if(point==4)
		{
			code=code | 0x80;
		}			
		//第五个数
		Write3rdChip_MAX7219(DIG_5,code);					

		//百位
		code=(data%1000)/100;
 		code=code_table[code];		
		if(point==3)
		{
			code=code | 0x80;
		}			
		//第六个数
		Write3rdChip_MAX7219(DIG_6,code);					

		//十位
		code=(data%100)/10;
 		code=code_table[code];			
		if(point==2)
		{
			code=code | 0x80;
		}			
		//第七个数
		Write3rdChip_MAX7219(DIG_7,code);					

		//个位
		code=data%10;
 		code=code_table[code];			
		if(point==1)
		{
			code=code | 0x80;
		}			
		//第八个数
		Write3rdChip_MAX7219(DIG_8,code);					
	}
}
