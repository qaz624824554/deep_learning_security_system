#include "Sensor.hpp"
#include <iostream>
#include <stdexcept>
#include <thread>

extern "C" {
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <gpiod.h>
#include <linux/i2c-dev.h>
}

std::atomic<bool> SR501::is_running = false;

void SR501::listen_state(std::function<void(bool state)> fn)
{
    is_running = true;

    int status;
    std::thread(
        [&](std::function<void(bool state)> fn) {
            try {
                while(is_running) {
                    int value = gpiod_ctxless_get_value("gpiochip4", 3, false, "sr501");

                    if(value == -1) {
                        std::cout << "error" << std::endl;
                    }

                    fn(value);

                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                }
            } catch(std::exception & e) {
                std::cout << "SR501---listen_state: " << e.what() << std::endl;
            }
        },
        fn)
        .detach();
}

void SR501::stop_listen_state()
{
    is_running = false;
}

void OLED::_i2c_init()
{
    char filename[32];
    sprintf(filename, "/dev/i2c-%d", SSD1306_I2C_BUS); // I2C bus number passed

    file_i2c = open(filename, O_RDWR);

    if(file_i2c < 0) {
        throw std::runtime_error("_i2c_init error");
    }

    if(ioctl(file_i2c, I2C_SLAVE, SSD1306_I2C_ADDR) < 0) // set slave address
    {
        throw std::runtime_error("_i2c_init error");
    }
}

void OLED::_i2c_write(uint8_t * ptr, int16_t len)
{
    if(write(file_i2c, ptr, len) != len) {
        throw std::runtime_error("_i2c_write error");
    }
}

void OLED::_i2c_read(uint8_t * ptr, int16_t len)
{
    if(read(file_i2c, ptr, len) != len) {
        throw std::runtime_error("_i2c_read error");
    }
}

OLED::OLED()
{
    _i2c_init();

    // test i2c connection
    uint8_t cmd    = SSD1306_COMM_CONTROL_BYTE;
    uint8_t result = 0;
    _i2c_write(&cmd, 1);
    _i2c_read(&result, 1);
    if(result == 0) {
        throw std::runtime_error("OLED i2c connection error");
    }

    uint16_t i     = 0;
    data_buf_[i++] = SSD1306_COMM_CONTROL_BYTE; // command control byte
    data_buf_[i++] = SSD1306_COMM_DISPLAY_OFF;  // display off
    data_buf_[i++] = SSD1306_COMM_DISP_NORM;    // Set Normal Display (default)
    data_buf_[i++] = SSD1306_COMM_CLK_SET;      // SET DISPLAY CLOCK DIV
    data_buf_[i++] = 0x80;                      // the suggested ratio 0x80
    data_buf_[i++] = SSD1306_COMM_MULTIPLEX;    // SSD1306_SET MULTIPLEX
    data_buf_[i++] = max_lines_ - 1;            // height is 32 or 64 (always -1)
    data_buf_[i++] = SSD1306_COMM_VERT_OFFSET;  // SET DISPLAY OFFSET
    data_buf_[i++] = 0;                         // no offset
    data_buf_[i++] = SSD1306_COMM_START_LINE;   // SET START LINE
    data_buf_[i++] = SSD1306_COMM_CHARGE_PUMP;  // CHARGE PUMP
    data_buf_[i++] = 0x14;                      // turn on charge pump
    data_buf_[i++] = SSD1306_COMM_MEMORY_MODE;  // MEMORY MODE
    data_buf_[i++] = SSD1306_PAGE_MODE;         // page mode
    data_buf_[i++] = SSD1306_COMM_HORIZ_NORM;   // SEGREMAP  Mirror screen horizontally (A0)
    data_buf_[i++] = SSD1306_COMM_SCAN_NORM;    // COMSCANDEC Rotate screen vertically (C0)
    data_buf_[i++] = SSD1306_COMM_COM_PIN;      // HARDWARE PIN
    data_buf_[i++] = 0x02;                      // for 32 lines

    data_buf_[i++] = SSD1306_COMM_CONTRAST;       // SETCONTRAST
    data_buf_[i++] = 0x7f;                        // default contract value
    data_buf_[i++] = SSD1306_COMM_PRECHARGE;      // SET PRECHARGE
    data_buf_[i++] = 0xf1;                        // default precharge value
    data_buf_[i++] = SSD1306_COMM_DESELECT_LV;    // SETVCOMDETECT
    data_buf_[i++] = 0x40;                        // default deselect value
    data_buf_[i++] = SSD1306_COMM_RESUME_RAM;     // DISPLAYALLON_RESUME
    data_buf_[i++] = SSD1306_COMM_DISP_NORM;      // NORMAL DISPLAY
    data_buf_[i++] = SSD1306_COMM_DISPLAY_ON;     // DISPLAY ON
    data_buf_[i++] = SSD1306_COMM_DISABLE_SCROLL; // Stop scroll

    _i2c_write(data_buf_, i);

    data_buf_[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf_[1] = SSD1306_COMM_DISP_NORM;
    _i2c_write(data_buf_, 2);
}

void OLED::set_XY(uint8_t x, uint8_t y)
{
    if(x >= max_columns_ || y >= (max_lines_ / 8)) return;

    global_x_ = x;
    global_y_ = y;

    data_buf_[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf_[1] = SSD1306_COMM_PAGE_NUMBER | (y & 0x0f);

    data_buf_[2] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);

    data_buf_[3] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);

    _i2c_write(data_buf_, 4);
}

void OLED::clear_line(uint8_t y)
{
    uint8_t i;
    if(y >= (max_lines_ / 8)) return;

    set_XY(0, y);

    data_buf_[0] = SSD1306_DATA_CONTROL_BYTE;
    for(i = 0; i < max_columns_; i++) data_buf_[i + 1] = 0x00;
    _i2c_write(data_buf_, 1 + max_columns_);
}

void OLED::clean_screen()
{
    uint8_t i;
    for(i = 0; i < (max_lines_ / 8); i++) {
        clear_line(i);
    }
}

void OLED::show(bool is_check)
{
    static OLED oled;

    if(!oled.is_first_show_ && oled.pre_status_ == is_check) return;

    oled.is_first_show_ = false;
    oled.pre_status_    = is_check;
    // clean screen
    oled.clean_screen();

    // set x,y
    oled.set_XY(0, 0);

    // write text
    uint16_t i = 0;

    const uint8_t(*font)[60] = is_check ? success_text : fail_text;

    // 为每一行数据预先分配位置
    for(int row = 0; row < 3; row++) {
        uint16_t start_col = row * 20; // 每行20列
        uint16_t end_col   = (row + 1) * 20;

        // 设置XY位置
        oled.set_XY(0, row);

        // 控制字节
        oled.data_buf_[i++] = SSD1306_DATA_CONTROL_BYTE;

        // 填充数据
        for(int j = 0; j < 6; j++) {
            for(int k = start_col; k < end_col; k++) {
                oled.data_buf_[i++] = font[j][k];
            }
        }

        // 写入数据
        oled._i2c_write(oled.data_buf_, i);
        i = 0; // 重置i以便下一行使用
    }
}
