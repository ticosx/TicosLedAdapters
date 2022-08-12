#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TkLedWs2812.h"
#include "led_strip_ws2812.h"
#include "ColorPattern.h"
#include "driver/rmt.h"
#include "Log.h"

TkLedWs2812::TkLedWs2812(led_info_t *led_info) : LedAdapter(led_info) {

}

bool TkLedWs2812::ledTaskCreate(uint32_t taskSize,int taskPriority )
{
    if(LedTaskHandle == NULL)
    {
         if(taskSize < 2048) taskSize = 2048; //至少任务的盏大小不能小于2K
         if(taskPriority < 3) taskPriority = 3;//默认优先等级为3
         if(xTaskCreate(TkLedWs2812::Led_Task, "Led_Task", taskSize , this, taskPriority, &LedTaskHandle)==true)
         {
            logInfo("creat led task ok ,LedTaskHandle:%d",LedTaskHandle);
           
            return true;
         }
         logInfo("creat led task failed ,LedTaskHandle:%d",LedTaskHandle);
    } 
    return false ;
}
bool TkLedWs2812::ledTaskDelete()
{
    if(LedTaskHandle != NULL)
    {
        vTaskDelete(LedTaskHandle);
        logInfo("delete led task ,LedTaskHandle:%d",LedTaskHandle);
        LedTaskHandle = NULL;
        LedClear();
        return true ;

    }
    return false;
}
bool TkLedWs2812::begin(){

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(tx_pin, tx_chn);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    strip_config = LED_STRIP_DEFAULT_CONFIG(nums, (led_ws2812_strip_dev_t)config.channel);
    strip = led_strip_new_ws2812(&strip_config);
    if (!strip) {
        logInfo( "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
   // xTaskCreate(TkLedWs2812::Led_Task, "Led_Task", 4096 , this, 4, &LedTaskHandle);
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
}

void TkLedWs2812::sendRefresh(void) {
    // Neopixel wants colors in green then red then blue order
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
  sendRefresh();
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
  sendRefresh();
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
      cur = cur->nextLooped(first);
      logInfo("led set,i:%d,r:%d,g:%d,b:%d\n",i,color.r,color.g,color.b);
    }
    sendRefresh();
    sei();
    show();
}

/**
 * HSV(Hue, Saturation, Value) 模型中，颜色的参数分别是：色调(H)，饱和度(S)，明度(V)。它更类似于人类感觉颜色的方式，
 * 颜色？深浅？明暗？通过这种方式可以很直观的改变渲染效果。
 * @brief 将HSV颜色空间转换为RGB颜色空间
 *      - 因为HSV使用起来更加直观、方便，所以代码逻辑部分使用HSV。但WS2812B RGB-LED灯珠的驱动使用的是RGB，所以需要转换。
 * 
 * @param  h HSV颜色空间的H：色调。单位°，范围0~360。（Hue 调整颜色，0°-红色，120°-绿色，240°-蓝色，以此类推）
 * @param  s HSV颜色空间的S：饱和度。单位%，范围0~100。（Saturation 饱和度高，颜色深而艳；饱和度低，颜色浅而发白）
 * @param  v HSV颜色空间的V：明度。单位%，范围0~100。（Value 控制明暗，明度越高亮度越亮，越低亮度越低）
 * @param  r RGB-R值的指针
 * @param  g RGB-G值的指针
 * @param  b RGB-B值的指针
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 * https://blog.csdn.net/Mark_md/article/details/115132435
 * https://blog.csdn.net/qq_51985653/article/details/113392665
 */

void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}
//
uint8_t TkLedWs2812::setLedMode(uint8_t mode)
{
   LedMode = mode;
   return 1;
}
//
uint8_t TkLedWs2812::setLedColor(TColor color)
{
   SetClolor = color;
   return 1;
}
//
uint8_t TkLedWs2812::setLedColorLight(uint8_t bright)
{
   LedBright = bright;
   return 1;
}
//
uint8_t TkLedWs2812::getLedMode(void)
{
     return LedMode;
}
//
TColor TkLedWs2812::getLedColor(void)
{
   return SetClolor;
}
//
uint8_t TkLedWs2812::getLedColorLight(void)
{
    return LedBright;
}
void TkLedWs2812::setLedConfig(uint8_t mode,uint8_t bright,TColor color)
{
   setLedMode(mode);
   setLedColorLight(bright);
   setLedColor(color);
  // reset_led_time();
}
//清屏
void TkLedWs2812::LedClear(void)
{      
        strip->clear(strip,50);
        vTaskDelay(pdMS_TO_TICKS(1));
}
//单色
void TkLedWs2812::LedOneColor(TColor color)
{
    for (uint32_t j = 0; j < _length; j ++) {
        strip->set_pixel(strip, j, color.r, color.g, color.b);
    }
    strip->refresh(strip, 100);
    vTaskDelay(pdMS_TO_TICKS(10));
    //logDebug("LedOneColor,red:%d,green:%d,blue:%",red,green,blue);
}

//三色循环
void TkLedWs2812::LedThreeColor(TColor color)
{
    for (uint32_t j = 0; j < _length; j ++) {
        if(j<2) 
        strip->set_pixel(strip, j, color.r, 0, 0);
        else if(j<4)
        strip->set_pixel(strip, j, 0, color.g, 0);
        else if(j<6)
            strip->set_pixel(strip, j, 0, 0, color.b);
    }
    strip->refresh(strip, 100);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// @description: 一个一个逐渐亮起 * @param {type} * @return: 
void TkLedWs2812::LedOneByOne(TColor color,uint32_t time)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;
    for (uint32_t j = 0; j < _length; j++){
        // Build RGB values
        hue = j * 360 / nums + start_rgb;
        led_strip_hsv2rgb(hue, 100, 30, &red, &green, &blue);
        // Write RGB values to strip driver
        strip->set_pixel(strip, j, red, green, blue);
        strip->refresh(strip, 100);
        vTaskDelay(pdMS_TO_TICKS(time));
    }
    strip->clear(strip, 50);
    vTaskDelay(pdMS_TO_TICKS(time));
    
}

//呼吸灯效果
void TkLedWs2812::LedbreathColor(TColor color)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint32_t color_V_MAX = 100;
    uint32_t count_v = 0;  
    bool plus = true ;

    red   = color.r;
    green = color.g;
    blue  = color.b;

    if(plus == true){//吸
        if(count_v < color_V_MAX){
            count_v+=1;
        }
        else {
            plus = false ;
            //呼吸转换的时候加一个停顿，模拟人的呼吸效果
            vTaskDelay(pdMS_TO_TICKS(10*50));
        }
    }
    else{//呼
        if(count_v > 1){
            count_v-=1;
        }
        else{
            plus = true ;
            //呼吸转换的时候加一个停顿，模拟人的呼吸效果
            vTaskDelay(pdMS_TO_TICKS(10*50));
        }
    }
    //调亮度
    if(red>0){
        red = count_v *  2.55f ;
    }
    if(green>0){
        green = count_v *  2.55f ;
    }
    if(blue>0){
        blue = count_v*  2.55f ;
    }
    for (uint32_t j = 0; j < _length; j ++) {
        strip->set_pixel(strip, j,red,green,blue);
    }
    strip->refresh(strip, 100);
    vTaskDelay(pdMS_TO_TICKS(10*3));

}

//彩虹效果
//@description: 彩虹效果 * @param {type} * @return: 
void TkLedWs2812::LedRanbow(uint32_t time,uint8_t V)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;

    for (int i = 0; i < 3; i++){
        for (int j = i; j < nums; j += 3){
            // Build RGB values
            hue = j * 360 / nums + start_rgb;
            led_strip_hsv2rgb(hue, 100, V, &red, &green, &blue);
            // Write RGB values to strip driver
            strip->set_pixel(strip, j, red, green, blue);
        }
        // Flush RGB values to LEDs
        strip->refresh(strip, 100);
        vTaskDelay(pdMS_TO_TICKS(time));
        strip->clear(strip, 50);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    start_rgb += 60;
    // logDebug("rgb test,j:%d",j);
}
// led task
void TkLedWs2812::Led_Task(void *arg) 
{
    TkLedWs2812 *self = (TkLedWs2812*) arg;
    uint8_t ret ;
    uint8_t mode = 0;
    uint8_t bright = 0;
    TColor color ;

	logInfo("Led_Task ok !\n\n");

    while(true)
    {   
        mode =  self->getLedMode();
        color = self->getLedColor();
        bright = self->getLedColorLight();

        switch(mode)
        {
            case SINGLE_COLOR_MODE: //单色
                {
                    self->LedOneColor(color);
                }break;
            case THREE_COLOR_MODE://三色
                {
                    self->LedThreeColor(color);
                }break;
            case ONEBYONE_COLOR_MODE://逐渐亮
                {
                    self->LedOneByOne(color,30);
                }break;
            case BREATH_COLOR_MODE://呼吸灯效果
                {
                    self->LedbreathColor(color);
                }break; 
            case RANBOW_COLOR_MODE://彩虹
                {
                    self->LedRanbow(30,bright);
                }break;

                default : self->LedClear();
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }    
    vTaskDelete(NULL);

}