#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"

void USART_Init(USART_Config *config) {
  // Enable GPIO clock
  rcu_periph_clock_enable(config->tx_rtc);
  rcu_periph_clock_enable(config->rx_rtc);

  // Enable USART clock
  rcu_periph_clock_enable(config->usartx_rtc);

  // Configure the USART TX and RX pins
  gpio_af_set(config->tx_port, config->af_x, config->tx_pin);
  gpio_af_set(config->rx_port, config->af_x, config->rx_pin);
  

  gpio_mode_set(config->tx_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                config->tx_pin);
  gpio_mode_set(config->rx_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                config->rx_pin);

  gpio_output_options_set(config->tx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          config->tx_pin);
  gpio_output_options_set(config->rx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          config->rx_pin);

  // USART configuration
  usart_deinit(config->usart_x);
  usart_baudrate_set(config->usart_x, config->baud);
  usart_parity_config(config->usart_x, USART_PM_NONE);
  usart_word_length_set(config->usart_x, USART_WL_8BIT);
  usart_stop_bit_set(config->usart_x, USART_STB_1BIT);
  usart_transmit_config(config->usart_x, USART_TRANSMIT_ENABLE);
  usart_receive_config(config->usart_x, USART_RECEIVE_ENABLE);
  usart_hardware_flow_coherence_config(config->usart_x, USART_HCM_NONE);

  // USART IRQn configure
  nvic_irq_enable(config->nvic_irq, 5, 0);
  usart_interrupt_enable(config->usart_x, USART_INT_RBNE);
  usart_data_first_config(config->usart_x, USART_MSBF_LSB);
  usart_enable(config->usart_x);
}

void USART_Send_Byte(USART_Config *config, uint8_t data) {
  usart_data_transmit(config->usart_x, data);
}

void USART_Send_Buff(USART_Config *config, uint8_t *data, uint32_t len) {
  uint32_t i = 0;
  while (len--) {
    USART_Send_Byte(config, data[i]);
    while (usart_flag_get(config->usart_x, USART_FLAG_TBE) == RESET);
    i++;
  }
}

void USART_Send_Data_length(USART_Config *config, uint8_t *data, uint32_t len) {
  uint32_t i = 0;
  while (len--) {
    USART_Send_Byte(config, data[i]);
    while (usart_flag_get(config->usart_x, USART_FLAG_TBE) == RESET);
    i++;
  }
}


void USART_printf(USART_Config *cfg, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    uint8_t buffer[256];
    int size = vsnprintf((char *)buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (size < 0 || size >= sizeof(buffer)) {
        return;  // If the formatted size is invalid or exceeds the buffer size, exit
    }
    
    USART_Send_Data_length(cfg, buffer, size + 1);  // Send the data via USART, including the null terminator
}
