
#ifndef __DISPLAY_IST3931_H__
#define __DISPLAY_IST3931_H__

#define IST3931_CMD_NOP                    0xe3
#define IST3931_CMD_IST_COMMAND_ENTRY      0x88
#define IST3931_CMD_EXIT_ENTRY             0xe3
#define IST3931_CMD_IST_COM_MAPPING        0x60
#define IST3931_CMD_POWER_CONTROL          0x2c
#define IST3931_CMD_BIAS                   0x30
#define IST3931_CMD_CT                     0xb1
#define IST3931_CMD_FRAME_CONTROL          0xb2
#define IST3931_CMD_SET_AX_ADD             0xc0
#define IST3931_CMD_SET_AY_ADD_LSB         0x00
#define IST3931_CMD_SET_AY_ADD_MSB         0x10
#define IST3931_CMD_SET_START_LINE_LSB     0x40
#define IST3931_CMD_SET_START_LINE_MSB     0x50
#define IST3931_CMD_OSC_CONTROL            0x2a
#define IST3931_CMD_DRIVER_DISPLAY_CONTROL 0x60
#define IST3931_CMD_SW_RESET               0x76
#define IST3931_CMD_SET_DUTY_LSB           0x90
#define IST3931_CMD_SET_DUTY_MSB           0xa0
#define IST3931_CMD_DISPLAY_ON_OFF         0x3c
#define IST3931_CMD_SLEEP_MODE             0x38

#define IST3931_CMD_BYTE    0x80
#define IST3931_DATA_BYTE   0xc0
#define IST3931_RESET_DELAY 50
#define IST3931_CMD_DELAY   10
#define IST3931_RAM_WIDTH   0x11
#define IST3931_RAM_HEIGHT  0x40

#include "stm8s.h"

typedef enum {
	LAOWANG
}SCREEN_TYPE;

typedef uint8_t (*i2c_write_func)(uint8_t device_addr,uint8_t* data, uint16_t len);
typedef void (*delay_ms)(uint16_t ms);
struct ist3931_config {

	SCREEN_TYPE type;
	bool vc;      // 电压转换电路使能
	bool vf;      // 电压跟随电路使能
	uint8_t bias; // 电压偏置比0-7
	uint8_t ct;   // 对比度设置0-255
	uint8_t duty; // 扫描占空比1-64
	uint16_t fr;  // 帧频分频比
	bool shl;     // 竖直坐标反转0/1
	bool adc;     // 水平坐标反转0/1
	bool eon;     // 强制亮屏0/1
	bool rev;     // 显示反转0/1
	uint8_t x_offset; // x坐标偏移
	uint8_t y_offset;// y坐标偏移
	i2c_write_func i2c_write;
	delay_ms delay;
};


uint8_t ist3931_driver_set_ay(const struct ist3931_config *config, uint8_t y);

uint8_t ist3931_driver_set_ax(const struct ist3931_config *config, uint8_t x);

uint8_t ist3931_init(const struct ist3931_config* config);

uint8_t ist3931_write_by_byte(const struct ist3931_config* config, const uint8_t x, const uint8_t y,
			 uint8_t width,uint8_t height, const void *buf);

uint8_t screen_adapt_write_byte(const struct ist3931_config *config, const uint8_t x, const uint8_t y, uint8_t width, uint8_t height, const void *buf);
uint8_t ist3931_write_bus(const struct ist3931_config *config,uint8_t *buf, bool command, uint16_t num_bytes);
#endif 


