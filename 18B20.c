#include "AllHead.h"
#include "18B20.h"

//IO����
#define DS18B20_DQ_H_1					(HT_GPIOB->PTSET |= (1<<4))
#define DS18B20_DQ_H_2					(HT_GPIOB->PTSET |= (1<<5))
#define DS18B20_DQ_L_1                  (HT_GPIOB->PTCLR |= (1<<4))
#define DS18B20_DQ_L_2                  (HT_GPIOB->PTCLR |= (1<<4))
#define DS18B20_DQ_ReadPin_1            (HT_GPIOB->PTDAT & (1<<4))
#define DS18B20_DQ_ReadPin_2            (HT_GPIOB->PTDAT & (1<<5))


//FIXME:��ʱ��������PB4��PB5
void DS18B20_DQ_DDR(BYTE ddr)
{
    
    if (ddr == 0)//����
    {
        HT_GPIOB->PTDIR &= 0xFFCF;//0:����
    }
    else if (ddr == 1)//���
    {
        HT_GPIOB->PTDIR |= 0x0030;//1�����
    }
    else
    {
        
    }

}

/*
��������DS18B20_reset
���ܣ���ʼ��DS18B20
���룺
�����
����ֵ����ʼ���ɹ�Ϊ0�����ɹ�Ϊ1
*/
BYTE DS18B20_reset(void) 
{
    //���ڵ�ƽ
    BYTE  Serial_Level = 0;
    //�ı�DQ����Ϊ���
    DS18B20_DQ_DDR(1);
    //���ø�
    DS18B20_DQ_H_1;
    //��ʱ700us��ʹ�����ȶ�
    api_Delay100us(7); 
    //��λ���壬�͵�λ
    DS18B20_DQ_L_1;
    //��������480us,����500us
    api_Delay100us(5); 
    //�ı�DQ����Ϊ����
    DS18B20_DQ_DDR(0);
    //���������ߣ��ͷ�����
    DS18B20_DQ_H_1;
    //�ȴ�15-60us������33us
    api_Delay10us(3); 
    //�ȴ�35us������33us
    api_Delay10us(3);  
    //�������ж���û�г�ʼ���ɹ�(DS18B20��û�з���Ӧ������)
    Serial_Level = DS18B20_DQ_ReadPin_1; 
    //����480us��������״̬������500us
    api_Delay100us(5);
    DS18B20_DQ_H_1;
    return Serial_Level;
}

/*
��������DS18B20_Wbyte
���ܣ�дһ���ֽ�
���룺uint8_t xbyte
�����
����ֵ��
*/
void DS18B20_Wbyte(BYTE Command_To_Send)
{
    //i:ѭ�����Ʊ�����wbit:ȡλ�������
    BYTE i = 0;
    BYTE wbit = 0;
    //�ı�DQ����Ϊ���
    DS18B20_DQ_DDR(1);
    //8��ѭ��ʵ����λд��
    for(i = 0; i < 8; i++)
    {
        //��ȡ��λ
        wbit = Command_To_Send & 0x01;
        //д1
        if(wbit)
        {
            DS18B20_DQ_H_1;
            //��������
            DS18B20_DQ_L_1;
            //��ʱ15us
            api_Delay10us(1);//����ܻ��е�����
            //����д1
            DS18B20_DQ_H_1;
            //��ʱ15us
            api_Delay10us(1);
            //���ָߵ�ƽ
            DS18B20_DQ_H_1;
            //delay_us(4);
            api_Delay10us(1);
        }
        //д0
        else
        {
            DS18B20_DQ_H_1;
            //��������
            DS18B20_DQ_L_1;
            //��ʱ15us
            api_Delay10us(1);
            //����д0
            DS18B20_DQ_L_1;
            //��ʱ15us
            api_Delay10us(3);
            //���ָߵ�ƽ
            DS18B20_DQ_H_1;
            api_Delay10us(1);
        }
        //Command_To_Send����һλ
        Command_To_Send = Command_To_Send >> 1;
    }
}

/*******************************************************************************
��������DS18B20_Rbit
���ܣ���DS18B20��һ��λ
���룺
�����
����ֵ����ȡ����λ
��ע��
*******************************************************************************/
BYTE DS18B20_Rbit(void)
{
    //rbit������λ���ݣ�Serial_Level��ȡ״̬����
    BYTE rbit = 0;
    BYTE i = 0;
    BYTE Serial_Level = 0;

    //�ı�DQΪ���ģʽ
    DS18B20_DQ_DDR(1);
    DS18B20_DQ_H_1;
    //����д0
    DS18B20_DQ_L_1;
    //��ʱ1us
    //api_Delay10us(1);//��������е�����
    api_DelayNop(12);


    //�ͷ�����
    DS18B20_DQ_H_1;
    //�ı�DQΪ����ģʽ
    DS18B20_DQ_DDR(0);
    api_DelayNop(12);

    //��ȡ���ߵ�ƽ״̬
    Serial_Level = DS18B20_DQ_ReadPin_1;
    //�����1���򷵻�0x80�����򷵻�0x00
    if (Serial_Level)
    {
        rbit = 0x80;
    }

    //�ͷ�����
    DS18B20_DQ_H_1;       
    //��ʱ��Լ60us
    api_Delay10us(6);

    return rbit;
}


/*******************************************************************************
��������DS18B20_Rbyte
���ܣ���DS18B20��һ���ֽ�
���룺
�����
����ֵ����ȡ�����ֽ�
��ע��
*******************************************************************************/
BYTE DS18B20_Rbyte(void)
{
    //rbyte�����յõ����ֽ�
    //tempbit���м��������
    BYTE rbyte = 0;
    BYTE i = 0;
    BYTE tempbit = 0;

    for (i = 0; i < 8; i++)
    {
        //��ȡλ
        tempbit = DS18B20_Rbit();
        //����ʵ�ָߵ�λ����
        rbyte = rbyte >> 1;
        //��������������
        rbyte = rbyte|tempbit;
    }
    return rbyte;
}

//��ȡ�¶ȵ�������
float ReadTemperature(void) 
{
    //Sign������λ
    //data���¶ȵ���������
    BYTE Sign = 0;
    WORD data = 0;
    BYTE TempL = 0;
    BYTE TempH = 0;
    float Measured_Temperature = 0;

    //DS18B20��ʼ��
    DS18B20_reset();
    //���������к�
    DS18B20_Wbyte(0xcc);
    //�����¶�ת��
    DS18B20_Wbyte(0x44);
    //�ȴ��¶�ת��
    api_Delayms(750);

    DS18B20_reset();
    DS18B20_Wbyte(0xcc);
    //���¶ȼĴ���
    DS18B20_Wbyte(0xbe); 
    TempL = DS18B20_Rbyte();
    TempH = DS18B20_Rbyte();
    //����λΪ��
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
        
    //��������
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
