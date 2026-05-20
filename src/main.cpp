#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <vector>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <time.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <soc/rtc_wdt.h>
#include <math.h>
#include "ui/web_ui.h"

namespace Config {
  constexpr uint8_t  DISPLAY_ROTATION  = 0;
  constexpr uint8_t  FRAME_DELAY_MS    = 100;

  constexpr uint16_t SCREEN_W          = 240;
  constexpr uint16_t SCREEN_H          = 240;
  constexpr uint16_t WALLPAPER_H       = 177;
  constexpr uint16_t CLOCK_BAR_Y       = 177;
  constexpr uint16_t CLOCK_BAR_H       = 63;

  constexpr uint8_t  TTP223_PIN        = 1;
  constexpr uint8_t  DEBOUNCE_TIME_MS  = 15;
  constexpr uint16_t HOLDING_TIME_MS   = 400;

  constexpr uint8_t  TAP_COUNT_TRIGGER = 3;
  constexpr uint16_t TAP_RESET_MS      = 500;

  constexpr uint8_t  SDA_PIN               = 8;
  constexpr uint8_t  SCL_PIN               = 9;
  constexpr uint8_t  MPU6050_I2C_ADDR      = 0x68;
  constexpr float    SHAKE_THRESHOLD       = 1.2;
  constexpr unsigned long SHAKE_DETECTION_TIME = 1000;
  constexpr unsigned long SHAKE_COOLDOWN_TIME  = 1000;

  constexpr char AP_SSID_DEFAULT[]     = "Mochi";
  constexpr char AP_PASS_DEFAULT[]     = "12345678";

  constexpr char WALLPAPER_PATH[]  = "/wallpaper.jpg";
  constexpr char WALLPAPER_TMP[]   = "/wallpaper_tmp.jpg";
  constexpr char AP_CONFIG_PATH[]  = "/ap_config.txt";
  constexpr char TIME_SAVE_PATH[]  = "/time.bin";
  constexpr char GREETING_PATH[]   = "/greeting.txt";
  constexpr char BRIGHTNESS_PATH[] = "/brightness.txt";
  constexpr char THEME_PATH[]      = "/theme.txt";

  constexpr uint8_t  BACKLIGHT_PIN    = 7;
  constexpr uint8_t  BACKLIGHT_CH     = 0;
  constexpr uint32_t BACKLIGHT_FREQ   = 5000;
  constexpr uint8_t  BACKLIGHT_RES    = 8;
  constexpr uint8_t  BRIGHTNESS_DEF   = 200;
}

typedef struct _VideoInfo {
  const uint8_t* const* frames;
  const uint16_t*       frames_size;
  uint16_t              num_frames;
  uint8_t               audio_idx;
} VideoInfo;

#include "assets/full1.h"
#include "assets/chongmat1.h"
#include "assets/xoadau1.h"

// ===== MODE & STATE ENUMS =====
enum class PlayerMode { PLAYING, CLOCK, ANALOG_CLOCK };
enum class ButtonState { IDLE, PRESSED, HELD, RELEASED_SHORT, RELEASED_HOLD };

// ===== LGFX =====
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host    = SPI2_HOST;
      cfg.spi_mode    = 3;
      cfg.freq_write  = 40000000;
      cfg.freq_read   = 20000000;
      cfg.spi_3wire   = true;
      cfg.use_lock    = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk    = 4;
      cfg.pin_mosi    = 6;
      cfg.pin_miso    = -1;
      cfg.pin_dc      = 3;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = -1;
      cfg.pin_rst          = 10;
      cfg.pin_busy         = -1;
      cfg.panel_width      = 240;
      cfg.panel_height     = 240;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = false;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = true;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

// ===== AP CONFIG MANAGER =====
class APConfigManager {
private:
  String _ssid;
  String _password;

public:
  void init() {
    _ssid     = Config::AP_SSID_DEFAULT;
    _password = Config::AP_PASS_DEFAULT;
    load();
  }

  void load() {
    if (!LittleFS.exists(Config::AP_CONFIG_PATH)) return;
    File f = LittleFS.open(Config::AP_CONFIG_PATH, "r");
    if (!f) return;
    String ssid = f.readStringUntil('\n');
    String pass = f.readStringUntil('\n');
    f.close();
    ssid.trim(); pass.trim();
    if (ssid.length() >= 1 && ssid.length() <= 32) _ssid = ssid;
    if (pass.length() >= 8 && pass.length() <= 64)  _password = pass;
    Serial.printf("AP config loaded: SSID=%s\n", _ssid.c_str());
  }

  bool save(const String& ssid, const String& pass) {
    if (ssid.length() < 1 || ssid.length() > 32) return false;
    if (pass.length() < 8 || pass.length() > 64)  return false;
    File f = LittleFS.open(Config::AP_CONFIG_PATH, "w");
    if (!f) return false;
    f.println(ssid);
    f.println(pass);
    f.close();
    _ssid     = ssid;
    _password = pass;
    Serial.printf("AP config saved: SSID=%s\n", _ssid.c_str());
    return true;
  }

  const String& ssid()     const { return _ssid; }
  const String& password() const { return _password; }

  static APConfigManager& getInstance() {
    static APConfigManager instance;
    return instance;
  }
};

// ===== WALLPAPER MANAGER =====
class WallpaperManager {
private:
  bool _hasWallpaper = false;
  bool _dirty        = false;

public:
  void init() {
    if (!LittleFS.begin(true)) {
      Serial.println("LittleFS mount failed");
      return;
    }
    _hasWallpaper = LittleFS.exists(Config::WALLPAPER_PATH);
    Serial.printf("LittleFS OK. Wallpaper: %s\n", _hasWallpaper ? "found" : "none");
  }

  bool hasWallpaper() const { return _hasWallpaper; }
  bool isDirty()      const { return _dirty; }
  void clearDirty()         { _dirty = false; }

  void notifyUpdated() {
    _hasWallpaper = true;
    _dirty = true;
  }

  void draw(LGFX* tft) {
    if (!_hasWallpaper) {
      tft->fillRect(0, 0, Config::SCREEN_W, Config::WALLPAPER_H, TFT_BLACK);
      return;
    }
    File f = LittleFS.open(Config::WALLPAPER_PATH, "r");
    if (!f) {
      tft->fillRect(0, 0, Config::SCREEN_W, Config::WALLPAPER_H, TFT_BLACK);
      return;
    }
    size_t   sz  = f.size();
    uint8_t* buf = (uint8_t*)malloc(sz);
    if (!buf) { f.close(); return; }
    f.read(buf, sz);
    f.close();
    tft->drawJpg(buf, sz, 0, 0, Config::SCREEN_W, Config::WALLPAPER_H);
    free(buf);
  }

  static WallpaperManager& getInstance() {
    static WallpaperManager instance;
    return instance;
  }
};

// ===== GREETING MANAGER =====
class GreetingManager {
private:
  bool   _enabled = false;
  String _name    = "";
  String _message = "";

public:
  void init() { load(); }

  void load() {
    _enabled = false;
    _name    = "";
    _message = "";
    if (!LittleFS.exists(Config::GREETING_PATH)) return;
    File f = LittleFS.open(Config::GREETING_PATH, "r");
    if (!f) return;
    String en  = f.readStringUntil('\n'); en.trim();
    String nm  = f.readStringUntil('\n'); nm.trim();
    String msg = f.readStringUntil('\n'); msg.trim();
    f.close();
    _enabled = (en == "1");
    _name    = nm;
    _message = msg;
    Serial.printf("Greeting loaded: enabled=%d name=%s msg=%s\n",
                  _enabled, _name.c_str(), _message.c_str());
  }

  bool save(bool enabled, const String& name, const String& message) {
    File f = LittleFS.open(Config::GREETING_PATH, "w");
    if (!f) return false;
    f.println(enabled ? "1" : "0");
    f.println(name);
    f.println(message);
    f.close();
    _enabled = enabled;
    _name    = name;
    _message = message;
    return true;
  }

  bool     isEnabled() const { return _enabled; }
  String   getName()   const { return _name; }
  String   getMessage()const { return _message; }

  String buildGreeting() const {
    return _message.length() > 0 ? _message : "halo!";
  }

  static GreetingManager& getInstance() {
    static GreetingManager instance;
    return instance;
  }
};

// ===== BRIGHTNESS MANAGER =====
class BrightnessManager {
private:
  uint8_t _brightness = Config::BRIGHTNESS_DEF;

public:
  void init() {
    ledcSetup(Config::BACKLIGHT_CH, Config::BACKLIGHT_FREQ, Config::BACKLIGHT_RES);
    ledcWrite(Config::BACKLIGHT_CH, 0);
    ledcAttachPin(Config::BACKLIGHT_PIN, Config::BACKLIGHT_CH);
    load();
    Serial.printf("Brightness init: loaded=%d (still OFF)\n", _brightness);
  }

  void load() {
    if (!LittleFS.exists(Config::BRIGHTNESS_PATH)) return;
    File f = LittleFS.open(Config::BRIGHTNESS_PATH, "r");
    if (!f) return;
    String s = f.readStringUntil('\n'); s.trim();
    f.close();
    int v = s.toInt();
    if (v >= 0 && v <= 255) _brightness = (uint8_t)v;
  }

  bool save(uint8_t v) {
    File f = LittleFS.open(Config::BRIGHTNESS_PATH, "w");
    if (!f) return false;
    f.println(v);
    f.close();
    _brightness = v;
    apply();
    return true;
  }

  void apply() { ledcWrite(Config::BACKLIGHT_CH, _brightness); }

  void setOn(bool on) { ledcWrite(Config::BACKLIGHT_CH, on ? _brightness : 0); }

  uint8_t get() const { return _brightness; }

  static BrightnessManager& getInstance() {
    static BrightnessManager instance;
    return instance;
  }
};

// ===== THEME MANAGER =====
class ThemeManager {
private:
  uint32_t _clockColor = 0xFFFFFF;
  uint32_t _dateColor  = 0xAAAAAA;

  uint16_t toRGB565(uint32_t rgb888) const {
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8)  & 0xFF;
    uint8_t b = (rgb888)       & 0xFF;
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
  }

public:
  void init() { load(); }

  void load() {
    if (!LittleFS.exists(Config::THEME_PATH)) return;
    File f = LittleFS.open(Config::THEME_PATH, "r");
    if (!f) return;
    String s1 = f.readStringUntil('\n'); s1.trim();
    String s2 = f.readStringUntil('\n'); s2.trim();
    f.close();
    auto parseHex = [](const String& s, uint32_t& out) {
      if (s.length() == 6 || s.length() == 7) {
        String hex = s.startsWith("#") ? s.substring(1) : s;
        if (hex.length() == 6) out = strtoul(hex.c_str(), nullptr, 16);
      }
    };
    parseHex(s1, _clockColor);
    if (s2.length() > 0) parseHex(s2, _dateColor);
  }

  bool save(const String& hexClock, const String& hexDate = "") {
    String hc = hexClock.startsWith("#") ? hexClock.substring(1) : hexClock;
    if (hc.length() != 6) return false;
    String hd = hexDate.startsWith("#") ? hexDate.substring(1) : hexDate;
    if (hd.length() != 6 && hd.length() != 0) return false;
    File f = LittleFS.open(Config::THEME_PATH, "w");
    if (!f) return false;
    f.println(hc);
    f.println(hd.length() == 6 ? hd : "AAAAAA");
    f.close();
    _clockColor = strtoul(hc.c_str(), nullptr, 16);
    if (hd.length() == 6) _dateColor = strtoul(hd.c_str(), nullptr, 16);
    return true;
  }

  uint16_t getClockColor565() const { return toRGB565(_clockColor); }
  uint16_t getDateColor565()  const { return toRGB565(_dateColor); }
  uint32_t getClockColorRGB() const { return _clockColor; }
  uint32_t getDateColorRGB()  const { return _dateColor; }

  String getHexString() const {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%06X", _clockColor);
    return String(buf);
  }

  String getDateHexString() const {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%06X", _dateColor);
    return String(buf);
  }

  static ThemeManager& getInstance() {
    static ThemeManager instance;
    return instance;
  }
};

// ===== DISPLAY MANAGER =====
class DisplayManager {
private:
  LGFX _tft;

  static constexpr uint32_t COL_BG       = 0x000000;
  static constexpr uint16_t COL_BG16     = 0x0000;
  static constexpr uint16_t COL_PINK     = 0xFBB6;
  static constexpr uint16_t COL_CREAM    = 0xFFDD;
  static constexpr uint16_t COL_LAVENDER = 0xCCBF;
  static constexpr uint16_t COL_WHITE    = 0xFFFF;
  static constexpr uint16_t COL_DARKPINK = 0xF8B4;
  static constexpr uint16_t COL_MUTED    = 0x9CF3;

  lgfx::LGFX_Sprite _splash;
  lgfx::LGFX_Sprite _backBuffer;  // Double buffer untuk smooth rendering

  void drawMochiPixelArt(lgfx::LGFX_Sprite* target, int16_t cx, int16_t cy) {
    const uint8_t ART[24][24] = {
      {0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0},
      {0,0,0,0,3,3,1,1,1,1,1,1,1,1,1,1,1,1,3,3,0,0,0,0},
      {0,0,0,3,1,1,5,5,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0,0},
      {0,0,3,1,1,5,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0},
      {0,3,1,1,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,4,4,1,1,1,1,4,4,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,4,4,1,1,1,1,4,4,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,3,1,1,3,1,1,1,1,3,1,1,3,1,1,1,1,1,3},
      {3,1,1,1,1,1,3,1,1,3,1,1,1,1,3,1,1,3,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},
      {3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3},
      {0,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0},
      {0,0,3,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,3,0},
      {0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0},
      {0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,0,0},
      {0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };
    const uint16_t palette[] = {
      0x0000, COL_CREAM, COL_DARKPINK, 0x2104, COL_PINK, COL_WHITE
    };
    const uint8_t SCALE = 2;
    for (int8_t row = 0; row < 24; row++) {
      for (int8_t col = 0; col < 24; col++) {
        uint8_t v = ART[row][col];
        if (v == 0) continue;
        target->fillRect(col * SCALE, row * SCALE, SCALE, SCALE, palette[v]);
      }
    }
  }

  float easeOutBounce(float t) {
    if (t < 1.0f / 2.75f)       return 7.5625f * t * t;
    else if (t < 2.0f / 2.75f) { t -= 1.5f / 2.75f;   return 7.5625f * t * t + 0.75f; }
    else if (t < 2.5f / 2.75f) { t -= 2.25f / 2.75f;  return 7.5625f * t * t + 0.9375f; }
    else                         { t -= 2.625f / 2.75f; return 7.5625f * t * t + 0.984375f; }
  }

  float easeOutCubic(float t) {
    return 1.0f - pow(1.0f - t, 3.0f);
  }

public:
  DisplayManager() : _tft(), _splash(&_tft), _backBuffer(&_tft) {}

  void init() {
    _tft.init();
    _tft.setRotation(Config::DISPLAY_ROTATION);
    _tft.fillScreen(TFT_BLACK);
    setBacklight(true);
  }

  void showSplash() {
    _tft.fillScreen(COL_BG16);
    setBacklight(true);

    // Create backbuffer for smooth rendering
    _backBuffer.createSprite(Config::SCREEN_W, Config::SCREEN_H);
    _backBuffer.setColorDepth(16);
    _backBuffer.fillSprite(COL_BG16);

    const uint8_t  MOCHI_PX       = 48;
    const int16_t  MOCHI_CX       = Config::SCREEN_W / 2;
    const int16_t  MOCHI_TARGET_Y = 82;
    const int16_t  MOCHI_START_Y  = -MOCHI_PX;

    _splash.createSprite(MOCHI_PX, MOCHI_PX);
    _splash.setColorDepth(16);

    // ===== ANIMASI JATUH MOCHI =====
    const uint8_t DROP_FRAMES = 40;
    for (uint8_t f = 0; f <= DROP_FRAMES; f++) {
      _backBuffer.fillSprite(COL_BG16);
      
      float t   = (float)f / DROP_FRAMES;
      float bt  = easeOutBounce(t);
      int16_t y = (int16_t)(MOCHI_START_Y + (MOCHI_TARGET_Y - MOCHI_START_Y) * bt);

      // Draw shadow
      float shadowAlpha = t * 0.7f;
      uint8_t shadowW = (uint8_t)(4 + 24 * shadowAlpha);
      _backBuffer.fillEllipse(MOCHI_CX, MOCHI_TARGET_Y + MOCHI_PX - 4,
                       shadowW, 3, 0x0803);

      // Draw Mochi
      _splash.fillSprite(COL_BG16);
      drawMochiPixelArt(&_splash, 0, 0);
      _splash.pushSprite(&_backBuffer, MOCHI_CX - MOCHI_PX/2, y);

      // Push to display
      _backBuffer.pushSprite(0, 0);
      
      esp_task_wdt_reset(); 
      delay(25);
    }
    _splash.deleteSprite();

    // ===== ANIMASI TULISAN MOCHI HURUF PER HURUF =====
    const char* title = "MOCHI";
    const uint8_t  TSIZE  = 4;
    const int16_t  TITLE_Y = MOCHI_TARGET_Y + MOCHI_PX + 10;
    const int16_t  TITLE_W = strlen(title) * 6 * TSIZE;
    const int16_t  TITLE_X = (Config::SCREEN_W - TITLE_W) / 2;

    // Redraw Mochi yang sudah di posisi akhir
    _splash.createSprite(MOCHI_PX, MOCHI_PX);
    _splash.setColorDepth(16);
    
    for (uint8_t i = 0; i < strlen(title); i++) {
      _backBuffer.fillSprite(COL_BG16);
      
      // Gambar ulang Mochi di posisi akhir
      _splash.fillSprite(COL_BG16);
      drawMochiPixelArt(&_splash, 0, 0);
      _splash.pushSprite(&_backBuffer, MOCHI_CX - MOCHI_PX/2, MOCHI_TARGET_Y);
      
      // Gambar tulisan MOCHI sampai huruf ke-i
      _backBuffer.setTextColor(COL_CREAM);
      _backBuffer.setTextSize(TSIZE);
      for (uint8_t j = 0; j <= i; j++) {
        _backBuffer.setCursor(TITLE_X + j * 6 * TSIZE, TITLE_Y);
        _backBuffer.print(title[j]);
      }

      // Gambar sparkles di sekitar huruf
      int16_t lx = TITLE_X + i * 6 * TSIZE + 10;
      int16_t ly = TITLE_Y + 8;
      _backBuffer.drawPixel(lx + 14, ly - 10, COL_PINK);
      _backBuffer.drawPixel(lx + 16, ly - 8,  COL_LAVENDER);
      _backBuffer.drawPixel(lx - 2,  ly - 12, COL_PINK);

      _backBuffer.pushSprite(0, 0);
      esp_task_wdt_reset(); 
      delay(70);
    }
    _splash.deleteSprite();

    // ===== SUBTITLE =====
    delay(80);
    const char* sub = "esp32-c3 clock";
    _backBuffer.setTextSize(1);
    _backBuffer.setTextColor(COL_MUTED);
    int16_t sw = strlen(sub) * 6;
    _backBuffer.setCursor((Config::SCREEN_W - sw) / 2, TITLE_Y + 6 * TSIZE + 8);
    _backBuffer.print(sub);
    _backBuffer.pushSprite(0, 0);

    // ===== SPARKLES =====
    struct { int16_t x, y; uint16_t col; uint8_t sz; uint8_t delay_ms; } sparkles[] = {
      { 30,  20,  COL_PINK,     2, 40 },
      { 200, 35,  COL_LAVENDER, 2, 50 },
      { 50,  90,  COL_CREAM,    1, 40 },
      { 195, 80,  COL_PINK,     2, 35 },
      { 110, 15,  COL_LAVENDER, 2, 60 },
      { 170, 110, COL_CREAM,    1, 40 },
      { 20,  55,  COL_PINK,     1, 50 },
      { 215, 55,  COL_LAVENDER, 2, 55 },
    };
    const uint8_t N_SPARKLES = sizeof(sparkles) / sizeof(sparkles[0]);

    for (uint8_t s = 0; s < N_SPARKLES; s++) {
      int16_t sx = sparkles[s].x;
      int16_t sy = sparkles[s].y;
      uint16_t sc = sparkles[s].col;
      uint8_t  sz = sparkles[s].sz;
      
      _backBuffer.drawPixel(sx, sy, sc);
      if (sz >= 2) {
        _backBuffer.drawPixel(sx-1, sy, sc); 
        _backBuffer.drawPixel(sx+1, sy, sc);
        _backBuffer.drawPixel(sx, sy-1, sc); 
        _backBuffer.drawPixel(sx, sy+1, sc);
      }
      if (sz >= 3) {
        _backBuffer.drawPixel(sx-2, sy, sc); 
        _backBuffer.drawPixel(sx+2, sy, sc);
        _backBuffer.drawPixel(sx, sy-2, sc); 
        _backBuffer.drawPixel(sx, sy+2, sc);
      }
      
      _backBuffer.pushSprite(0, 0);
      delay(sparkles[s].delay_ms);
    }

    // ===== PROGRESS BAR =====
    delay(80);
    const int16_t BAR_W = 140;
    const int16_t BAR_H = 8;
    const int16_t BAR_X = (Config::SCREEN_W - BAR_W) / 2;
    const int16_t BAR_Y = 210;
    const uint8_t BAR_R = 4;

    _backBuffer.fillRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, BAR_R, 0x2104);
    _backBuffer.pushSprite(0, 0);

    // Animasi loading bar (gerak-gerak)
    for (uint8_t d = 0; d < 12; d++) {
      _backBuffer.fillRoundRect(BAR_X + 1, BAR_Y + 1, BAR_W - 2, BAR_H - 2, BAR_R - 1, 0x2104);
      float t   = (float)d / 11;
      int16_t dx = BAR_X + 4 + (int16_t)((BAR_W - 12) * (d % 2 == 0 ? t : 1.0f - t));
      _backBuffer.fillCircle(dx, BAR_Y + BAR_H / 2, 3, COL_PINK);
      _backBuffer.pushSprite(0, 0);
      esp_task_wdt_reset(); 
      delay(30);
    }

    // Progress bar fill
    const char* labels[] = { "initializing..", "loading assets.", "almost ready!!" };
    const int16_t  LABEL_Y    = BAR_Y + BAR_H + 10;
    const uint8_t  FILL_STEPS = 40;

    for (uint8_t i = 0; i <= FILL_STEPS; i++) {
      float t   = easeOutCubic((float)i / FILL_STEPS);
      int16_t fw = (int16_t)((BAR_W - 2) * t);

      _backBuffer.fillRoundRect(BAR_X + 1, BAR_Y + 1, BAR_W - 2, BAR_H - 2, BAR_R - 1, 0x2104);
      if (fw > 0) {
        for (int16_t px = 0; px < fw; px++) {
          float pct = (float)px / (BAR_W - 2);
          uint8_t r = (uint8_t)(0xF8 + (0xCC - 0xF8) * pct) >> 3;
          uint8_t g = (uint8_t)(0x90 + (0xC0 - 0x90) * pct) >> 2;
          uint8_t b = (uint8_t)(0xB0 + (0xBF - 0xB0) * pct) >> 3;
          uint16_t col = (r << 11) | (g << 5) | b;
          _backBuffer.drawFastVLine(BAR_X + 1 + px, BAR_Y + 1, BAR_H - 2, col);
        }
        _backBuffer.fillCircle(BAR_X + 1 + fw, BAR_Y + BAR_H / 2, 2, COL_WHITE);
      }

      const char* lbl = (t < 0.4f) ? labels[0] : (t < 0.85f) ? labels[1] : labels[2];
      _backBuffer.fillRect(0, LABEL_Y, Config::SCREEN_W, 10, COL_BG16);
      _backBuffer.setTextSize(1);
      _backBuffer.setTextColor(COL_MUTED);
      int16_t lw = strlen(lbl) * 6;
      _backBuffer.setCursor((Config::SCREEN_W - lw) / 2, LABEL_Y);
      _backBuffer.print(lbl);

      _backBuffer.pushSprite(0, 0);
      esp_task_wdt_reset(); 
      delay(25);
    }

    delay(200);
    _backBuffer.deleteSprite();
    _tft.fillScreen(TFT_BLACK);
  }

  void showGreeting() {
    String greeting = GreetingManager::getInstance().buildGreeting();
    _tft.fillScreen(TFT_BLACK);

    lgfx::LGFX_Sprite sp(&_tft);
    sp.createSprite(48, 48);
    sp.setColorDepth(16);
    sp.fillSprite(TFT_BLACK);
    const uint8_t ART[24][24] = {
      {0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0},
      {0,0,0,0,3,3,1,1,1,1,1,1,1,1,1,1,1,1,3,3,0,0,0,0},
      {0,0,0,3,1,1,5,5,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0,0},
      {0,0,3,1,1,5,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0},
      {0,3,1,1,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,4,4,1,1,1,1,4,4,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,4,4,1,1,1,1,4,4,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,3,1,1,3,1,1,1,1,3,1,1,3,1,1,1,1,1,3},
      {3,1,1,1,1,1,3,1,1,3,1,1,1,1,3,1,1,3,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3,3,1,1,1,1,1,1,3},
      {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
      {3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},
      {3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3},
      {0,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0},
      {0,0,3,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,3,0},
      {0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0},
      {0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,0,0},
      {0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };
    const uint16_t palette[] = {
      0x0000, COL_CREAM, COL_DARKPINK, 0x2104, COL_PINK, COL_WHITE
    };
    for (int8_t row = 0; row < 24; row++)
      for (int8_t col = 0; col < 24; col++) {
        uint8_t v = ART[row][col];
        if (v == 0) continue;
        sp.fillRect(col*2, row*2, 2, 2, palette[v]);
      }
    sp.pushSprite((Config::SCREEN_W - 48) / 2, 52);
    sp.deleteSprite();

    _tft.setTextColor(COL_CREAM);

    if (greeting.length() <= 18) {
      _tft.setTextSize(2);
      int16_t tw = greeting.length() * 6 * 2;
      _tft.setCursor((Config::SCREEN_W - tw) / 2, 118);
      _tft.print(greeting);
    } else {
      int mid = greeting.length() / 2;
      int split = mid;
      for (int d = 0; d <= mid; d++) {
        if (split - d >= 0 && greeting[split - d] == ' ') { split = split - d; break; }
        if (split + d < (int)greeting.length() && greeting[split + d] == ' ') { split = split + d; break; }
      }
      String line1 = greeting.substring(0, split);
      String line2 = greeting.substring(split + 1);
      _tft.setTextSize(2);
      int16_t tw1 = line1.length() * 6 * 2;
      int16_t tw2 = line2.length() * 6 * 2;
      _tft.setCursor((Config::SCREEN_W - tw1) / 2, 112);
      _tft.print(line1);
      _tft.setCursor((Config::SCREEN_W - tw2) / 2, 130);
      _tft.print(line2);
    }

    _tft.setTextSize(1);
    _tft.setTextColor(0x4208);
    const char* hint = "tap to continue";
    int16_t hw = strlen(hint) * 6;
    _tft.setCursor((Config::SCREEN_W - hw) / 2, 190);
    _tft.print(hint);

    unsigned long start = millis();
    while (millis() - start < 8000) {
      esp_task_wdt_reset();
      bool tapAllowed = (millis() - start) > 600;
      if (tapAllowed && digitalRead(Config::TTP223_PIN) == LOW) break;
      delay(30);
    }

    _tft.fillScreen(TFT_BLACK);
  }

  LGFX* getTft() { return &_tft; }

  void setBacklight(bool on) { BrightnessManager::getInstance().setOn(on); }

  void drawFrame(const VideoInfo* video, uint16_t frameIndex) {
    const uint8_t* jpg_data = (const uint8_t*)pgm_read_ptr(&video->frames[frameIndex]);
    uint16_t       jpg_size = pgm_read_word(&video->frames_size[frameIndex]);
    _tft.drawJpg(jpg_data, jpg_size, 0, 0);
  }

  void drawWallpaper() {
    WallpaperManager::getInstance().draw(&_tft);
  }

  void drawClockBar() {
    struct tm timeinfo;
    lgfx::LGFX_Sprite sprite(&_tft);
    sprite.createSprite(Config::SCREEN_W, Config::CLOCK_BAR_H);
    sprite.fillScreen(TFT_BLACK);

    if (!getLocalTime(&timeinfo, 50)) {
      sprite.setTextColor(TFT_WHITE);
      sprite.setTextSize(2);
      const char* noTime = "--:--:--";
      int16_t tw = strlen(noTime) * 6 * 2;
      sprite.setCursor((Config::SCREEN_W - tw) / 2, 12);
      sprite.print(noTime);
      sprite.setTextSize(1);
      sprite.setCursor(52, 40);
      sprite.setTextColor(TFT_DARKGREY);
      sprite.print("Sync waktu via WiFi");
    } else {
      char timeBuf[9];
      char dateBuf[11];
      strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
      strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y", &timeinfo);

      uint16_t clockCol = ThemeManager::getInstance().getClockColor565();
      sprite.setTextColor(clockCol);
      sprite.setTextSize(3);
      int16_t tw = strlen(timeBuf) * 6 * 3;
      sprite.setCursor((Config::SCREEN_W - tw) / 2, 7);
      sprite.print(timeBuf);

      uint16_t dateCol = ThemeManager::getInstance().getDateColor565();
      sprite.setTextColor(dateCol);
      sprite.setTextSize(2);
      int16_t dw = strlen(dateBuf) * 6 * 2;
      sprite.setCursor((Config::SCREEN_W - dw) / 2, 40);
      sprite.print(dateBuf);
    }

    sprite.pushSprite(0, Config::CLOCK_BAR_Y);
    sprite.deleteSprite();
  }

  static DisplayManager& getInstance() {
    static DisplayManager instance;
    return instance;
  }
};

// ===== ANALOG CLOCK FACE =====
class AnalogClockFace {
private:
  // Gambar satu jarum dengan lebar w pixel, dengan ekor pendek berlawanan arah
  void drawHand(lgfx::LGFX_Sprite* sp,
                int16_t cx, int16_t cy,
                float   angle,
                uint8_t len,
                uint8_t tailLen,
                uint8_t w,
                uint16_t col,
                uint16_t shadowCol) {
    float ex   = cx + len     * cos(angle);
    float ey   = cy + len     * sin(angle);
    float tx   = cx - tailLen * cos(angle);
    float ty   = cy - tailLen * sin(angle);
    float perp = angle + M_PI / 2.0f;

    // Shadow satu pixel di samping
    sp->drawLine(cx + cos(perp), cy + sin(perp),
                 ex + cos(perp), ey + sin(perp), shadowCol);
    sp->drawLine(cx - cos(perp), cy - sin(perp),
                 ex - cos(perp), ey - sin(perp), shadowCol);

    // Jarum utama (lebar w)
    for (int8_t i = -(int8_t)(w / 2); i <= (int8_t)(w / 2); i++) {
      sp->drawLine(
        (int16_t)(tx + i * cos(perp)), (int16_t)(ty + i * sin(perp)),
        (int16_t)(ex + i * cos(perp)), (int16_t)(ey + i * sin(perp)),
        col
      );
    }
  }

public:
  void draw(LGFX* tft) {
    const int16_t CX = 120, CY = 120, R = 108;

    lgfx::LGFX_Sprite sp(tft);
    sp.createSprite(Config::SCREEN_W, Config::SCREEN_H);
    sp.fillScreen(TFT_BLACK);

    // ── Lingkaran luar ──────────────────────────────────────
    sp.drawCircle(CX, CY, R,     0x4208);  // gelap
    sp.drawCircle(CX, CY, R - 1, 0x8410);  // sedikit terang

    // ── Tick marks & angka 1-12 ─────────────────────────────
    for (uint8_t i = 0; i < 60; i++) {
      float angle   = (i * 6.0f - 90.0f) * DEG_TO_RAD;
      bool  isHour  = (i % 5 == 0);

      uint8_t  tickOuter = R - 2;
      uint8_t  tickInner = isHour ? R - 14 : R - 7;
      uint16_t tickCol   = isHour ? 0x8C71 : 0x4208;

      int16_t x0 = CX + tickOuter * cos(angle);
      int16_t y0 = CY + tickOuter * sin(angle);
      int16_t x1 = CX + tickInner * cos(angle);
      int16_t y1 = CY + tickInner * sin(angle);

      if (isHour) {
        // Tick jam lebih tebal
        sp.drawLine(x0, y0, x1, y1, tickCol);
        sp.drawLine(x0 + 1, y0, x1 + 1, y1, tickCol);
      } else {
        sp.drawLine(x0, y0, x1, y1, tickCol);
      }

      // Angka jam
      if (isHour) {
        uint8_t num  = i / 5;
        if (num == 0) num = 12;
        uint8_t numR = R - 26;
        int16_t nx   = CX + numR * cos(angle);
        int16_t ny   = CY + numR * sin(angle);

        sp.setTextSize(1);
        sp.setTextColor(0xC618);  // abu terang

        // Offset agar angka center di posisi
        int8_t offX = (num >= 10) ? -5 : -3;
        int8_t offY = -4;
        sp.setCursor(nx + offX, ny + offY);
        sp.print(num);
      }
    }

    // ── Baca waktu ──────────────────────────────────────────
    struct tm t;
    if (!getLocalTime(&t, 50)) {
      // Kalau waktu belum diset, tampilkan tanda tanya di tengah
      sp.setTextColor(0x4208);
      sp.setTextSize(2);
      sp.setCursor(112, 112);
      sp.print("?");
      sp.pushSprite(0, 0);
      sp.deleteSprite();
      return;
    }

    float secAngle  = (t.tm_sec  * 6.0f          - 90.0f) * DEG_TO_RAD;
    float minAngle  = (t.tm_min  * 6.0f + t.tm_sec  * 0.1f - 90.0f) * DEG_TO_RAD;
    float hourAngle = ((t.tm_hour % 12) * 30.0f + t.tm_min * 0.5f - 90.0f) * DEG_TO_RAD;

    // ── Jarum jam — PUTIH, lebar 5 ───────────────────
    drawHand(&sp, CX, CY, hourAngle,
             /*len*/52, /*tail*/12, /*w*/5,
             /*col*/0xFFFF,        // PUTIH
             /*shadow*/0x8410);    // abu

    // ── Jarum menit — PUTIH panjang, lebar 3 ─────────
    drawHand(&sp, CX, CY, minAngle,
             /*len*/82, /*tail*/14, /*w*/3,
             /*col*/0xFFFF,        // PUTIH
             /*shadow*/0x8410);    // abu

    // ── Jarum detik — MERAH tipis, panjang ───────────
    {
      int16_t sx2 = CX + 90  * cos(secAngle);
      int16_t sy2 = CY + 90  * sin(secAngle);
      int16_t sx1 = CX - 20  * cos(secAngle);
      int16_t sy1 = CY - 20  * sin(secAngle);
      sp.drawLine(sx1, sy1, sx2, sy2, 0xF800);  // MERAH
    }

    // ── Center cap ──────────────────────────────────────────
    sp.fillCircle(CX, CY, 6, 0xF800);   // merah
    sp.fillCircle(CX, CY, 3, 0xFFFF);   // putih kecil di tengah
    sp.drawCircle(CX, CY, 6, 0x8000);   // outline gelap

    sp.pushSprite(0, 0);
    sp.deleteSprite();
  }

  static AnalogClockFace& getInstance() {
    static AnalogClockFace instance;
    return instance;
  }
};

// ===== MPU6050 MANAGER =====
class MPU6050Manager {
private:
  bool          _initialized      = false;
  unsigned long _lastReadTime     = 0;
  float         _accelX = 0, _accelY = 0, _accelZ = 0;
  bool          _shakeDetected    = false;
  float         _lastMagnitude    = 1.0;
  int           _shakeCount       = 0;
  unsigned long _shakeWindowStart = 0;
  unsigned long _lastShakeTime    = 0;

  void writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(Config::MPU6050_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
  }

  void readAccelData() {
    Wire.beginTransmission(Config::MPU6050_I2C_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(Config::MPU6050_I2C_ADDR, (uint8_t)6);
    int16_t ax = (Wire.read() << 8) | Wire.read();
    int16_t ay = (Wire.read() << 8) | Wire.read();
    int16_t az = (Wire.read() << 8) | Wire.read();
    _accelX = ax / 16384.0f;
    _accelY = ay / 16384.0f;
    _accelZ = az / 16384.0f;
  }

public:
  void init() {
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);
    Wire.beginTransmission(Config::MPU6050_I2C_ADDR);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      writeRegister(0x6B, 0x00);
      writeRegister(0x1C, 0x00);
      writeRegister(0x1A, 0x03);
      delay(100);
      _initialized = true;
      Serial.println("MPU6050 initialized");
    } else {
      Serial.printf("MPU6050 init failed, error: %d\n", error);
    }
  }

  void update() {
    if (!_initialized) return;
    unsigned long now = millis();
    if (now - _lastReadTime < 20) return;
    _lastReadTime = now;

    readAccelData();
    float magnitude  = sqrt(_accelX*_accelX + _accelY*_accelY + _accelZ*_accelZ);
    float deltaAccel = abs(magnitude - _lastMagnitude);
    _lastMagnitude   = magnitude;

    if (now - _lastShakeTime < Config::SHAKE_COOLDOWN_TIME) return;

    if (deltaAccel > Config::SHAKE_THRESHOLD) {
      if (_shakeCount == 0) _shakeWindowStart = now;
      _shakeCount++;
      if (_shakeCount >= 3 && (now - _shakeWindowStart) <= Config::SHAKE_DETECTION_TIME) {
        if (!_shakeDetected) {
          _shakeDetected = true;
          _lastShakeTime = now;
          Serial.printf("Shake detected! %d shakes\n", _shakeCount);
        }
      }
    }
    if (_shakeCount > 0 && (now - _shakeWindowStart) > Config::SHAKE_DETECTION_TIME) {
      _shakeCount = 0;
    }
  }

  bool isShakeDetected() {
    bool d = _shakeDetected;
    if (d) _shakeDetected = false;
    return d;
  }

  static MPU6050Manager& getInstance() {
    static MPU6050Manager instance;
    return instance;
  }
};

// ===== INPUT MANAGER =====
class InputManager {
private:
  ButtonState   _buttonState       = ButtonState::IDLE;
  unsigned long _buttonPressTime   = 0;
  bool          _physicalState     = false;
  unsigned long _lastDebounceTime  = 0;
  unsigned long _lastUpdateTime    = 0;
  uint8_t       _tapCount          = 0;
  unsigned long _lastTapTime       = 0;
  bool          _multiTapTriggered = false;

public:
  void init() {
    pinMode(Config::TTP223_PIN, INPUT_PULLUP);
    _lastUpdateTime = millis();
  }

  void quickUpdate() {
    unsigned long now = millis();
    if (now - _lastUpdateTime < 2) return;
    _lastUpdateTime = now;
    bool newState = (digitalRead(Config::TTP223_PIN) == LOW);
    if (newState != _physicalState) _lastDebounceTime = now;
    _physicalState = newState;
  }

  void update() {
    quickUpdate();
    processButton();
    checkTapReset();
  }

  // Consuming: baca state dan reset RELEASED_* ke IDLE
  ButtonState getButtonState() {
    ButtonState s = _buttonState;
    if (_buttonState == ButtonState::RELEASED_SHORT ||
        _buttonState == ButtonState::RELEASED_HOLD) {
      _buttonState = ButtonState::IDLE;
    }
    return s;
  }

  // Non-consuming: hanya lihat state tanpa mereset
  ButtonState peekButtonState() const {
    return _buttonState;
  }

  bool isMultiTapTriggered() {
    bool t = _multiTapTriggered;
    if (t) _multiTapTriggered = false;
    return t;
  }

private:
  void processButton() {
    unsigned long now = millis();
    if (now - _lastDebounceTime <= Config::DEBOUNCE_TIME_MS) return;

    if (_physicalState) {
      if (_buttonState == ButtonState::IDLE ||
          _buttonState == ButtonState::RELEASED_SHORT ||
          _buttonState == ButtonState::RELEASED_HOLD) {
        _buttonState     = ButtonState::PRESSED;
        _buttonPressTime = now;
      } else if (_buttonState == ButtonState::PRESSED &&
                 now - _buttonPressTime >= Config::HOLDING_TIME_MS) {
        _buttonState = ButtonState::HELD;
      }
    } else {
      if (_buttonState == ButtonState::PRESSED) {
        _tapCount++;
        _lastTapTime = now;
        Serial.printf("Tap count: %d\n", _tapCount);
        if (_tapCount >= Config::TAP_COUNT_TRIGGER) {
          _multiTapTriggered = true;
          _tapCount          = 0;
          Serial.println("Multi-tap triggered!");
        }
        _buttonState = ButtonState::RELEASED_SHORT;
      } else if (_buttonState == ButtonState::HELD) {
        _tapCount    = 0;
        _lastTapTime = 0;
        _buttonState = ButtonState::RELEASED_HOLD;
      }
    }
  }

  void checkTapReset() {
    if (_tapCount > 0 && _lastTapTime > 0 &&
        millis() - _lastTapTime > Config::TAP_RESET_MS) {
      Serial.printf("Tap reset (count was %d)\n", _tapCount);
      _tapCount    = 0;
      _lastTapTime = 0;
    }
  }

public:
  static InputManager& getInstance() {
    static InputManager instance;
    return instance;
  }
};

// ===== CLOCK MANAGER =====
class ClockManager {
private:
  bool _timeSynced = false;
  unsigned long _lastSaveTime = 0;

  void saveToFlash() {
    time_t now = time(nullptr);
    if (now > 0) {
      File f = LittleFS.open(Config::TIME_SAVE_PATH, "w");
      if (f) {
        f.write((uint8_t*)&now, sizeof(now));
        f.close();
        Serial.printf("Time saved: %lu\n", now);
      }
    }
  }

public:
  void init() {
    configTime(0, 0, nullptr);
    loadFromFlash();
    Serial.println("Clock manager initialized");
  }

  void loadFromFlash() {
    if (!LittleFS.exists(Config::TIME_SAVE_PATH)) return;
    File f = LittleFS.open(Config::TIME_SAVE_PATH, "r");
    if (!f) return;
    time_t saved;
    if (f.read((uint8_t*)&saved, sizeof(saved)) == sizeof(saved) && saved > 0) {
      struct timeval tv = { saved, 0 };
      settimeofday(&tv, nullptr);
      _timeSynced = true;
      Serial.printf("Loaded saved time: %lu\n", saved);
    }
    f.close();
  }

  bool isTimeSynced() { return _timeSynced; }

  void setTime(int year, int month, int day, int hour, int minute, int second) {
    struct tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;
    time_t epoch = mktime(&t);
    struct timeval tv = { epoch, 0 };
    settimeofday(&tv, nullptr);
    _timeSynced = true;
    Serial.printf("Time set: %04d-%02d-%02d %02d:%02d:%02d\n",
                  year, month, day, hour, minute, second);
    saveToFlash();
  }

  void periodicSave() {
    unsigned long now = millis();
    if (now - _lastSaveTime >= 30000) {
      _lastSaveTime = now;
      saveToFlash();
    }
  }

  static ClockManager& getInstance() {
    static ClockManager instance;
    return instance;
  }
};

// ===== WEB MANAGER =====
class WebManager {
private:
  WebServer  _server;
  DNSServer  _dns;
  File       _uploadFile;
  bool       _uploadSuccess  = false;
  bool       _restartPending = false;

  static const char* getHTML() {
    return MOCHI_HTML;
  }

public:
  WebManager() : _server(80) {}

  void init() {
    const String& ssid = APConfigManager::getInstance().ssid();
    const String& pass = APConfigManager::getInstance().password();

    IPAddress ip(192, 168, 4, 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid.c_str(), pass.c_str());
    Serial.printf("AP: %s | IP: %s\n", ssid.c_str(), WiFi.softAPIP().toString().c_str());

    _dns.start(53, "*", ip);

    _server.on("/", [this]() { _server.send(200, "text/html", getHTML()); });
    _server.on("/generate_204", [this]() { _server.send(204, "text/plain", ""); });
    _server.on("/gen_204",      [this]() { _server.send(204, "text/plain", ""); });
    _server.on("/hotspot-detect.html", [this]() {
      _server.send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });
    _server.on("/ncsi.txt",        [this]() { _server.send(200, "text/plain", "Microsoft NCSI"); });
    _server.on("/connecttest.txt", [this]() { _server.send(200, "text/plain", "Microsoft Connect Test"); });

    _server.on("/settime", [this]() {
      if (!_server.hasArg("y") || !_server.hasArg("mo") || !_server.hasArg("d") ||
          !_server.hasArg("h") || !_server.hasArg("mi") || !_server.hasArg("s")) {
        _server.send(400, "text/plain", "Parameter tidak lengkap"); return;
      }
      ClockManager::getInstance().setTime(
        _server.arg("y").toInt(),  _server.arg("mo").toInt(), _server.arg("d").toInt(),
        _server.arg("h").toInt(),  _server.arg("mi").toInt(), _server.arg("s").toInt()
      );
      _server.send(200, "text/plain", "✅ Waktu berhasil disinkronkan!");
    });

    _server.on("/setap", HTTP_POST, [this]() {
      if (!_server.hasArg("ssid") || !_server.hasArg("pass")) {
        _server.send(400, "text/plain", "Parameter tidak lengkap"); return;
      }
      String ssid = _server.arg("ssid"); ssid.trim();
      String pass = _server.arg("pass"); pass.trim();
      if (ssid.length() < 1 || ssid.length() > 32) { _server.send(400, "text/plain", "❌ SSID harus 1-32 karakter"); return; }
      if (pass.length() < 8 || pass.length() > 64)  { _server.send(400, "text/plain", "❌ Password harus 8-64 karakter"); return; }
      if (!APConfigManager::getInstance().save(ssid, pass)) { _server.send(500, "text/plain", "❌ Gagal menyimpan ke flash"); return; }
      _server.send(200, "text/plain", "✅ Tersimpan! Perangkat restart…");
      _restartPending = true;
    });

    _server.on("/setgreeting", HTTP_POST, [this]() {
      String enabled = _server.hasArg("enabled") ? _server.arg("enabled") : "0";
      String message = _server.hasArg("message") ? _server.arg("message") : "";
      message.trim();
      if (message.length() > 48) message = message.substring(0, 48);
      if (GreetingManager::getInstance().save(enabled == "1", "", message))
        _server.send(200, "text/plain", "✅ Greeting tersimpan!");
      else
        _server.send(500, "text/plain", "❌ Gagal menyimpan");
    });

    _server.on("/getgreeting", [this]() {
      auto& gm = GreetingManager::getInstance();
      String msg = gm.getMessage();
      msg.replace("\\", "\\\\"); msg.replace("\"", "\\\"");
      String json = "{\"enabled\":" + String(gm.isEnabled() ? "true" : "false") +
                    ",\"message\":\"" + msg + "\"}";
      _server.send(200, "application/json", json);
    });

    _server.on("/upload", HTTP_POST,
      [this]() {
        _server.send(_uploadSuccess ? 200 : 500, "text/plain",
          _uploadSuccess ? "✅ Wallpaper berhasil disimpan!" : "❌ Gagal menyimpan wallpaper");
      },
      [this]() {
        HTTPUpload& upload = _server.upload();
        if (upload.status == UPLOAD_FILE_START) {
          _uploadFile    = LittleFS.open(Config::WALLPAPER_TMP, "w");
          _uploadSuccess = false;
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          if (_uploadFile) _uploadFile.write(upload.buf, upload.currentSize);
        } else if (upload.status == UPLOAD_FILE_END) {
          if (_uploadFile) {
            _uploadFile.close();
            LittleFS.remove(Config::WALLPAPER_PATH);
            LittleFS.rename(Config::WALLPAPER_TMP, Config::WALLPAPER_PATH);
            WallpaperManager::getInstance().notifyUpdated();
            _uploadSuccess = true;
          }
        }
      }
    );

    _server.on("/setbrightness", HTTP_POST, [this]() {
      if (!_server.hasArg("value")) { _server.send(400, "text/plain", "❌ Missing value"); return; }
      int v = _server.arg("value").toInt();
      if (v < 0 || v > 255) { _server.send(400, "text/plain", "❌ Nilai 0-255"); return; }
      if (BrightnessManager::getInstance().save((uint8_t)v))
        _server.send(200, "text/plain", "✅ Brightness disimpan!");
      else
        _server.send(500, "text/plain", "❌ Gagal menyimpan");
    });

    _server.on("/settheme", HTTP_POST, [this]() {
      if (!_server.hasArg("color")) { _server.send(400, "text/plain", "❌ Missing color"); return; }
      String color = _server.arg("color"); color.trim();
      String dateColor = _server.hasArg("dateColor") ? _server.arg("dateColor") : ""; dateColor.trim();
      if (ThemeManager::getInstance().save(color, dateColor))
        _server.send(200, "text/plain", "✅ Theme tersimpan!");
      else
        _server.send(400, "text/plain", "❌ Format warna tidak valid");
    });

    _server.on("/getsettings", [this]() {
      String json = "{\"brightness\":" + String(BrightnessManager::getInstance().get()) +
                    ",\"clockColor\":\"" + ThemeManager::getInstance().getHexString() + "\"" +
                    ",\"dateColor\":\""  + ThemeManager::getInstance().getDateHexString() + "\"" +
                    ",\"hasWallpaper\":" + (WallpaperManager::getInstance().hasWallpaper() ? "true" : "false") + "}";
      _server.send(200, "application/json", json);
    });

    _server.on("/getwallpaper", [this]() {
      if (!WallpaperManager::getInstance().hasWallpaper()) {
        _server.send(404, "text/plain", "No wallpaper");
        return;
      }
      File f = LittleFS.open(Config::WALLPAPER_PATH, "r");
      if (!f) {
        _server.send(500, "text/plain", "Failed to open");
        return;
      }
      _server.streamFile(f, "image/jpeg");
      f.close();
    });

    _server.begin();
    Serial.println("Web server started");
  }

  void handle() {
    _dns.processNextRequest();
    _server.handleClient();
    if (_restartPending) { delay(500); ESP.restart(); }
  }

  static WebManager& getInstance() {
    static WebManager instance;
    return instance;
  }
};

// ===== VIDEO PLAYER =====
class VideoPlayer {
private:
  std::vector<VideoInfo*> _videos;
  uint8_t       _currentIndex         = 0;
  uint16_t      _currentFrame         = 0;
  PlayerMode    _currentMode          = PlayerMode::PLAYING;
  unsigned long _lastFrameTime        = 0;
  bool          _playingSpecialVideo  = false;
  bool          _playingHoldVideo     = false;
  uint16_t      _savedFrame           = 0;
  unsigned long _lastClockUpdate      = 0;
  bool          _wallpaperNeedsRedraw = false;
  bool          _holdSwitchDone       = false;

public:
  VideoPlayer() {
    _videos = { &full1, &chongmat1, &xoadau1 };
    Serial.printf("Loaded %d videos\n", (int)_videos.size());
  }

  void init() {
    DisplayManager::getInstance().getTft()->fillScreen(TFT_BLACK);
    _currentMode   = PlayerMode::PLAYING;
    _currentIndex  = 0;
    _currentFrame  = 0;
    _lastFrameTime = millis();
    DisplayManager::getInstance().setBacklight(true);
    Serial.println("VideoPlayer started");
  }

  // ── 3x tap: Video→Digital, Digital→Video, Analog tidak dihandle sini ──
  void toggleClockMode() {
    if (_currentMode == PlayerMode::PLAYING) {
      // VIDEO → CLOCK DIGITAL
      _savedFrame           = _currentFrame;
      _currentMode          = PlayerMode::CLOCK;
      _playingSpecialVideo  = false;
      _playingHoldVideo     = false;
      _wallpaperNeedsRedraw = true;
      _lastClockUpdate      = 0;
      Serial.println("Video → Clock digital");

    } else if (_currentMode == PlayerMode::CLOCK) {
      // CLOCK DIGITAL → VIDEO
      _currentMode   = PlayerMode::PLAYING;
      _currentIndex  = 0;
      _currentFrame  = _savedFrame;
      _lastFrameTime = millis();
      Serial.println("Clock digital → Video");

    } else if (_currentMode == PlayerMode::ANALOG_CLOCK) {
      // ANALOG → VIDEO (jika entah bagaimana 3x tap di sini)
      _currentMode   = PlayerMode::PLAYING;
      _currentIndex  = 0;
      _currentFrame  = _savedFrame;
      _lastFrameTime = millis();
      Serial.println("Analog → Video");
    }

    // Flush state tombol supaya tidak ada event sisa
    InputManager::getInstance().getButtonState();
  }

  PlayerMode getMode() const { return _currentMode; }

  void update() {
    // ── CLOCK DIGITAL / ANALOG ───────────────────────────────────────────
    if (_currentMode == PlayerMode::CLOCK || _currentMode == PlayerMode::ANALOG_CLOCK) {

      // Cek HOLD → toggle antara digital dan analog (keduanya saat HELD, permanen)
      ButtonState state = InputManager::getInstance().peekButtonState();
      if (state == ButtonState::HELD && !_holdSwitchDone) {
        _holdSwitchDone = true;
        // Konsumsi state agar tidak bocor ke handler lain
        InputManager::getInstance().getButtonState();
        if (_currentMode == PlayerMode::CLOCK) {
          _currentMode     = PlayerMode::ANALOG_CLOCK;
          DisplayManager::getInstance().getTft()->fillScreen(TFT_BLACK);
          _lastClockUpdate = 0;
          Serial.println("Clock digital → Analog");
        } else {
          _currentMode          = PlayerMode::CLOCK;
          _wallpaperNeedsRedraw = true;
          _lastClockUpdate      = 0;
          Serial.println("Analog → Clock digital");
        }
      } else if (state == ButtonState::IDLE || state == ButtonState::RELEASED_SHORT ||
                 state == ButtonState::RELEASED_HOLD) {
        // Reset flag setelah jari dilepas agar hold berikutnya bisa trigger lagi
        if (_holdSwitchDone) {
          InputManager::getInstance().getButtonState(); // konsumsi RELEASED_HOLD
          _holdSwitchDone = false;
        }
      }

      // Render sesuai mode
      if (_currentMode == PlayerMode::CLOCK) {
        // Redraw wallpaper kalau perlu
        if (_wallpaperNeedsRedraw || WallpaperManager::getInstance().isDirty()) {
          DisplayManager::getInstance().drawWallpaper();
          WallpaperManager::getInstance().clearDirty();
          _wallpaperNeedsRedraw = false;
          _lastClockUpdate      = 0;
        }
        // Update clock bar tiap detik
        unsigned long now = millis();
        if (now - _lastClockUpdate >= 1000) {
          _lastClockUpdate = now;
          DisplayManager::getInstance().drawClockBar();
        }

      } else {
        // ANALOG: redraw tiap detik
        unsigned long now = millis();
        if (now - _lastClockUpdate >= 1000) {
          _lastClockUpdate = now;
          AnalogClockFace::getInstance().draw(DisplayManager::getInstance().getTft());
        }
      }

      // Buang event shake di clock mode agar tidak keterusan ke video
      MPU6050Manager::getInstance().isShakeDetected();
      return;
    }

    // ── VIDEO PLAYING ─────────────────────────────────────────────────────
    handleShakeDetection();
    handleButtonInput();

    unsigned long now = millis();
    if (now - _lastFrameTime >= Config::FRAME_DELAY_MS) {
      _lastFrameTime = now;
      DisplayManager::getInstance().drawFrame(getCurrentVideo(), _currentFrame);
      _currentFrame++;
      if (_currentFrame >= getCurrentVideo()->num_frames) {
        if (_playingSpecialVideo) {
          _playingSpecialVideo = false;
          _currentIndex        = 0;
          _currentFrame        = _savedFrame;
        } else if (_playingHoldVideo) {
          _currentFrame = 0;
        } else {
          _currentFrame = 0;
        }
      }
    }
  }

private:
  void handleShakeDetection() {
    if (!MPU6050Manager::getInstance().isShakeDetected()) return;
    if (_playingSpecialVideo) return;
    if (_playingHoldVideo) _playingHoldVideo = false;
    _savedFrame          = (_currentIndex == 0) ? _currentFrame : _savedFrame;
    _playingSpecialVideo = true;
    _currentIndex        = 1;
    _currentFrame        = 0;
  }

  void handleButtonInput() {
    ButtonState state = InputManager::getInstance().getButtonState();
    if (state == ButtonState::HELD && !_playingHoldVideo && !_playingSpecialVideo) {
      _savedFrame       = _currentFrame;
      _playingHoldVideo = true;
      _currentIndex     = 2;
      _currentFrame     = 0;
    }
    if (state == ButtonState::RELEASED_HOLD && _playingHoldVideo && !_playingSpecialVideo) {
      _playingHoldVideo = false;
      _currentIndex     = 0;
      _currentFrame     = _savedFrame;
    }
  }

  VideoInfo* getCurrentVideo() const { return _videos[_currentIndex]; }

public:
  static VideoPlayer& getInstance() {
    static VideoPlayer instance;
    return instance;
  }
};

// ===== SETUP & LOOP =====
void setup() {
  Serial.begin(115200);
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);
  if (reason == ESP_RST_BROWNOUT) Serial.println("⚠️  BROWNOUT DETECTED");

  WallpaperManager::getInstance().init();
  APConfigManager::getInstance().init();
  GreetingManager::getInstance().init();
  BrightnessManager::getInstance().init();
  ThemeManager::getInstance().init();

  DisplayManager::getInstance().init();
  DisplayManager::getInstance().showSplash();
  if (GreetingManager::getInstance().isEnabled()) {
    DisplayManager::getInstance().showGreeting();
  }

  ClockManager::getInstance().init();
  WebManager::getInstance().init();

  InputManager::getInstance().init();
  MPU6050Manager::getInstance().init();

  VideoPlayer::getInstance().init();
  Serial.println("Boot complete");
}

void loop() {
  InputManager::getInstance().update();
  MPU6050Manager::getInstance().update();

  if (InputManager::getInstance().isMultiTapTriggered()) {
    VideoPlayer::getInstance().toggleClockMode();
  }

  VideoPlayer::getInstance().update();
  WebManager::getInstance().handle();
  ClockManager::getInstance().periodicSave();
}
