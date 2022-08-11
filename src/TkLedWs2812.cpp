#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TkLedWs2812.h"
#include "led_strip_ws2812.h"
#include "ColorPattern.h"
#include "driver/rmt.h"
#include "Log.h"

TkLedWs2812::TkLedWs2812(led_info_t *led_info) : LedAdapter(led_info) {

}

bool TkLedWs2812::begin(){

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(tx_pin, tx_chn);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    strip_config = LED_STRIP_DEFAULT_CONFIG(nums, (led_strip_dev_t)config.channel);
    strip = led_strip_new_ws2812(&strip_config);
    if (!strip) {
        logInfo( "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    //xTaskCreate(TkLedWs2812::Led_Task, "Led_Task", 4096 , this, 4, NULL);
    // Show simple rainbow chasing pattern
    logInfo("install WS2812 driver ok !");
    return true;
}

bool TkLedWs2812::end(){

    return true;
}

bool TkLedWs2812::init() {

    if(led_info->led_nums != 0){
        tx_pin = (gpio_num_t)led_info->tx_pin;
        tx_chn = (rmt_channel_t)led_info->tx_chn;
        nums = led_info->led_nums;
        _length = nums;
        begin();
        logInfo("led init ok ! \n");
         return true;
    }
    else {
         logInfo("led init failed ! \n");
         return false;
    }
   

    //logInfo("\n");

   
}
bool TkLedWs2812::deinit() {
    end();
    logInfo("led deinit \n");
    return true;
}


int8_t getBitPosition(uint8_t b) {
    int8_t pos = -1;
    uint8_t t = b;
    while (t > 0) {
        t >>= 1;
        pos++;
    }
    return pos;
}


uint32_t TkLedWs2812::getLength() {
    return _length;
}


void TkLedWs2812::sendPixel(uint32_t j ,uint8_t r, uint8_t g, uint8_t b) {
    // Neopixel wants colors in green then red then blue order
    strip->set_pixel(strip, j, r, g, b);
    strip->refresh(strip, 100);
}

// Just wait long enough without sending any bots to cause the pixels to latch and display the last sent frame
void TkLedWs2812::show() {
   // _delay_us((RES / 1000UL) + 1); // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}

// Display a single color on the whole strip
void TkLedWs2812::fill(uint8_t r, uint8_t g, uint8_t b) {
    cli();
    for (uint32_t i = 0; i < _length; i++) {
        sendPixel(i,r, g, b);
    }
    sei();
    show();
}

void TkLedWs2812::fill(TColor color) {
  cli();
  for (uint32_t i = 0; i < _length; i++) {
      sendPixel(i,color.r, color.g, color.b);
  }
  sei();
  show();
}

void TkLedWs2812::fill(TColor color, uint32_t pixelCount, bool fromTail) {
  cli();
  // If fromTail is true
  // Skip pixels by filling them with black color
  uint32_t pixelsToSkip = fromTail ? _length - pixelCount : 0;
  for (uint32_t i = 0; i < pixelsToSkip && i < _length; i++) {
      sendPixel(i,0, 0, 0);
  }
  // Fill pixels
  for (uint32_t i = pixelsToSkip; i < (pixelsToSkip + pixelCount); i++) {
      sendPixel(i,color.r, color.g, color.b);
  }
  sei();
  show();
}

void TkLedWs2812::fillPattern(LedPattern* pat, uint32_t shift) {

    logInfo("led fillPattern,shift:%d\n",shift);
    LedPatternNode* first = pat->first();
    LedPatternNode* cur = first;
    uint32_t _shift = (uint32_t) shift % _length;
    for (uint32_t s = 0; s < _shift; s++) {
      cur = cur->nextLooped(first);
    }
    cli();
    for (uint32_t i = 0; i < _length; i++) {
      TColor color = cur->color();
      sendPixel(i,color.r, color.g, color.b);
      //sendPixel(i,100, 0, 30);
      cur = cur->nextLooped(first);
      logInfo("led set,i:%d,r:%d,g:%d,b:%d\n",i,color.r,color.g,color.b);
    }
    sei();
    show();
}
// led task
void TkLedWs2812::Led_Task(void *arg) 
{
    while(1){
        vTaskDelay(pdMS_TO_TICKS(20));
    }
     vTaskDelete(NULL);
     #if 0
    uint8_t ret ;
	printf("Led_Task ok !\n\n");
     uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint32_t color = 0;
    uint16_t LED_NUMBER = 0;
    uint32_t  color_v = 0;   

    while(true)
    {
        color = get_led_color();
        Led_mode = get_led_mode();
        color_v = get_led_color_v();

        red   = (color >> 16) & 0x0000ff;
        green = (color >> 8)  & 0x0000ff;
        blue  = (color >> 0)  & 0x0000ff;
        switch(Led_mode)
        {
            case 0: //单色
                {
                    LedOneColor(strip, nums,red,green,blue);
                }break;
            case 1://三色
                {
                        LedThreeColor(strip,LED_NUMBER,color_v,255,255,255);
                }break;
            case 2://逐渐亮
                {
                    LedOneByOne(strip,LED_NUMBER,50,50);
                }break;
            case 3://呼吸灯效果
                {
                    LedbreathColor(strip ,LED_NUMBER,color_v,red,green,blue);  
                }break; 
            case 4://彩虹
                {
                    LedRanbow(strip,LED_NUMBER,color_v,500);
                }break;
                default : LedClear(strip);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    vTaskDelete(NULL);
    #endif
}