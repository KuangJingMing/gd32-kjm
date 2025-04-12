#include "hx711.h"
#include "oled.h"


//Á¬½ÓP19¶Ë¿Ú

static unsigned long HX711_Buffer = 0;
static unsigned long Weight_Maopi = 0;
static long Weight_object = 0;
static int GapValue = 550;

static unsigned long HX711_Read(void) {
  uint32_t Count = 0;
  uint8_t i = 0;
	
  HX711_SCK_LOW;
  while (HX711_DOUT_IN);

  for (i = 0; i < 24; i++) {

    HX711_SCK_HIGH;
    Count = Count << 1;
    HX711_SCK_LOW;
    if (HX711_DOUT_IN)
      Count++;
  }
  HX711_SCK_HIGH;
  Count = Count ^ 0x800000;
  HX711_SCK_HIGH;

  return Count;
}

static void Get_Maopi(void) { Weight_Maopi = HX711_Read(); }

static void GPIO_HX711_Init(void) {
  rcu_periph_clock_enable(HX711_SCK_RTC);
  rcu_periph_clock_enable(HX711_DOUT_RTC);

  gpio_mode_set(HX711_SCK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,
                HX711_SCK_PIN);
  gpio_output_options_set(HX711_SCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,
                          HX711_SCK_PIN);

  gpio_mode_set(HX711_DOUT_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
                HX711_DOUT_PIN);
  Get_Maopi();
}

static long Get_Weight(void) {

  HX711_Buffer = HX711_Read();

  Weight_object = HX711_Buffer;
  Weight_object = Weight_object - Weight_Maopi;

  if (Weight_object > 0) {
    Weight_object = ((float)Weight_object / GapValue);
  } else {
    Weight_object = 0;
  }

  return Weight_object;
}

static void test(void) {
  char str[40];
  Get_Weight();
  sprintf(str, "weight: %d g   ", (int)Weight_object);
  oled.show_str(16, 2, str, 16);
}

hx711_i hx711 = {
    .init = GPIO_HX711_Init,
    .test = test,
    .get_weight = Get_Weight,
};
