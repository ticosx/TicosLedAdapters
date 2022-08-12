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
    ONEBYONE_COLOR_MODE,
    BREATH_COLOR_MODE,
    RANBOW_COLOR_MODE,
    MAX_MODE
} ;

class TkLedWs2812 : public LedAdapter  {
public:
    TkLedWs2812(led_info_t *led_info);
    virtual ~TkLedWs2812(){};
    virtual void fill(TColor color) override;
    virtual void fill(uint8_t r, uint8_t g, uint8_t b) override;
    virtual void fill(TColor color, uint32_t pixelCount, bool fromTail) override;
    virtual void fillPattern(LedPattern* pat, uint32_t shift) override;
    //virtual void sendPixel(uint32_t j,uint8_t r, uint8_t g, uint8_t b) override;
    virtual void setLedConfig(uint8_t mode,uint8_t bright,TColor color) override;
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
    void LedbreathColor(TColor color);
    void LedOneByOne(TColor color,uint32_t time);
    void LedThreeColor(TColor color);
    void LedOneColor(TColor color);
    void LedClear(void);
    uint8_t setLedMode(uint8_t mode);
    uint8_t setLedColor(TColor color);
    uint8_t setLedColorLight(uint8_t bright);
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
    TColor SetClolor ;
    TaskHandle_t LedTaskHandle = NULL;



};
#endif

