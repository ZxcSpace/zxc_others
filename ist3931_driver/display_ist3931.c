
#include "display_ist3931.h"
#include "zxc_i2c.h"
#include "zxc_config.h"
#include "zxc_uart.h"
#define 	IST3931_ADDR 0x7E


uint8_t ist3931_write_bus(const struct ist3931_config* config, uint8_t *buf, bool command,
			     uint16_t num_bytes)
{
	uint8_t control_byte = command ? IST3931_CMD_BYTE : IST3931_DATA_BYTE;
	uint8_t i2c_write_buf[IST3931_RAM_WIDTH * 2];

	for (uint16_t i = 0; i < num_bytes; i++) {
		i2c_write_buf[i * 2] = control_byte;
		i2c_write_buf[i * 2 + 1] = buf[i];
	};
	return config->i2c_write(IST3931_ADDR,i2c_write_buf,num_bytes * 2);

}


//参考电压设置
static uint8_t ist3931_set_power(const struct ist3931_config* config)
{
	uint8_t cmd_buf = IST3931_CMD_POWER_CONTROL | config->vc | config->vf << 1;

	return ist3931_write_bus(config,&cmd_buf, 1, 1);

}

//电压偏置
static inline uint8_t ist3931_set_bias(const struct ist3931_config* config)
{
	uint8_t cmd_buf = IST3931_CMD_BIAS | config->bias;

	return ist3931_write_bus(config,&cmd_buf, 1, 1);
}

//参考电压
static inline uint8_t ist3931_set_ct(const struct ist3931_config* config)
{
	uint8_t cmd_buf[2] = {IST3931_CMD_CT, config->ct};

	return ist3931_write_bus(config,cmd_buf, 1, 2);
}

static inline uint8_t ist3931_set_fr(const struct ist3931_config* config)
{
	uint8_t cmd_buf[3] = {IST3931_CMD_FRAME_CONTROL, config->fr & 0x00FF, config->fr >> 8};

	return ist3931_write_bus(config,cmd_buf, 1, 3);
}

//设置占空比
static uint8_t ist3931_set_duty(const struct ist3931_config* config)
{
	uint8_t cmd_buf[2] = {(IST3931_CMD_SET_DUTY_LSB | (config->duty & 0x0F)),
			      (IST3931_CMD_SET_DUTY_MSB | (config->duty >> 4))};

	return ist3931_write_bus(config,cmd_buf, 1, 2);
}

static uint8_t ist3931_driver_display_control(const struct ist3931_config* config)
{
	uint8_t cmd_buf = IST3931_CMD_DRIVER_DISPLAY_CONTROL | config->shl << 3 | config->adc << 2 |
			  config->eon << 1 | config->rev;

	return ist3931_write_bus(config,&cmd_buf, 1, 1);
}

static uint8_t ist3931_driver_set_display_on(const struct ist3931_config* config)
{
	uint8_t cmd_buf = IST3931_CMD_DISPLAY_ON_OFF | 1;

	return ist3931_write_bus(config,&cmd_buf, 1, 1);
}

static uint8_t ist3931_driver_set_com_pad_map(const struct ist3931_config* config)
{
	uint8_t cmd_buf[5] = {
		IST3931_CMD_IST_COMMAND_ENTRY, IST3931_CMD_IST_COMMAND_ENTRY,
		IST3931_CMD_IST_COMMAND_ENTRY, IST3931_CMD_IST_COMMAND_ENTRY,
		IST3931_CMD_IST_COM_MAPPING | 1,
	};
	uint8_t cmd_buf2 = IST3931_CMD_EXIT_ENTRY;

	ist3931_write_bus(config,&cmd_buf[0], 1, 5);
	config->delay(10);
	ist3931_write_bus(config,&cmd_buf2, 1, 1);
	return 0;
}

uint8_t ist3931_driver_set_ay(const struct ist3931_config* config, uint8_t y)
{
	uint8_t cmd_buf1 = IST3931_CMD_SET_AY_ADD_LSB | ((config->y_offset + y) & 0x0F);
	uint8_t cmd_buf2 = IST3931_CMD_SET_AY_ADD_MSB | ((config->y_offset + y) >> 4);
	uint8_t cmd_buf[2] = {cmd_buf1, cmd_buf2};

	return ist3931_write_bus(config,&cmd_buf[0], 1, 2);
}

uint8_t ist3931_driver_set_ax(const struct ist3931_config* config, uint8_t x)
{
	uint8_t cmd_buf = IST3931_CMD_SET_AX_ADD | (config->x_offset + x);

	return ist3931_write_bus(config,&cmd_buf, 1, 1);
}

uint8_t ist3931_init(const struct ist3931_config* config)
{

	if (ist3931_driver_set_com_pad_map(config))
	{
		return 1;
	}
	config->delay(20);
	if (ist3931_set_duty(config))
	{
		return 1;
	}
	config->delay(20);
	if (ist3931_set_power(config))
	{
		return 1;
	}
	if (ist3931_set_bias(config))
	{
		return 1;
	}
	if (ist3931_set_ct(config))
	{
		return 1;
	}
	if (ist3931_set_fr(config))
	{
		return 1;
	}
	if (ist3931_driver_display_control(config))
	{
		return 1;
	}
	if (ist3931_driver_set_display_on(config))
	{
		return 1;
	}
	config->delay(10);
	
	return 0;
}

/// @brief 向指定坐标写入字节，注意行坐标为字节，+1等于偏移了一个字节
/// @param config 
/// @param x 起始x坐标，以字节为单位
/// @param y 
/// @param width 此处的width是以字节为单位的
/// @param height 
/// @param buf 
/// @return 
uint8_t ist3931_write_by_byte(const struct ist3931_config* config, const uint8_t x, const uint8_t y,
			 uint8_t width,uint8_t height, const void *buf)
{
	uint8_t *data_start = (uint8_t *)buf;
	uint8_t width_tmp = width;
	if (x + width > IST3931_RAM_WIDTH)
	{
		width_tmp = IST3931_RAM_WIDTH - x;
	}
	
	if (y + height > IST3931_RAM_HEIGHT)
	{
		height = IST3931_RAM_HEIGHT - y;
		return 1;
	}
	if (ist3931_driver_set_ay(config, 0)) {
		return 1;
	}
	if (ist3931_driver_set_ax(config, 0)) {
		return 1;
	}

	for (uint8_t i = 0; i < height; i++) {
		ist3931_driver_set_ay(config, i+y);
		ist3931_driver_set_ax(config, x);
		ist3931_write_bus(config,data_start, 0, width_tmp);
		data_start += width;
	}
	return 0;
}

/// @brief 
/// @param config 
/// @param x 起始x坐标，以字节为单位
/// @param y 
/// @param width 此处的width是以字节为单位的
/// @param height 
/// @param buf 
/// @return 
uint8_t screen_adapt_write_byte(const struct ist3931_config* config, const uint8_t x, const uint8_t y,
			 uint8_t width,uint8_t height, const void *buf){
	uint8_t *data_start = (uint8_t *)buf;

	for (uint8_t i = 0; i < height; i++) {
		uint8_t ay_true = i + y;
		//此处为了适配隔壁老王买的5毛屏幕，地址居然是隔行的
		if (config->type == LAOWANG)
		{
			ay_true = (ay_true % 2 == 0) ? (ay_true/2) : ((ay_true-1)/2+16);
		}
		
		ist3931_driver_set_ay(config, ay_true);
		ist3931_driver_set_ax(config, x);
		ist3931_write_bus(config,data_start, 0, width);
		data_start += width;
	}

	return 0;
	
}






