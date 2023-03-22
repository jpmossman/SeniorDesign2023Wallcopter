#include "cam.hpp"
#include <Arduino.h>
#include "pins.hpp"
#include "client.hpp"
#include "esp_camera.h"

/* ---------------------------------------------------- Globals ---------------------------------------------------- */
/* ----------------------------------------- Public Function Declarations ------------------------------------------ */
int cam::init(void) {
    // Make the camera config
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // If PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if(psramFound()){
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return err;
    }

    sensor_t * s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, -2); // lower the saturation
        Serial.println("PID is OV3660");
    }
    else Serial.println("PID is NOT OV3660");
    // drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

    return 0;
}

void cam::send_jpg_buffer(const char* topic) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;

    static int64_t last_frame = 0;
    if (!last_frame) last_frame = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        res = ESP_FAIL;
        return;
    }
    else {
        if (fb->format != PIXFORMAT_JPEG) {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            esp_camera_fb_return(fb);
            fb = NULL;
            if (!jpeg_converted) {
                Serial.println("JPEG compression failed");
                res = ESP_FAIL;
            }
        }
        else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
        }
    }
    // Get fb
    
    if (fb) {
        Serial.printf("Sending FB\n");
        client::publish(topic, fb->buf, _jpg_buf_len);
        Serial.printf("    JPG sent of length %d\n", _jpg_buf_len);

        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;
    } else if (_jpg_buf) {
        Serial.printf("Sending JPG\n");
        client::publish(topic, fb->buf, _jpg_buf_len);
        Serial.printf("    JPG sent of length %d\n", _jpg_buf_len);

        free(_jpg_buf);
        _jpg_buf = NULL;
    }

    /* There's some frame time stuff here in the template. Seemed unnecessary so I omitted it.*/
    int64_t fr_end = esp_timer_get_time();
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    // Serial.printf("MJPG: %uB %ums (%.1ffps)\n", 
    //   (uint32_t)(_jpg_buf_len),
    //   (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time
    // );
}


/* ----------------------------------------- Private Function Declarations ----------------------------------------- */
/* ------------------------------------------ Public Function Definitions ------------------------------------------ */
/* ----------------------------------------- Private Function Definitions ------------------------------------------ */
