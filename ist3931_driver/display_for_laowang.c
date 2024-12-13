
#include <string.h>
#include "display_for_laowang.h"

#define HEIGHT_PIX 32
#define WIDTH_PIX 64

//屏幕缓存区
static uint8_t screen_buf[HEIGHT_PIX][WIDTH_PIX/8] = {{0}};

//配置
static const struct ist3931_config config = {
    .type = LAOWANG,
    .vc = 1,        // 电压转换电路使能
    .vf = 1,        // 电压跟随电路使能
    .bias = 2,         // 偏置比
    .ct = 200,         // 对比度设置
    .duty = 32,        // 扫描占空比
    .fr = 60,          // 帧频分频比
    .shl = 1,          // 从 COM1->COMN
    .adc = 0,          // 从 SEG1->SEG132(注：如果要反转该值，需要加偏移)
    .eon = 0,          // 正常显示
    .rev = 0,          // RAM 中 1 映射到 LCD 点亮
    .x_offset = 0,     // 水平偏移
    .y_offset = 0,     // 垂直偏移
    .i2c_write = zxc_i2c_write_only, //该函数用户提供
    .delay = zxc_delay_ms   //该函数用户提供
};

/// @brief 初始化
/// @return 
uint8_t display_for_laowang_init(){
    
    if(ist3931_init(&config)){
        return 1;
    }
    clear_screen(0);
    return 0;
}


/// @brief 清屏
/// @param config 
/// @param val 0或1，填充该值
void clear_screen(uint8_t val){

    val = (val == 0) ? 0:0xff;
    
    memset(screen_buf,val,HEIGHT_PIX * (WIDTH_PIX/8));
    screen_write_by_pix(0,0,WIDTH_PIX,HEIGHT_PIX,screen_buf);
}

/// @brief 指定像素位置和宽度写入
/// @param config 
/// @param x 水平起始坐标
/// @param y 高度起始坐标
/// @param width 宽度
/// @param height 高度
/// @param buf 数据
/// @return 
uint8_t screen_write_by_pix(const uint8_t x, const uint8_t y,
                            uint8_t width, uint8_t height, const void *buf) {
    if ((x + width) > WIDTH_PIX || (y + height) > HEIGHT_PIX) {
        return 1; // 超出范围
    }

    uint8_t *buf_pointer = (uint8_t *)buf;
    uint8_t x_start = x / 8;      // x的字节坐标
    uint8_t x_bis = x % 8;        // x的位偏移
    uint8_t x_end = (x + width - 1) / 8;

    for (uint8_t i = 0; i < height; i++) {
        for (uint8_t j = x_start; j <= x_end; j++) {
            uint8_t before_b = 0;
            uint8_t current_b = 0;

            if (j == x_start) {
                // 起始字节处理：保留已有数据的高位部分
                before_b = (screen_buf[i + y][j] & ~(0xFF >> x_bis));
                current_b = ((*buf_pointer) >> x_bis); // 提取当前字节的高位到低位
                buf_pointer+=1;
            } else {
                
                // 中间字节：处理跨字节的数据
                before_b = (*(buf_pointer-1) << (8 - x_bis)); // 提取上一个字节的低位到高位

                current_b = (j == x_end && x_bis != 0) ? (screen_buf[i + y][j]  & (0xff>>x_bis)) : \
                (current_b = ((*buf_pointer++) >> x_bis));
                
            }

            screen_buf[i + y][j] = (before_b | current_b);
            
        }

        // 映射 Y 坐标到硬件地址
        uint8_t ay_true = i + y;
        ay_true = (ay_true % 2 == 0) ? (ay_true / 2) : ((ay_true - 1) / 2 + 16);

        // 更新硬件
        ist3931_driver_set_ay(&config, ay_true);
        ist3931_driver_set_ax(&config, x_start);
        ist3931_write_bus(&config,&screen_buf[i + y][x_start], 0, x_end - x_start + 1);
    }

    return 0;
}

