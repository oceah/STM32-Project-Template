#include "app.h"

#include "sys.h"
#include "pin.h"
#include "i2c.h"
#include "serial_port.h"
#include "edge_trigger.h"
#include "scheduled_task.h"
using namespace hal;

#include "D36A.h"

#include "oceah/hachimi.hpp"
using namespace oceah;

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* -+-+-+-+-+- 引脚定义 -+-+-+-+-+- */
Pin LED1(LED1_GPIO_Port, LED1_Pin);
Pin I2C1_SCL(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin);
Pin I2C1_SDA(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin);
Pin I2C2_SCL(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin);
Pin I2C2_SDA(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin);
Pin DAC_CS(DAC_CS_GPIO_Port, DAC_CS_Pin);

PWM D36A_ST1(&htim2, TIM_CHANNEL_1);
Pin D36A_DIR1(D36A_DIR1_GPIO_Port, D36A_DIR1_Pin);
Pin D36A_EN1(D36A_EN1_GPIO_Port, D36A_EN1_Pin);

PWM FAN_PWM(&htim3, TIM_CHANNEL_1);

/* -+-+-+-+-+- 通信协议 -+-+-+-+-+- */
I2C i2c1(I2C1_SCL, I2C1_SDA);
I2C i2c2(I2C2_SCL, I2C2_SDA);

/* -+-+-+-+-+- 外设定义 -+-+-+-+-+- */
SerialPort com1(&huart1);
SerialPort com2(&huart2);

/* -+-+-+-+-+- 硬件定义 -+-+-+-+-+- */
D36A motor({D36A_ST1, D36A_DIR1}, D36A::nullpart, 32);

/* -+-+-+-+-+- 散热控制 -+-+-+-+-+- */
void motor_up()
{
    D36A_EN1 = 1;
    motor.rotate(D36A::Select::Part1, -180.0f);
}

void motor_down()
{
    motor.rotate(D36A::Select::Part1, 180.0f);
    D36A_EN1 = 0;
}

void fan_set(float v)
{
    static bool up = false;
    if (v <= 0.0f) {
        // fan stop
        FAN_PWM.set(0.0f);
        // motor down
        if (up) {
            motor_down();
            up = false;
        }
    } else {
        // fan start
        FAN_PWM.set(v);
        // motor up
        if (!up) {
            motor_up();
            up = true;
        }
    }
}

/* -+-+-+-+-+- 服务定义 -+-+-+-+-+- */
HaChiMi hachimi;

HaChiMi_Service ping_service(
    "ping",
    [](const HaChiMi_Command &cmd) {
        if (cmd.argc != 0) {
            com1.write("ERROR\r\n");
            return;
        }
        com1.write("Ok\r\n");
    });

HaChiMi_Service echo_service(
    "echo",
    [](const HaChiMi_Command &cmd) {
        if (cmd.argc != 1) {
            com1.write("ERROR\r\n");
            return;
        }
        com1.write(cmd.argv[0]);
        com1.write("\r\n");
    });

HaChiMi_Service fan_service(
    "fan",
    [](const HaChiMi_Command &cmd) {
        if (cmd.argc != 1) {
            com1.write("ERROR\r\n");
            return;
        }
        float v = std::strtof(cmd.argv[0], nullptr);
        if (std::isnan(v)) {
            com1.write("ERROR\r\n");
            return;
        }
        com1.write("busy...\r\n");
        fan_set(v);
        com1.write("Ok\r\n");
    });

static void init()
{
    /* -+-+-+-+-+- 硬件初始化 -+-+-+-+-+- */
    FAN_PWM.set(0.0f);
    FAN_PWM.start();
    D36A_DIR1 = 0;
    motor.set_rpm(D36A::Select::Part1, 30);
    motor_down();

    /* -+-+-+-+-+- 注册服务 -+-+-+-+-+- */
    hachimi.register_service(&ping_service);
    hachimi.register_service(&echo_service);
    hachimi.register_service(&fan_service);
}

void App_Start()
{
    init();
    static uint8_t buf[1024];
    while (1) {
        auto n = com1.read(buf, sizeof(buf));
        hachimi.push(buf, n);
    }
}

void App_Error_Handler()
{
    LED1 = 0;
    while (1) {
    }
}
