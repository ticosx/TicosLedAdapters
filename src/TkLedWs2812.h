/* 
Copyright (c) 2022 TIWATER. All Rights Reserved.
*/
#ifndef _LEDWS2182_H_
#define _LEDWS2182_H_

#include <Arduino.h>
#include <ColorPattern.h>
#include "led_strip_ws2812.h"
#include "LedAdapter.h"
#include "driver/rmt.h"
#include "HardwareAdapter.h"

class TkLedWs2812 : public LedAdapter  {
public:
    TkLedWs2812(led_info_t *led_info);
    virtual ~TkLedWs2812(){};
    virtual void fill(uint8_t r, uint8_t g, uint8_t b) override;
    
    virtual void fill(TColor color, uint32_t pixelCount, bool fromTail) override;
    virtual void fillPattern(LedPattern* pat, uint32_t shift) override;
    virtual void sendPixel(uint32_t j,uint8_t r, uint8_t g, uint8_t b) override;
    virtual void show() override;
    virtual uint32_t getLength()override;
    virtual bool init() override; 
    virtual bool deinit() override; 


private:
    bool begin();
    bool end() ;
    void fill(TColor color);
    static void Led_Task(void *arg) ;
    uint32_t _length;
    gpio_num_t    tx_pin = GPIO_NUM_10;
    rmt_channel_t tx_chn = RMT_CHANNEL_0;
    uint32_t nums = 6;
    led_ws2812_strip_t *strip;
    led_ws2812_strip_config_t strip_config;

};
#endif

