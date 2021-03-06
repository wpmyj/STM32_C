/*
程序说明:  	
            ADC采集处理程序 --- STM32F107VCT 
						
注意事项：	在定义ADC寄存器地址时，一定要使用__IO uint16_t数据格式，否则会出错。
						另外需注意ADCColValue[ADCCollectTime][ADCChannelNum]此数组的结构。					
						

创建者:     FangYikaii   
创建时间:   2017-01-18 
修改时间:   2017-01-18
*/

/*
ADC：Analog-To-Digital Converter 模拟信号：信息参数在给定范围内表现为连续信号(电压/电流/声音) 数字信号：幅度的取值是离散的，幅值表示为被限制在有限个数值内
ADC: 转换精度有关,一般12位精度---对模拟信号逐次逼近转化为数字信号来做的（通过模拟信号与芯片内部模拟信号部件，也就是参考电压的对比，转变成数字信号给芯片内部的数字信号寄存器，用0/1放在寄存器）
步骤：1、采样：把一段模拟信号分解为许多节点�
			2、保持：把信号保持一段时间，否则可能是一个干扰或者信号毛刺，后续的量化过程需要一段的时间
			3、量化：幅值量化，把采样信号通过舍入或截尾的方法变为只有有限个有效数字的数
			4、编码：将离散幅值变为二进制数

ADC分辨率；输出数字量变化一个最小量时，模拟信号的变化量，8位AD,可以描述255个刻度的精度,比如测量5v电压，精度5/255=0.02v的分辨率
量化误差：ADC有限位数对模拟量进行量化而引起的误差
转换速率：转换速率是指完成一次从模拟转换到数字所需要的时间的倒数
线性度：实际转换特性与理想直线的最大偏差
绝对精度：实际模拟量与理论模拟输入的误差的最大值

ADC供电要求：2.4V-3.6V

使用方法：1.配置最大值和最小值 ADCRes.ADCMaxValue[ADCChannelNum] ADCRes.ADCMinValue[ADCChannelNum] 
					2.采集每秒，每个通道的ADC值，计算采集个数并求和---计算每秒每个通道的ADC平均值，此时平均值为电压
					3.例：如果测压力，压力值0-25MPA，用归一的值*压力系数  (ADCRes.ADCSumValue[5]-ADCRes.ADCMinValue[5])*25/(ADCRes.ADCMaxValue[5]-ADCRes.ADCMinValue[5])  【压力】
					4.ADC的和，ADC的Count归零
*/
//========================================
//头文件
#include "main.h"

//========================================
//变量定义
volatile union FlagADCA flagADC;
struct ADCResult ADCRes;

//========================================
//函数声明
void ADC_Configuration(void);										//完成ADC的配置
void ADC_ValueFilter(void);											//ADC采集值滤波


//========================================
//子模块函数


/***********************************************************************
函数名称：void ADC_Configuration(void)
功    能：完成ADC的配置
输入参数：
输出参数：
编写时间： 2017-01-18 
编 写 人： FangYikaii
注    意：
***********************************************************************/
void ADC_Configuration(void)
{

	ADC_InitTypeDef ADC_InitStructure;      //ADC初始化结构体声明
	DMA_InitTypeDef DMA_InitStructure;      //DMA初始化结构体声明
  GPIO_InitTypeDef GPIO_InitStructure;    //GPIO初始化结构体声明
	NVIC_InitTypeDef NVIC_InitStructure;    //NVIC初始化结构体声明
	
	//GPIO的配置
	
  /* Configure PC0-PC6 (ADC Channel10-15) as analog input -------------------------*/
  //PC0 作为模拟通道10输入引脚                         
  GPIO_InitStructure.GPIO_Pin = ADCNN11_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;      //模拟输入
  GPIO_Init(ADCNN11_GPIOX, &GPIO_InitStructure);
  RCC_APB2PeriphClockCmd(ADCNN11_RCC_APB2Periph_GPIOX, ENABLE);	
	
  //PC1 作为模拟通道11输入引脚                         
  GPIO_InitStructure.GPIO_Pin = ADCNN12_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(ADCNN12_GPIOX, &GPIO_InitStructure);	
  RCC_APB2PeriphClockCmd(ADCNN12_RCC_APB2Periph_GPIOX, ENABLE);	
	
  //PC2 作为模拟通道12输入引脚                         
  GPIO_InitStructure.GPIO_Pin = ADCNN13_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(ADCNN13_GPIOX, &GPIO_InitStructure);	
  RCC_APB2PeriphClockCmd(ADCNN13_RCC_APB2Periph_GPIOX, ENABLE);
	
  //PC3 作为模拟通道13输入引脚                         
  GPIO_InitStructure.GPIO_Pin = ADCNN14_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(ADCNN14_GPIOX, &GPIO_InitStructure);	
  RCC_APB2PeriphClockCmd(ADCNN14_RCC_APB2Periph_GPIOX, ENABLE);
	
  //PC4 作为模拟通道14输入引脚                         
  GPIO_InitStructure.GPIO_Pin = ADCNN15_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(ADCNN15_GPIOX, &GPIO_InitStructure);	
  RCC_APB2PeriphClockCmd(ADCNN15_RCC_APB2Periph_GPIOX, ENABLE);
	
  //PC5 作为模拟通道15输入引脚                         
  GPIO_InitStructure.GPIO_Pin = ADCNN16_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(ADCNN16_GPIOX, &GPIO_InitStructure);	
  RCC_APB2PeriphClockCmd(ADCNN1_RCC_APB2Periph_GPIOX, ENABLE);
	
	//DMA的配置 DMA1
	
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;    								//DMA对应的外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCRes.ADCColValue;   			//内存存储基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;														//DMA的转换模式为SRC模式，由外设搬移到内存
  DMA_InitStructure.DMA_BufferSize = ADCCollectTime*ADCChannelNum;		   				//DMA缓存大小，ADCCollectTime*ADCChannelNum个,单位为DMA_MemoryDataSize
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;							//接收一次数据后，设备地址禁止后移
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;												//关闭接收一次数据后，目标内存地址后移
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  	//定义外设数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  					//DMA搬数据尺寸，HalfWord就是为16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   														//转换模式，循环缓存模式。
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;														//DMA优先级高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  														//M2M模式禁用
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);           												//DMA_Channel1 通道初始化
	
 	DMA_ITConfig(DMA1_Channel1,DMA1_IT_TC1, ENABLE);																//DMA_Channel1 通道中断配置--6通道多次采样，耗时约为3ms	 
 	DMA_ClearITPendingBit(DMA1_IT_TC1);

	/* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

	//NVIC的配置，2抢断，0响应
	
	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);			//抢断优先级 0-15   响应优先级 0
	/* Enable the TIM5 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	//ADC的配置，ADC1

  /* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  										//独立的转换模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;		  											//开启扫描模式
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;   										//开启连续转换模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//ADC外部开关，关闭状态
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   								//对齐方式,ADC为12位中，右对齐方式
  ADC_InitStructure.ADC_NbrOfChannel = ADCChannelNum;	 										//开启通道数，ADCChannelNum个
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel10 configuration ADC通道组， 第10个通道 采样顺序1，转换时间 */ 
  ADC_RegularChannelConfig(ADC1, ADCNN11_ChannelNO	, 1, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADCNN12_ChannelNO	, 2, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADCNN13_ChannelNO	, 3, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADCNN14_ChannelNO	, 4, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADCNN15_ChannelNO	, 5, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADCNN16_ChannelNO	, 6, ADC_SampleTime_55Cycles5);	
	
	//ADC1转换出来的数据通过DMA1传输	
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);	  															//ADC命令，使能DMA传输
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);  																	//开启ADC1
  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);	  														//重新校准
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));  							//等待重新校准完成
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);																//开始校准
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));	   								//等待校准完成
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);										//连续转换开始，ADC通过DMA方式不断的更新RAM区。

}

/***********************************************************************
函数名称：void ADC_ValueFilter(void)
功    能：ADC采集值滤波--耗时约3ms
输入参数：
输出参数：
编写时间：2017.01.18
编 写 人：
注    意：
***********************************************************************/
void ADC_ValueFilter(void)
{
	uint32_t sum;
	uint16_t t;
	uint16_t i,j,k;
	float fvalue;
	
	//ADCRes.ADCColValue[][k]按大到小顺序排列
	//采集通道个数的循环
	for(k=0;k<ADCChannelNum;k++)			
	{
		//采集时间内的循环
		for(i=0;i<(ADCCollectTime-1);i++)
		{			
			for(j=i+1;j<ADCCollectTime;j++)
			{
				if(ADCRes.ADCColValue[i][k]>ADCRes.ADCColValue[j][k])
				{
					t=ADCRes.ADCColValue[i][k];
					ADCRes.ADCColValue[i][k]=ADCRes.ADCColValue[j][k];
					ADCRes.ADCColValue[j][k]=t;
				}				
			}
		}		
	}
	//采集通道个数的循环
	for(k=0;k<ADCChannelNum;k++)									//均值滤波
	{
		sum=0;
		for(i=5;i<(ADCCollectTime-5);i++)
		{
			sum+=ADCRes.ADCColValue[i][k];	
		}
		//
		fvalue=(3.3*sum)/4096;			 								//2^12=4096,将AD值转换为电压值3.3V		
		//采样时间内的均值
		fvalue=fvalue/(ADCCollectTime-10);	
		ADCRes.ADCResValue[k]=fvalue;
//		fvalue=fvalue*1000;
//		ADCRes.ADCResValue[k]=(uint16_t)fvalue;			
	}
}

//========================================
//函数名称:void TIM5_IRQHandler(void)
//函数功能: TIM5中断处理程序
//入口参数:    
//出口参数:      
//========================================
/**
  * @brief  This function handles TIM5 global interrupt request.
  * @param  None
  * @retval None
  */
void DMA1_Channel1_IRQHandler(void)
{
	if(DMA1_IT_TC1)
	{
		flagADC.Bits.ADCOK=TRUE;
		DMA_ClearITPendingBit(DMA1_IT_TC1);		
	}

}
