#include "iic.h"
#include "systick.h"

// Macros for setting and reading pins
#define I2C_SCL_LOW(cfg) (GPIO_BC(cfg->scl_port) |= cfg->scl_pin)
#define I2C_SCL_HIGH(cfg) (GPIO_BOP(cfg->scl_port) |= cfg->scl_pin)
#define I2C_SDA_LOW(cfg) (GPIO_BC(cfg->sda_port) |= cfg->sda_pin)
#define I2C_SDA_HIGH(cfg) (GPIO_BOP(cfg->sda_port) |= cfg->sda_pin)
#define I2C_SDA_READ(cfg) (GPIO_ISTAT(cfg->sda_port) & cfg->sda_pin)

void IIC_Init(IIC_Config *cfg) {
  rcu_periph_clock_enable(cfg->scl_rtc);
  rcu_periph_clock_enable(cfg->sda_rtc);

  gpio_mode_set(cfg->scl_port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,
                cfg->scl_pin);
  gpio_output_options_set(cfg->scl_port, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,
                          cfg->scl_pin);
  gpio_mode_set(cfg->sda_port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,
                cfg->sda_pin);
  gpio_output_options_set(cfg->sda_port, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,
                          cfg->sda_pin);

  I2C_SCL_HIGH(cfg);
  delay_1us(4);
  I2C_SDA_HIGH(cfg);
  delay_1us(4);
}

void SDA_IN(IIC_Config *cfg) {
  rcu_periph_clock_enable(cfg->sda_rtc);
  gpio_mode_set(cfg->sda_port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, cfg->sda_pin);
}

void SDA_OUT(IIC_Config *cfg) {
  rcu_periph_clock_enable(cfg->sda_rtc);
  gpio_mode_set(cfg->sda_port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,
                cfg->sda_pin);
  gpio_output_options_set(cfg->sda_port, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,
                          cfg->sda_pin);

  I2C_SDA_HIGH(cfg);
}

void IIC_Start(IIC_Config *cfg) {
  SDA_OUT(cfg);
  I2C_SDA_HIGH(cfg);
  delay_1us(5);
  I2C_SCL_HIGH(cfg);
  delay_1us(5);
  I2C_SDA_LOW(cfg); // START: when CLK is high, DATA changes from high to low
  delay_1us(5);
  I2C_SCL_LOW(cfg); // Pull down I2C clock line, prepare to send data
}

void IIC_Stop(IIC_Config *cfg) {
  SDA_OUT(cfg);
  I2C_SCL_LOW(cfg);
  I2C_SDA_LOW(cfg); // STOP: when CLK is high, DATA changes from low to high
  delay_1us(5);
  I2C_SCL_HIGH(cfg);
  I2C_SDA_HIGH(cfg); // Send I2C stop signal
  delay_1us(5);
}

uint8_t IIC_Wait_Ack(IIC_Config *cfg) {
  uint16_t ucErrTime = 0;
  SDA_IN(cfg); // Set SDA as input
  I2C_SDA_HIGH(cfg);
  delay_1us(1);
  I2C_SCL_HIGH(cfg);
  delay_1us(1);
  while (I2C_SDA_READ(cfg)) {
    ucErrTime++;
    if (ucErrTime > 250) {
      IIC_Stop(cfg);
      return 1; // Acknowledge error
    }
  }
  I2C_SCL_LOW(cfg); // Release SDA line
  return 0;
}

void IIC_Ack(IIC_Config *cfg) {
  I2C_SCL_LOW(cfg);
  SDA_OUT(cfg);
  I2C_SDA_LOW(cfg);
  delay_1us(2);
  I2C_SCL_HIGH(cfg);
  delay_1us(5);
  I2C_SCL_LOW(cfg);
}

void IIC_NAck(IIC_Config *cfg) {
  I2C_SCL_LOW(cfg);
  SDA_OUT(cfg);
  I2C_SDA_HIGH(cfg);
  delay_1us(2);
  I2C_SCL_HIGH(cfg);
  delay_1us(5);
  I2C_SCL_LOW(cfg);
}

void IIC_Send_Byte(IIC_Config *cfg, uint8_t txd) {
  uint8_t t;
  SDA_OUT(cfg);
  I2C_SCL_LOW(cfg); // Prepare to send data
  for (t = 0; t < 8; t++) {
    if ((txd & 0x80) >> 7) {
      I2C_SDA_HIGH(cfg);
    } else {
      I2C_SDA_LOW(cfg);
    }
    txd <<= 1;
    delay_1us(2);
    I2C_SCL_HIGH(cfg);
    delay_1us(2);
    I2C_SCL_LOW(cfg);
    delay_1us(2);
  }
}

uint8_t IIC_Read_Byte(IIC_Config *cfg, unsigned char ack) {
  unsigned char i, receive = 0;
  SDA_IN(cfg); // Set SDA as input
  for (i = 0; i < 8; i++) {
    I2C_SCL_LOW(cfg);
    delay_1us(2);
    I2C_SCL_HIGH(cfg);
    receive <<= 1;
    if (I2C_SDA_READ(cfg))
      receive++;
    delay_1us(1);
  }
  if (!ack)
    IIC_NAck(cfg); // Send NACK
  else
    IIC_Ack(cfg); // Send ACK
  return receive;
}

void IIC_Master_Transmit(IIC_Config *cfg, uint16_t DevAddress, uint8_t *pData,
                         uint16_t Size) {
  IIC_Start(cfg); // Send start signal
  IIC_Send_Byte(cfg, (uint8_t)(DevAddress << 1) |
                         0); // Send device address with write flag
  IIC_Wait_Ack(cfg);         // Wait for ACK
  while (Size--) {           // Send data
    IIC_Send_Byte(cfg, *pData++);
    IIC_Wait_Ack(cfg); // Wait for ACK
  }
  IIC_Stop(cfg); // Send stop signal
}

uint8_t IIC_Master_Receive(IIC_Config *cfg, uint16_t DevAddress, uint8_t *pData,
                           uint16_t Size) {
  IIC_Start(cfg); // Send start signal
  IIC_Send_Byte(cfg, (uint8_t)(DevAddress << 1) |
                         1); // Send device address with read flag
  if (IIC_Wait_Ack(cfg)) {   // Wait for ACK
    IIC_Stop(cfg);           // If not acknowledged, send stop signal
    return 1;                // Indicate error
  }
  while (Size) {
    if (Size == 1) {
      *pData = IIC_Read_Byte(cfg, 0); // Read data without ACK
    } else {
      *pData = IIC_Read_Byte(cfg, 1); // Read data with ACK
    }
    pData++;
    Size--;
  }
  IIC_Stop(cfg); // Send stop signal
  return 0;      // Indicate success
}
