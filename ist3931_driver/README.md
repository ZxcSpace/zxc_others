适配老王5毛屏幕驱动

最简易使用方式：

打开`display_for_laowang.c`，提供`i2c写函数`和`毫秒延时函数`，就如图中的位置：

![image-20241212201856738](.\README.assets\image-20241212201856738.png)

调用初始化函数：

```c
display_for_laowang_init();
```

调用下面的函数进行显示

```c
uint8_t screen_write_by_pix(const uint8_t x, const uint8_t y,uint8_t width, uint8_t height, const void *buf)
```

这仅仅实现了驱动该屏幕的功能，我没有注意过多的细节，你可以根据自己需求修改它。

