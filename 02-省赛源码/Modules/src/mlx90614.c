#include "mlx90614.h"
#include "iic.h"


//连接P15端口

mlx90614_t mlx90614 = {
    .init = SMBus_Init,
    .get_temp = SMBus_ReadTemp,
};


// 定义 I2C 配置结构体
static IIC_Config i2c_config = {
    .scl_port = GPIOA,
    .scl_pin = GPIO_PIN_5,
    .scl_rtc = RCU_GPIOA,
    .sda_port = GPIOA,
    .sda_pin = GPIO_PIN_4,
    .sda_rtc = RCU_GPIOA
};

// 初始化函数
void SMBus_Init(void) {
    IIC_Init(&i2c_config);
}

// 计算并返回温度值
float SMBus_ReadTemp(void) {
    uint8_t data[3]; // [低字节, 高字节, PEC校验值]
    IIC_Start(&i2c_config);

    // 发送从设备地址（写模式）+ 寄存器地址
    IIC_Send_Byte(&i2c_config, (SA << 1) | 0);
    if (IIC_Wait_Ack(&i2c_config)) {
        IIC_Stop(&i2c_config);
        return 0.0; // 通信失败
    }
    IIC_Send_Byte(&i2c_config, RAM_TA);
    if (IIC_Wait_Ack(&i2c_config)) {
        IIC_Stop(&i2c_config);
        return 0.0; // 通信失败
    }

    // 重新启动，切换到读模式
    IIC_Start(&i2c_config);
    IIC_Send_Byte(&i2c_config, (SA << 1) | 1);
    if (IIC_Wait_Ack(&i2c_config)) {
        IIC_Stop(&i2c_config);
        return 0.0; // 通信失败
    }

    // 读取温度数据（2字节）和 PEC 校验
    data[0] = IIC_Read_Byte(&i2c_config, 1); // 低字节 + ACK
    data[1] = IIC_Read_Byte(&i2c_config, 1); // 高字节 + ACK
    data[2] = IIC_Read_Byte(&i2c_config, 0); // PEC 校验字节 + NACK
    IIC_Stop(&i2c_config);

    // 组合温度数据
    uint16_t tempRaw = (data[1] << 8) | data[0];
    float temperature = (tempRaw * 0.02) - 273.15;

    return temperature;
}

