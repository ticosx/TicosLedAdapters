/* 
Copyright (c) 2022 TIWATER. All Rights Reserved.
*/
#ifndef _LEDWS2182_H_
#define _LEDWS2182_H_

#include <Arduino.h>
#include "freertos/task.h"
#include <ColorPattern.h>
#include "led_strip_ws2812.h"
#include "LedAdapter.h"
#include "driver/rmt.h"
#include "HardwareAdapter.h"

typedef enum {
    SINGLE_COLOR_MODE = 0,  /* */
    THREE_COLOR_MODE,
    ONEBYONE_COLOR_MODE,//
    BREATH_COLOR_MODE,//单色呼吸
    RANBOW_COLOR_MODE,//彩色旋转
    UPS_DOWN_MODE,//单色起伏
    BLUETOOTH_CONNECTING_MODE,//蓝牙连接中
    BLUETOOTH_CONNECTED_MODE,//蓝牙连接成功
    FIREWARE_UPDATE_MODE,//固件升级
    DRINKWATER_REMIN_MODE,//饮水达标提醒
    DRINKWATER_ALARM_MODE,//饮水闹钟
    MAX_MODE
} ;

uint8_t  const onebyonevalue[6][18]={ {0,50,0, 50,15,0, 0,50,0,  50,15,0, 0,50,0, 50,10,0}, //第一轮
                                     { 50,10,0, 0,50,0, 50,15,0, 0,50,0, 50,15,0, 0,50,0}, //第二轮
                                     { 0,50,0, 50,10,0, 0,50,0, 50,10,0, 0,50,0, 50,10,0}, //第三轮
                                     { 50,10,0, 0,50,0, 50,15,0, 0,50,0, 50,15,0, 0,50,0}, //第四轮
                                     { 0,50,0, 50,10,0, 0,50,0, 50,10,0, 0,50,0, 50,10,0}, //第五轮
                                     { 50,10,0, 0,50,0, 50,15,0, 0,50,0, 50,15,0, 0,50,0}  //第六轮 
}; 

uint8_t  const alarmvalue[6][18] = { { 0,50,20, 0,40,16, 0,30,12, 0,20,8,  0,10,4,  5,5,5}, //第一轮
                                     { 0,40,16, 0,30,12, 0,20,8,  0,10,4,  5,5,5,0, 50,20},//第二轮
                                     { 0,30,12, 0,20,8,  0,10,4,  5,5,5,   0,50,20, 0,40,16}, //第三轮
                                     { 0,20,8,  0,10,4,  5,5,5,   0,50,20, 0,40,16, 0,30,12}, //第四轮
                                     { 0,10,4,  5,5,5,   0,50,20, 0,40,16, 0,30,12, 0,20,8}, //第五轮
                                     { 5,5,5,   0,50,20, 0,40,16, 0,30,12, 0,20,8,  0,10,4} //第六轮 
}; 

class TkLedWs2812 : public LedAdapter  {
public:
    TkLedWs2812(led_info_t *led_info);
    virtual ~TkLedWs2812(){};
    virtual void fill(TColor color) override;
    virtual void fill(uint8_t r, uint8_t g, uint8_t b) override;
    virtual void fill(TColor color, uint32_t pixelCount, bool fromTail) override;
    virtual void fillPattern(LedPattern* pat, uint32_t shift) override;
    //virtual void sendPixel(uint32_t j,uint8_t r, uint8_t g, uint8_t b) override;
    virtual void setLedConfig(uint8_t mode,uint8_t bright,uint32_t delaytime,TColor color) override;
    virtual void show() override;
    virtual uint32_t getLength()override;
    virtual bool init() override; 
    virtual bool deinit() override; 
    virtual bool ledTaskCreate(uint32_t taskSize,int taskPriority )override; 
    virtual bool ledTaskDelete()override; 

private:
    bool begin();
    bool end() ;
   // void fill(TColor color);
    void sendPixel(uint32_t j,uint8_t r, uint8_t g, uint8_t b) ;
    void sendRefresh(void);
    void LedRanbow(uint32_t time,uint8_t V);
    void LedUpAndDownColor(TColor color,uint32_t time);
    void LedBreathColor(TColor color,uint32_t time);
    void LedOneByOne(TColor color,uint32_t time);
    void LedThreeColor(TColor color);
    void LedOneColor(TColor color);
    void LedBluetoothConnect(TColor color,uint32_t time,uint8_t state);
    void LedFirewareUpdate(TColor color,uint32_t time);
    void LedDrinkWaterAlarm(uint32_t time);
    void LedClear(void);
    uint8_t setLedMode(uint8_t mode);
    uint8_t setLedColor(TColor color);
    uint8_t setLedColorLight(uint8_t bright);
    uint8_t setLedDelayTime(uint32_t delaytime);
    uint32_t getLedDelayTime(void);
    uint8_t getLedMode(void);
    uint8_t getLedColorLight(void);
    TColor getLedColor(void);
 
    static void Led_Task(void *arg) ;

    uint32_t _length;
    gpio_num_t    tx_pin = GPIO_NUM_10;
    rmt_channel_t tx_chn = RMT_CHANNEL_0;
    uint32_t nums = 6;
    led_ws2812_strip_t *strip;
    led_ws2812_strip_config_t strip_config;
    uint8_t LedMode = 0;
    uint8_t LedBright = 0;
    uint32_t LedDelayTime = 0;
    TColor SetClolor ;
    TaskHandle_t LedTaskHandle = NULL;



};
#endif

