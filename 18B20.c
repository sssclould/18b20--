#include "AllHead.h"
#include "18B20.h"

//IO操作
#define DS18B20_DQ_H_1					(HT_GPIOB->PTSET |= (1<<4))
#define DS18B20_DQ_H_2					(HT_GPIOB->PTSET |= (1<<5))
#define DS18B20_DQ_L_1                  (HT_GPIOB->PTCLR |= (1<<4))
#define DS18B20_DQ_L_2                  (HT_GPIOB->PTCLR |= (1<<5))
#define DS18B20_DQ_ReadPin_1            (HT_GPIOB->PTDAT & (1<<4))
#define DS18B20_DQ_ReadPin_2            (HT_GPIOB->PTDAT & (1<<5))


//FIXME:暂时单独配置PB4和PB5
void DS18B20_DQ_DDR(BYTE ddr)
{
    
    if (ddr == 0)//输入
    {
        HT_GPIOB->PTDIR &= 0xFFCF;//0:输入
    }
    else if (ddr == 1)//输出
    {
        HT_GPIOB->PTDIR |= 0x0030;//1：输出
    }
    else
    {
        
    }

}

/*
函数名：DS18B20_reset
功能：初始化DS18B20
输入：
输出：
返回值：初始化成功为0，不成功为1
*/
BYTE DS18B20_reset(void) 
{
    //串口电平
    BYTE  Serial_Level = 0;
    //改变DQ引脚为输出
    DS18B20_DQ_DDR(1);
    //先置高
    DS18B20_DQ_H_1;
    //延时700us，使总线稳定
    api_Delay100us(7); 
    //复位脉冲，低电位
    DS18B20_DQ_L_1;
    //保持至少480us,这里500us
    api_Delay100us(5); 
    //改变DQ引脚为输入
    DS18B20_DQ_DDR(0);
    //拉高数据线，释放总线
    DS18B20_DQ_H_1;
    //等待15-60us，这里33us
    api_Delay10us(3); 
    //等待35us，这里33us
    api_Delay10us(3);  
    //聆听，判断有没有初始化成功(DS18B20有没有发送应答脉冲)
    Serial_Level = DS18B20_DQ_ReadPin_1; 
    //至少480us后进入接收状态，这里500us
    api_Delay100us(5);
    DS18B20_DQ_H_1;
    return Serial_Level;
}

/*
函数名：DS18B20_Wbyte
功能：写一个字节
输入：uint8_t xbyte
输出：
返回值：
*/
void DS18B20_Wbyte(BYTE Command_To_Send)
{
    //i:循环控制变量，wbit:取位运算变量
    BYTE i = 0;
    BYTE wbit = 0;
    //改变DQ引脚为输出
    DS18B20_DQ_DDR(1);
    //8次循环实现逐位写入
    for(i = 0; i < 8; i++)
    {
        //先取低位
        wbit = Command_To_Send & 0x01;
        //写1
        if(wbit)
        {
            DS18B20_DQ_H_1;
            //拉低总线
            DS18B20_DQ_L_1;
            //延时15us
            api_Delay10us(1);//这可能会有点问题
            //总线写1
            DS18B20_DQ_H_1;
            //延时15us
            api_Delay10us(1);
            //保持高电平
            DS18B20_DQ_H_1;
            //delay_us(4);
            api_Delay10us(1);
        }
        //写0
        else
        {
            DS18B20_DQ_H_1;
            //总线拉低
            DS18B20_DQ_L_1;
            //延时15us
            api_Delay10us(1);
            //总线写0
            DS18B20_DQ_L_1;
            //延时15us
            api_Delay10us(3);
            //保持高电平
            DS18B20_DQ_H_1;
            api_Delay10us(1);
        }
        //Command_To_Send右移一位
        Command_To_Send = Command_To_Send >> 1;
    }
}

/*******************************************************************************
函数名：DS18B20_Rbit
功能：从DS18B20读一个位
输入：
输出：
返回值：读取到的位
备注：
*******************************************************************************/
BYTE DS18B20_Rbit(void)
{
    //rbit是最终位数据，Serial_Level是取状态变量
    BYTE rbit = 0;
    BYTE i = 0;
    BYTE Serial_Level = 0;

    //改变DQ为输出模式
    DS18B20_DQ_DDR(1);
    DS18B20_DQ_H_1;
    //总线写0
    DS18B20_DQ_L_1;
    //延时1us
    //api_Delay10us(1);//这里可能有点问题
    api_DelayNop(12);


    //释放总线
    DS18B20_DQ_H_1;
    //改变DQ为输入模式
    DS18B20_DQ_DDR(0);
    api_DelayNop(12);

    //获取总线电平状态
    Serial_Level = DS18B20_DQ_ReadPin_1;
    //如果是1，则返回0x80，否则返回0x00
    if (Serial_Level)
    {
        rbit = 0x80;
    }

    //释放总线
    DS18B20_DQ_H_1;       
    //延时大约60us
    api_Delay10us(6);

    return rbit;
}


/*******************************************************************************
函数名：DS18B20_Rbyte
功能：从DS18B20读一个字节
输入：
输出：
返回值：读取到的字节
备注：
*******************************************************************************/
BYTE DS18B20_Rbyte(void)
{
    //rbyte：最终得到的字节
    //tempbit：中间运算变量
    BYTE rbyte = 0;
    BYTE i = 0;
    BYTE tempbit = 0;

    for (i = 0; i < 8; i++)
    {
        //读取位
        tempbit = DS18B20_Rbit();
        //右移实现高低位排序
        rbyte = rbyte >> 1;
        //或运算移入数据
        rbyte = rbyte|tempbit;
    }
    return rbyte;
}

//读取温度的主程序
float ReadTemperature(void) 
{
    //Sign：符号位
    //data：温度的整数部分
    BYTE Sign = 0;
    WORD data = 0;
    BYTE TempL = 0;
    BYTE TempH = 0;
    float Measured_Temperature = 0;

    //DS18B20初始化
    DS18B20_reset();
    //跳过读序列号
    DS18B20_Wbyte(0xcc);
    //启动温度转换
    DS18B20_Wbyte(0x44);
    //等待温度转换
    api_Delayms(750);

    DS18B20_reset();
    DS18B20_Wbyte(0xcc);
    //读温度寄存器
    DS18B20_Wbyte(0xbe); 
    TempL = DS18B20_Rbyte();
    TempH = DS18B20_Rbyte();
    //符号位为负
    if(TempH > 0x70)
    {
        TempL = ~TempL;
        TempH = ~TempH;
        Sign = 0;
    }
    else
    {
        Sign = 1;
    }
        
    //整数部分
    data = TempH;
    data <<=  8;
    data += TempL;
    Measured_Temperature = (float)data*0.0625;



    if (Sign)
    {
        return Measured_Temperature;
    }  
    else
    {
        return -Measured_Temperature;
    }
        
}
