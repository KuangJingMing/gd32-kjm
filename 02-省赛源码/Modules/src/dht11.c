#include "dht11.h"

// 连接P10端口

uint16_t temperature, humidity;


void dht11_delay_us(uint32_t us) {
    taskDISABLE_INTERRUPTS();  // 进入临界区，禁用中断
    uint32_t i = us * 42;  // 粗略计算，需校准
    while (i--) {
        __NOP();  // 确保编译器不优化掉空循环
    }
    taskENABLE_INTERRUPTS();   // 退出临界区，恢复中断
}

/**************************************************************
 *功  能：复位DHT11
 *参  数: 无
 *返回值: 无
 **************************************************************/
static void DHT11_Rst(void) {
  DHT11_IO_OUT(); // SET OUTPUT
  DHT11_DQ_OUT_0; // 拉低DQ
  delay_1ms(20);  // 拉低至少18ms
  DHT11_DQ_OUT_1; // DQ=1
  dht11_delay_us(40);  // 主机拉高20~40us
}

/**************************************************************
 *功  能：等待DHT11的回应
 *参  数: 无
 *返回值: 返回1:未检测到DHT11的存在； 返回0:存在
 **************************************************************/
static uint8_t DHT11_Check(void) {
  uint8_t retry = 0;
  DHT11_IO_IN();                       // SET INPUT
  while (DHT11_DQ_IN() && retry < 100) // DHT11会拉低40~80us
  {
    retry++;
    dht11_delay_us(1);
  }
  if (retry >= 100)
    return 1;
  else
    retry = 0;
  while (!DHT11_DQ_IN() && retry < 100) // DHT11拉低后会再次拉高40~80us
  {
    retry++;
    dht11_delay_us(1);
  }
  if (retry >= 100)
    return 1;
  return 0;
}

/**************************************************************
 *功  能：从DHT11读取一个位
 *参  数: 无
 *返回值: 1/0
 **************************************************************/
static uint8_t DHT11_Read_Bit(void) {
  uint8_t retry = 0;
  while (DHT11_DQ_IN() && retry < 100) // 等待变为低电平
  {
    retry++;
    dht11_delay_us(1);
  }
  retry = 0;
  while (!DHT11_DQ_IN() && retry < 100) // 等待变高电平
  {
    retry++;
    dht11_delay_us(1);
  }
  dht11_delay_us(40); // 等待40us
  if (DHT11_DQ_IN())
    return 1;
  else
    return 0;
}

/**************************************************************
 *功  能：从DHT11读取一个字节
 *参  数: 无
 *返回值: 读到的数据
 **************************************************************/
static uint8_t DHT11_Read_Byte(void) {
  uint8_t i, dat;
  dat = 0;
  for (i = 0; i < 8; i++) {
    dat <<= 1;
    dat |= DHT11_Read_Bit();
  }
  return dat;
}

/**************************************************************
 *功  能：从DHT11读取一次数据
 *参  数: temp:温度值(范围:0~50°)；humi:湿度值(范围:20%~90%)
 *返回值: 0,正常;1,读取失败
 **************************************************************/
static uint8_t DHT11_Read_Data(uint16_t *temp, uint16_t *humi) {
  uint8_t buf[5];
  uint8_t i;
  DHT11_Rst();
  if (DHT11_Check() == 0) {
    for (i = 0; i < 5; i++) // 读取40位数据
    {
      buf[i] = DHT11_Read_Byte();
    }
    if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
      //			*humi = buf[0] * 10 + buf[1];
      //			*temp = buf[2] * 10 + buf[3];
      *humi = buf[0];
      *temp = buf[2];
    }
  } else
    return 1;
  return 0;
}

static void DQ_OUT(void) {
  rcu_periph_clock_enable(DQ_RTC);

  gpio_mode_set(DQ_GPIO_Port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, DQ_Pin);
  gpio_output_options_set(DQ_GPIO_Port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          DQ_Pin);
}

static void DQ_IN(void) {
  rcu_periph_clock_enable(DQ_RTC);

  gpio_mode_set(DQ_GPIO_Port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, DQ_Pin);
}

static void DHT11_Hardware_Init(void) {
  rcu_periph_clock_enable(DQ_RTC);

  gpio_mode_set(DQ_GPIO_Port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, DQ_Pin);
  gpio_output_options_set(DQ_GPIO_Port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          DQ_Pin);
}

static void DHT11_Software_Init(void) {
  DHT11_Rst(); // 复位DHT11
  while (DHT11_Check())
    ; // 返回1:不存在；返回0:存在
}

static void DHT11_Init(void) {
  DHT11_Hardware_Init();
  DHT11_Software_Init();
}

static void DHT11_Test(void) {
  DHT11_Read_Data(&temperature, &humidity); // 读取温湿度值
  oled.show_str(10, 6, "temperature", 16);
  oled.show_str(10, 3, "humidity", 16);
  oled.show_int32_num(100, 6, temperature, 3, 16);
  oled.show_int32_num(100, 3, humidity, 3, 16);
}

static uint16_t DHT11_GetTemperature(void) {
	delay_1ms(1);
	return temperature;
}

static uint16_t DHT11_GetHumidity(void) {
	delay_1ms(1);
	return humidity;
}

dht11_i dht11 = {
    .init = DHT11_Init,
    .read = DHT11_Update,
    .test = DHT11_Test,
		.get_temperature = DHT11_GetTemperature,
		.get_humidity = DHT11_GetHumidity,
};

static void DHT11_Update(void) {
  DHT11_Read_Data(&temperature, &humidity);
}
