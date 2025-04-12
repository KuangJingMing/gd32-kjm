#include "button_callback.h"

static TaskHandle_t taskHandlers[100];

//通用打印回调 START
void task_log(const char *fmt, ...) {
    char formattedData[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(formattedData, 256, fmt, args);
    va_end(args);
		hmi.send_string("t0.txt", formattedData);
		LOG_I("%s", formattedData);
}
//通用打印回调 END

//数码管计时任务 START
static void TASK1_Thread(void *arg) {
	nixie_tube.init();
	dht11.init();
	fan.init();
	rgb_led.init();
	for ( ; ; ) {
		dht11.read();
		uint16_t val = dht11.get_temperature();
		nixie_tube.show_num(val);
		if (val <= 25) {
			rgb_led.set_color(COLOR_BLUE);
			fan.set_speed(0);
		}
		else {
			rgb_led.set_color(COLOR_OFF);
			fan.set_speed(1000);
		}
	}
}

static void task1_releases(void) {
	nixie_tube.off();
	fan.set_speed(0);
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//数码管计时任务 END


//LED控制任务 START 
static void waterfall_led(void)
{
	rgb_led.set_color(COLOR_BLUE);
	delay_1ms(1000);
	rgb_led.set_color(COLOR_YELLOW);
	delay_1ms(1000);
	beep.control(1);
	rgb_led.set_color(COLOR_WHITE);
	delay_1ms(1000);
	beep.control(0);
	rgb_led.off();
	delay_1ms(3000);
}

static void TASK2_Thread(void *arg)
{
		rgb_led.init();
		beep.init();
    for (;;) 
    {
			waterfall_led();
			delay_1ms(10);
    }
}
static void task2_releases(void) {
	beep.control(0);
	rgb_led.off();
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//LED控制任务 END

uint16_t speed = 0;

//语音控制任务 START
static void bkrc_callback(uint8_t data) {
	switch (data) {
			case 26:
				hmi.printf("t0.txt", "紧急求救");
				for (int i = 0; i < 3; i++) {
					beep.control(1);
					rgb_led.set_color(COLOR_RED);
					delay_1ms(500);
					beep.control(0);
					rgb_led.set_color(COLOR_OFF);
					delay_1ms(500);
				}
				break;
			case 27:
				hmi.printf("t0.txt", "安全模式");
				for (int i = 100; i >= 0; i--) {
					rgb_led.set_pwm_timer(COLOR_BLUE, i, LED_DEFAULT_FREQ);
					delay_1ms(15);
				}
				break;
	}
}
static void TASK3_Thread(void *arg)
{
		rgb_led.init();
		beep.init();
		bkrc_speak.init();
		bkrc_speak.send_cmd(69);
		bkrc_speak.callback = bkrc_callback;
    for (;;)
    {
			bkrc_speak.asr();
			vTaskDelay(10);
    }
}
static void task3_releases(void) {
	rgb_led.stop_pwm_timer();
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//语音控制任务 END

//超声波测距任务 START
static void TASK4_Thread(void *arg) {
	ultrasonic.init();
	beep.init();
	rgb_led.init();
	for ( ; ; ) {
		float val = ultrasonic.get_cm();
		hmi.printf("t0.txt", "超声波距离为：%.2lf", val);
		if (val < 15) {
			rgb_led.blink_control(COLOR_RED, 500);
		} else if (val >= 15 && val <= 20) {
			beep.control(0);
			rgb_led.off();
		}
		else if (val > 20) {
			rgb_led.blink_control(COLOR_GREEN, 500);
			beep.control(0);
		}
		delay_1ms(10);
	}
}
static void task4_releases(void) {
	beep.control(0);
	rgb_led.off();
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//超声波测距任务 END

//DHT11测温任务 START

#define TIME_HISTORY_SIZE 7

// DHT11 测温任务
static void TASK5_Thread(void *arg) {
    dht11.init();
		ds18b20.init();
    rtc_clock_config();
    hmi.printf("t7.txt", "温湿度控制系统");
		static RTC_Time time_history[TIME_HISTORY_SIZE];
		static char txts[][30] = {
			"t14.txt",
			"t13.txt",
			"t11.txt",
			"t10.txt",
			"t9.txt",
			"t8.txt",
			"t6.txt",
		};
    // 初始化时间历史数组
    for (int i = 0; i < TIME_HISTORY_SIZE; i++) {
        time_history[i].minute = 0;
        time_history[i].second = 0;
    }

    for (;;) {
        // 获取当前时间
        RTC_Time current_time = rtc_get_time();

        // 位移赋值：将历史时间向后移动
        for (int i = TIME_HISTORY_SIZE - 1; i > 0; i--) {
            time_history[i] = time_history[i - 1];
        }
        time_history[0] = current_time; // 最新时间放在索引 0

        // 发送温湿度数据到曲线
        dht11.read();
        hmi.send_to_curve("s0", 0, ds18b20.get_temperature(), 0, 120);
        hmi.send_to_curve("s0", 1, dht11.get_humidity(), 0, 120);

        // 显示时间到 HMI
				for (int i = 0; i < TIME_HISTORY_SIZE; i++) {
					hmi.printf(txts[i], "%d:%d", time_history[i].minute, time_history[i].second);
				}
        // FreeRTOS 延时
        vTaskDelay(pdMS_TO_TICKS(100)); // 延时 100ms
    }
}

static void task5_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//DHT11测温任务 END

//HX711称重任务 START
static void TASK6_Thread(void *arg) {
	hx711.init();
	rgb_led.init();
	uint8_t light = 0;
	uint8_t light_flag = 0;
	for ( ; ; ) {
		long data = hx711.get_weight();
		LOG_I("data = %d", (int)data);
		if (data >= 40 && data <= 60) {
			hmi.send_string("t0.txt", " 乘客已上车 ");
			rgb_led.set_pwm_timer(COLOR_RED, light, LED_DEFAULT_FREQ); 
		} else {
			hmi.send_string("t0.txt", " 无效数字 ");
		}
		if (light_flag == 0) light += 20;
		else if (light_flag == 1) light -= 20;
		if (light >= 100) light_flag = 1;
		else if (light == 0) light_flag = 0;
		delay_1ms(10);
	}
}

static void task6_releases(void) {
	rgb_led.stop_pwm_timer();
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}

//HX711称重任务 END

//滑块LED任务 START
static void TASK7_Thread(void *arg) {
	rgb_led.init();
	uint16_t slider_vals[3];
	for ( ; ; ) {
		slider_vals[0] = hmi.get_widget_val(lcd_rx_data, lcd_data_size, "slider1");
		slider_vals[1] = hmi.get_widget_val(lcd_rx_data, lcd_data_size, "slider2");
		slider_vals[2] = hmi.get_widget_val(lcd_rx_data, lcd_data_size, "slider3");
		uint16_t val = 0;
		for (int i = 0; i < 3; i++) if (slider_vals[i] > val) val = slider_vals[i];
		rgb_led.set_pwm_timer((Color){slider_vals[0], slider_vals[1], slider_vals[2]}, val, LED_DEFAULT_FREQ);
		LOG_I("%d", val);
		delay_1ms(10);
	}
}


static void task7_releases(void) {
	rgb_led.stop_pwm_timer();
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//滑块LED任务 END

// RC522刷卡任务 START
static void rc522_log_callback(const char *fmt, ...) {
    char formattedData[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(formattedData, 256, fmt, args);
    va_end(args);
		hmi.send_string("t0.txt", formattedData);
}

static void task8_read_card(void *arg) {
	LOG_I("read");
	rc522.read(10, 0, NULL);
}

static void task8_write_card(void *arg) {
	LOG_I("write");
	rc522.write(10, 0, "KAUNG");
}

static void TASK8_Thread(void *arg) {
	rc522.init();
	rc522.log = rc522_log_callback;
	for ( ; ; ) {
			delay_1ms(1000);
	}
}

static void task8_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//RC522刷卡任务 END

//C1016指纹任务 START
static void task9_enroll(void *arg) {
	c1016.enroll();
}
static void task9_identify(void *arg) {
	c1016.identfify();
}

static void task9_delete(void *arg) {
	uint16_t val = hmi.get_widget_val(lcd_rx_data, lcd_data_size, "id");
	LOG_I("val = %d", val);
	c1016.delete_id(1, val);
}

static void TASK9_Thread(void *arg) {
	c1016.show_log = task_log;
	c1016.init();
	vTaskDelete(NULL);
	for ( ; ; ) {
			delay_1ms(1000);
	}
}

static void task9_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//C1016指纹任务 END

//蜂鸣器任务 START
static void TASK10_Thread(void *arg) {
	beep.init();
	beep.play_music(music_example_7, music_example_7_size);
	for ( ; ; ) {
			delay_1ms(1000);
	}
}

static void task10_releases(void) {
	beep.control(0);
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//蜂鸣器任务 END


//光照曲线任务 START
static void TASK11_Thread(void *arg) {
    bh1750.init();
    rtc_clock_config();
    static RTC_Time time_history[TIME_HISTORY_SIZE];
    static char txts[][30] = {
			"t6.txt",
			"t8.txt",
			"t9.txt",
			"t10.txt",
			"t11.txt",
			"t13.txt",
			"t14.txt",
    };
    
    // 初始化时间历史数组
    for (int i = 0; i < TIME_HISTORY_SIZE; i++) {
        time_history[i].minute = 0;
        time_history[i].second = 0;
    }

    // 使用Tick计数来跟踪时间
    TickType_t last_update_ticks = 0;
    const TickType_t ticks_per_second = pdMS_TO_TICKS(1000); // 1秒的tick数

    for (;;) {
        // 获取当前时间
        bh1750.read();
        uint16_t val = bh1750.get();
        hmi.printf("t7.txt", "当前光照强度为：%d LX", val);
        RTC_Time current_time = rtc_get_time();
        
        // 更新时间历史数组
        for (int i = TIME_HISTORY_SIZE - 1; i > 0; i--) {
            time_history[i] = time_history[i - 1];
        }
        time_history[0] = current_time;
        
        // 发送光照数据到曲线
        hmi.send_to_curve("s0", 0, val, 0, 600);
        
        // 获取当前tick计数
        TickType_t current_ticks = xTaskGetTickCount();
        
        // 检查是否过了一秒
        if ((current_ticks - last_update_ticks) >= ticks_per_second) {
            // 显示时间到 HMI
            for (int i = 0; i < TIME_HISTORY_SIZE; i++) {
                hmi.printf(txts[i], "%02d:%02d", time_history[i].minute, time_history[i].second);
            }
            last_update_ticks = current_ticks; // 更新最后刷新时间
        }
        
        // FreeRTOS 延时
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms循环频率
    }
}

static void task11_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//光照曲线任务 END

//矩阵键盘任务 START
void keyboard_callback(uint8_t key_num) {
	const char map[16] = {
		'1', '2', '3', 'N',
		'4', '5', '6', 'N',
		'7', '8', '9', 'N',
		'#', '0', '*', 'N'
	};
	char c = map[key_num - 1];
	hmi.printf("t0.txt", "按键：%c", c);
	if (c == '1') {
		motor.set_speed(100);
		rgb_led.set_color(COLOR_RED);
	} else if (c == '2') {
		motor.set_speed(0);
		rgb_led.set_color(COLOR_BLUE);
	} else if (c == '3') {
		motor.set_speed(-100);
		rgb_led.set_color(COLOR_GREEN);
	} 
}

static void TASK12_Thread(void *arg) {
	keyboard.init();
	motor.init();
	rgb_led.init();
	keyboard.ch455g_callback = keyboard_callback;
	for ( ; ; ) {
		keyboard.read();
		delay_1ms(10);
	}
}

static void task12_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//矩阵键盘任务 END

//舵机任务 START
static int16_t angle = 0;
//舵机按键回调
static void key_callback(uint8_t key_num) {
	switch (key_num) {
		case 0x01:
			if (angle < 90) angle += 10;
			break;
		case 0x02:
			if (angle > -90) angle -= 10;
			break;
		case 0x04:
			angle = 0;
			break;
		case 0x08:
			angle = -90;
			break;
	}
	servo.set_angle(angle);
}

static void TASK13_Thread(void *arg) {
	key.init();
	key.click_cb = key_callback;
	servo.init();
	servo.set_angle(angle);
	for ( ; ; ) {
		key.read_all();
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "当前舵机角度为：%d 度", angle);
		hmi.send_string("t0.txt", buffer);
		delay_1ms(10);
	}
}

static void task13_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//舵机任务 END

//点阵屏幕任务 START
static void TASK14_Thread(void *arg) {
	matrix.init();
	matrix.display_countdown_99();
}

static void task14_releases(void) {
	matrix.clear();
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//点阵屏任务 END


//步进电机任务 START
static uint8_t step_motor_direction = 0;

static void task15_corotation(void *arg) {
	step_motor_direction = 1;
}
static void task15_reversal(void *arg) {
	step_motor_direction = 2;
}
static void TASK15_Thread(void *arg) {
	step_motor.init();
	for ( ; ; ) {
		switch (step_motor_direction) {
			case 0:
				break;
			case 1:
				step_motor.corotation(1);
				break;
			case 2:
				step_motor.reverse(1);
				break;
		}
		delay_1ms(10);
	}
}

static void task15_releases(void) {
	step_motor_direction = 0;
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//步进电机任务 END

//姿态传感器 MPU6050 START
static void TASK16_Thread(void *arg) {
	mpu6050.init();
	mpu_data mpuData;
	for ( ; ; ) {
		mpu6050.get_data(&mpuData);
		short temp = mpu6050.get_temperature();
		hmi.printf("t5.txt", "%.2lf", mpuData.angle_x / 10.0f);
		hmi.printf("t6.txt", "%.2lf", mpuData.angle_y / 10.0f);
		hmi.printf("t7.txt", "%.2lf", mpuData.angle_z / 10.0f);
		hmi.printf("t8.txt", "%.2lf", temp / 10.0f);
		delay_1ms(10);
	}
}

static void task16_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}

//姿态传感器 MPU6050 END
static uint16_t fan_speed = 0;

//风扇传感器 START
static void task17_key_callback(uint8_t key_num) {
	switch (key_num) {
		case 0x01:
			if (fan_speed < 1000) fan_speed += 100;
			break;
		case 0x02:
			if (fan_speed > 0) fan_speed -= 100;
			break;
		case 0x04:
			fan_speed = 0;
			break;
		case 0x08:
			fan_speed = 1000;
			break;
	}
	fan.set_speed(fan_speed);
}

static void TASK17_Thread(void *arg) {
	fan.init();
	key.init();
	key.click_cb = task17_key_callback;
	fan.set_speed(fan_speed);
	for ( ; ; ) {
		key.read_all();
		hmi.printf("t0.txt", "风扇转速：%d", fan_speed);
		delay_1ms(20);
	}
}

static void task17_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//风扇传感器 END

//酒精 ADC START
static void TASK18_Thread(void *arg) {
	infrared_distance.init();
	for ( ; ; ) {
		hmi.printf("t0.txt", "酒精值为：%.2lf", infrared_distance.get_distance());
		delay_1ms(500);
	}
}

static void task18_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
//酒精 ADC END

//屏幕任务 START

static void TASK19_Thread(void *arg) {
	hx711.init();
	beep.init();
	for ( ; ; ) {
		long val = hx711.get_weight();
		hmi.printf("t12.txt", "%dg", val);
		if (val > 50) {
			hmi.printf("t13.txt", "禁止通行");
			beep.control(1);
			rgb_led.set_color(COLOR_RED);
		} else {
			hmi.printf("t13.txt", "可以通行");
			beep.control(0);
			rgb_led.set_color(COLOR_OFF);
		}
		delay_1ms(10);
	}
}

static void task19_releases(void) {
	motor.set_speed(0);
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}

//屏幕任务 END

//按键任务 START

uint8_t flag = 0;
static void TASK20_Thread(void *arg) {
	rgb_led.init();
	bh1750.init();
	for ( ; ; ) {
		bh1750.read();
		uint16_t val = bh1750.get();
		if (val < 100) {
			if (flag == 1) continue;
			for (int i = 0; i <= 100; i++) {
				rgb_led.set_pwm_timer(COLOR_PURPLE, i, LED_DEFAULT_FREQ);
				delay_1ms(20);
			}
			flag = 1;
		}
		else {
			if (flag == 0) continue;
			for (int i = 100; i >= 0; i--) {
				rgb_led.set_pwm_timer(COLOR_PURPLE, i, LED_DEFAULT_FREQ);
				delay_1ms(20);
			}
			flag = 0;
		} 
		delay_1ms(20);
	}
}

static void task20_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}

//按键任务END

/* 模板START
static void TASKxx_Thread(void *arg) {
	for ( ; ; ) {
		delay_1ms(1000);
	}
}

static void taskxx_releases(void) {
	for (int i = 0; i < 2; i++) hmi.send_string("t0.txt", "任务已停止");
}
模板 END */


// 任务表 (Task Table)
// 该表用于存储所有任务的信息，包括任务函数、释放函数、任务名称、优先级、任务句柄以及启动和停止命令。
// 通过遍历该表，可以根据接收到的命令启动或停止相应的任务。
// 参数一 task_func 要轮询执行的任务
// 参数二 release_func 任务释放资源的函数
// 参数三 task name 任务名称
// 参数四 priority 任务优先级默认 2
// 参数五 handler 任务句柄
// 参数六 start_cmd 任务启动命令
// 参数七 stop_cmd 任务停止命令
// 参数八 actions 函数指针数组用来存放当前任务的各种各样的动作和行为
// 参数九 actions_cmds 对应函数指针数组的命令
TaskInfo taskTables[30] = {
    {TASK1_Thread, task1_releases, "TASK1", 2, &taskHandlers[0], 0x03, 0x05, {NULL}, {0}}, // 数码管任务
    {TASK2_Thread, task2_releases, "TASK2", 2, &taskHandlers[1], 0x01, 0x02, {NULL}, {0}}, // LED 任务
    {TASK3_Thread, task3_releases, "TASK3", 2, &taskHandlers[2], 0x06, 0x07, {NULL}, {0}}, // 语音任务
    {TASK4_Thread, task4_releases, "TASK4", 2, &taskHandlers[3], 0x08, 0x09, {NULL}, {0}}, // 超声波任务
    {TASK5_Thread, task5_releases, "TASK5", 2, &taskHandlers[4], 0x0A, 0x0B, {NULL}, {0}}, // DHT11 任务
    {TASK6_Thread, task6_releases, "TASK6", 2, &taskHandlers[5], 0x0C, 0x0D, {NULL}, {0}}, // HX711 任务
    {TASK7_Thread, task7_releases, "TASK7", 2, &taskHandlers[6], 0x0E, 0x0F, {NULL}, {0}}, // 滑块LED任务
    {TASK8_Thread, task8_releases, "TASK8", 2, &taskHandlers[7], 0x10, 0x13, {task8_read_card, task8_write_card}, {0x11, 0x12}}, // 读卡任务
		{TASK9_Thread, task9_releases, "TASK9", 2, &taskHandlers[8], 0x14, 0x18, {task9_enroll, task9_identify, task9_delete}, {0x15, 0x16, 0x17}}, // 指纹任务
		{TASK10_Thread, task10_releases, "TASK10", 2, &taskHandlers[9], 0x19, 0x1A, {NULL}, {0}}, // 蜂鸣器任务
		{TASK11_Thread, task11_releases, "TASK11", 2, &taskHandlers[10], 0x1B, 0x1C, {NULL}, {0}}, // 光照曲线任务
		{TASK12_Thread, task12_releases, "TASK12", 2, &taskHandlers[11], 0x1D, 0x1E, {NULL}, {0}}, // 矩阵键盘曲线任务
		{TASK13_Thread, task13_releases, "TASK13", 2, &taskHandlers[12], 0x1F, 0x20, {NULL}, {0}}, // 舵机任务
		{TASK14_Thread, task14_releases, "TASK14", 2, &taskHandlers[13], 0x21, 0x22, {NULL}, {0}}, // 点阵屏任务
		{TASK15_Thread, task15_releases, "TASK15", 2, &taskHandlers[14], 0x23, 0x24, {task15_corotation, task15_reversal}, {0x25, 0x26}}, // 步进电机任务
		{TASK16_Thread, task16_releases, "TASK16", 2, &taskHandlers[15], 0x27, 0x28, {NULL}, {0}}, // 姿态传感器任务
		{TASK17_Thread, task17_releases, "TASK17", 2, &taskHandlers[16], 0x29, 0x2A, {NULL}, {0}}, // 点阵屏任务
		{TASK18_Thread, task18_releases, "TASK18", 2, &taskHandlers[17], 0x2B, 0x2C, {NULL}, {0}}, // 酒精任务
		{TASK19_Thread, task19_releases, "TASK19", 2, &taskHandlers[18], 0x2D, 0x2E, {NULL}, {0}}, // 屏幕任务
		{TASK20_Thread, task20_releases, "TASK20", 2, &taskHandlers[19], 0x2F, 0x30, {NULL}, {0}}, // 按键任务
};



//	按键回调 button_callback
//  串口屏命令来临时 会通过遍历 taskTables执行响应的任务
void button_callback(uint8_t data) {
    for (int i = 0; i < sizeof(taskTables) / sizeof(TaskInfo); i++) {
        // 检查是否是任务启动或停止命令
        if (taskTables[i].task_start_cmd == data) {
            start_task(taskTables[i].task_func, taskTables[i].task_name, 
                       taskTables[i].priority, taskTables[i].handler);
            return;
        } else if (taskTables[i].task_stop_cmd == data) {
            stop_task(taskTables[i].handler, taskTables[i].task_name, 
                      taskTables[i].release_func);
            return;
        }

        // 遍历 actions_cmds，找到匹配的 data
        for (int j = 0; j < MAX_TASK_ACTION_SIZE; j++) {
            if (taskTables[i].actions_cmds[j] == data && taskTables[i].actions[j] != NULL) {
                taskTables[i].actions[j](NULL); // 调用对应的函数
                return;
            }
        }
    }
}
