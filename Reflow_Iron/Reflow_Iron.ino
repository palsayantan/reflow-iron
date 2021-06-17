#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <max6675.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "icon.h"

// Pinouts
#define fan        1
#define relay      0
#define buzzer     11

#define thermoCS   10
#define thermoDO   12
#define thermoCLK  13

#define oneWireBus A5

// Variables
int temp = 0;
int temp1 = 0;
unsigned int preHeat = 120;
unsigned int reflow = 180;
unsigned int coolDown = 50;
unsigned int timer = 60;
unsigned int mm = 88;
unsigned int ss = 88;
bool reflowing;
bool cooling;
bool fanState;
int saveTimer;
unsigned long previousMillis = 0;
int x = 165, y = 231, w = 36, h = 36;


MCUFRIEND_kbv tft;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// require touch screen calibration
// Put your calibrated touchSensor values here
int XP = 6, XM = A2, YP = A1, YM = 7; //240x320 ID=0x3229
const int TS_LEFT = 139, TS_RT = 895, TS_TOP = 901, TS_BOT = 118;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// buttons
Adafruit_GFX_Button on_btn, off_btn, up1, dwn1, up2, dwn2, up3, dwn3, up4, dwn4, fanBtn;

// get touch input
int pixel_x, pixel_y;
bool Touch_getXY(void)
{
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}

#define BLACK   0x0000
#define WHITE   0xFFFF
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0

void setup(void)
{
  //Serial.begin(9600);   // Turn off serial communication while using serial ports
  pinMode(buzzer, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(fan, OUTPUT);
  digitalWrite(buzzer, LOW);
  digitalWrite(relay, LOW);
  digitalWrite(fan, LOW);
  //sensors.begin();
  tft.begin(0x3229);
  tft.setRotation(2);
  tft.fillScreen(BLACK);
  tft.setTextColor(BLACK, YELLOW);
  tft.setTextSize(2);
  tft.setCursor(45, 5);
  tft.print(" REFLOW IRON ");
  tft.fillRect(45, 0, 156, 5, YELLOW);
  tft.fillRect(45, 20, 156, 5, YELLOW);

  tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
  tft.pushColors((const uint8_t*)fan1, w * h, 1, false);
  tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
  fanBtn.initButton(&tft,  220, 250, 18, 24, WHITE, BLACK, WHITE, "F", 2);
  fanBtn.drawButton(false);

  // initialise buttons
  up1.initButton(&tft,  195, 108, 20, 20, WHITE, BLACK, WHITE, "+", 2);
  dwn1.initButton(&tft,  220, 108, 20, 20, WHITE, BLACK, WHITE, "-", 2);
  up1.drawButton(false);
  dwn1.drawButton(false);

  up2.initButton(&tft,  195, 143, 20, 20, WHITE, BLACK, WHITE, "+", 2);
  dwn2.initButton(&tft,  220, 143, 20, 20, WHITE, BLACK, WHITE, "-", 2);
  up2.drawButton(false);
  dwn2.drawButton(false);

  up3.initButton(&tft,  195, 178, 20, 20, WHITE, BLACK, WHITE, "+", 2);
  dwn3.initButton(&tft,  220, 178, 20, 20, WHITE, BLACK, WHITE, "-", 2);
  up3.drawButton(false);
  dwn3.drawButton(false);

  up4.initButton(&tft,  195, 213, 20, 20, WHITE, BLACK, WHITE, "+", 2);
  dwn4.initButton(&tft,  220, 213, 20, 20, WHITE, BLACK, WHITE, "-", 2);
  up4.drawButton(false);
  dwn4.drawButton(false);

  on_btn.initButton(&tft,  60, 295, 100, 40, WHITE, 0x068B, WHITE, "START", 2);
  off_btn.initButton(&tft, 180, 295, 100, 40, WHITE, 0xF340, WHITE, "RESET", 2);
  on_btn.drawButton(false);
  off_btn.drawButton(false);
}

void loop(void)
{
  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();

    temp = thermocouple.readCelsius();      // get temperature from thermocouple
    //sensors.requestTemperatures();
    //temp1 = sensors.getTempCByIndex(0);   // get ambient temperature

    if (reflowing) {
      // pre heating stage
      if (temp < preHeat)
        tft.drawRect(0, 90, 240, 35, RED);

      // reflow stage
      if (temp > preHeat) {
        tft.drawRect(0, 90, 240, 35, BLACK);
        tft.drawRect(0, 125, 240, 35, GREEN);

        // turn off the heating element if temp exceeds set reflow temp
        if (temp > reflow) {
          digitalWrite(relay, LOW);
        }
        // turn it back on otherwise
        if (temp < reflow) {
          digitalWrite(relay, HIGH);
        }
        // countdown timer
        timer--;
        // timer finished
        if (timer == 0) {
          reflowing = false;        // turn off reflow
          cooling = true;           // turn on cooling
          digitalWrite(relay, LOW); // turn off relay
          digitalWrite(buzzer, HIGH); // turn off buzzer
          delay(500);
          digitalWrite(buzzer, LOW); // turn off buzzer
          tft.drawRect(0, 125, 240, 35, BLACK);
        }
        mm = timer / 60;
        ss = timer % 60;
        tft.fillRect(130, 203, 50, 20, BLACK);
        tft.fillRect(30, 233, 120, 40, BLACK);
      }
    }
    tft.fillRoundRect(115, 40, 115, 41, 15, RED);
  }

  if (cooling) {
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    tft.pushColors((const uint8_t*)fan1, w * h, 1, false);
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
    delay(50);
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    tft.pushColors((const uint8_t*)fan2, w * h, 1, false);
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
    delay(50);
    if (temp > coolDown) {
      digitalWrite(fan, HIGH);
      tft.drawRect(0, 160, 240, 35, BLUE);
    }
    if (temp < coolDown) {
      cooling = false;
      timer = saveTimer;
      digitalWrite(fan, LOW);
      tft.drawRect(0, 160, 240, 35, BLACK);
      tft.fillRect(130, 203, 50, 20, BLACK);
    }
  }

  bool down = Touch_getXY();

  // Start | Reset
  on_btn.press(down && on_btn.contains(pixel_x, pixel_y));
  off_btn.press(down && off_btn.contains(pixel_x, pixel_y));

  if (on_btn.justPressed())  on_btn.drawButton(true);
  if (off_btn.justPressed()) off_btn.drawButton(true);
  if (on_btn.justReleased()) {
    on_btn.drawButton();
    if (timer > 30) {
      saveTimer = timer;
      reflowing = true;
      mm = timer / 60;
      ss = timer % 60;
      tft.fillRect(130, 203, 50, 20, BLACK);
      tft.fillRect(30, 233, 120, 40, BLACK);
      digitalWrite(relay, HIGH);
    }
  }
  if (off_btn.justReleased()) {
    off_btn.drawButton();
    asm volatile ("jmp 0");   // reset
  }
  // PreHeat
  up1.press(down && up1.contains(pixel_x, pixel_y));
  dwn1.press(down && dwn1.contains(pixel_x, pixel_y));

  if (up1.justPressed())  up1.drawButton(true);
  if (dwn1.justPressed()) dwn1.drawButton(true);
  if (up1.justReleased()) {
    up1.drawButton();
    tft.fillRect(130, 98, 50, 20, BLACK);
    preHeat++;
  }
  if (dwn1.justReleased()) {
    dwn1.drawButton();
    tft.fillRect(130, 98, 50, 20, BLACK);
    preHeat--;
  }
  // Reflow
  up2.press(down && up2.contains(pixel_x, pixel_y));
  dwn2.press(down && dwn2.contains(pixel_x, pixel_y));

  if (up2.justPressed())  up2.drawButton(true);
  if (dwn2.justPressed()) dwn2.drawButton(true);
  if (up2.justReleased()) {
    up2.drawButton();
    tft.fillRect(130, 133, 50, 20, BLACK);
    reflow++;
  }
  if (dwn2.justReleased()) {
    dwn2.drawButton();
    tft.fillRect(130, 133, 50, 20, BLACK);
    reflow--;
  }
  // cooldown
  up3.press(down && up3.contains(pixel_x, pixel_y));
  dwn3.press(down && dwn3.contains(pixel_x, pixel_y));

  if (up3.justPressed())  up3.drawButton(true);
  if (dwn3.justPressed()) dwn3.drawButton(true);
  if (up3.justReleased()) {
    up3.drawButton();
    tft.fillRect(130, 168, 50, 20, BLACK);
    coolDown++;
  }
  if (dwn3.justReleased()) {
    dwn3.drawButton();
    tft.fillRect(130, 168, 50, 20, BLACK);
    coolDown--;
  }
  // set timer
  up4.press(down && up4.contains(pixel_x, pixel_y));
  dwn4.press(down && dwn4.contains(pixel_x, pixel_y));

  if (up4.justPressed())  up4.drawButton(true);
  if (dwn4.justPressed()) dwn4.drawButton(true);
  if (up4.justReleased()) {
    up4.drawButton();
    tft.fillRect(130, 203, 50, 20, BLACK);
    timer += 5;
  }
  if (dwn4.justReleased()) {
    dwn4.drawButton();
    tft.fillRect(130, 203, 50, 20, BLACK);
    timer -= 5;
  }
  // fan
  fanBtn.press(down && fanBtn.contains(pixel_x, pixel_y));

  if (fanBtn.justPressed())  fanBtn.drawButton(true);
  if (fanBtn.justReleased()) {
    fanBtn.drawButton();
    fanState = !fanState;
  }

  if (fanState) {
    digitalWrite(fan, HIGH);
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    tft.pushColors((const uint8_t*)fan1, w * h, 1, false);
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
    delay(50);
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    tft.pushColors((const uint8_t*)fan2, w * h, 1, false);
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
    delay(50);
  }
  else {
    digitalWrite(fan, LOW);
  }

  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.setCursor(5, 50);
  tft.print("Temp : ");
  tft.setTextColor(WHITE);
  tft.print(temp);
  tft.print((char)247);
  tft.print("C");
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 100);
  tft.print("Pre Heat : ");
  tft.print(preHeat);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 135);
  tft.print("Reflow   : ");
  tft.print(reflow);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 170);
  tft.print("Cooldown : ");
  tft.print(coolDown);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 205);
  tft.print("Timer(s) : ");
  tft.print(timer);

  tft.setTextSize(4);
  tft.setTextColor(WHITE);
  tft.setCursor(30, 235);
  tft.print(mm);
  tft.print(":");
  if (ss < 10) tft.print(0);
  tft.print(ss);
  tft.setTextSize(2);
}
