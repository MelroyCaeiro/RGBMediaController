#include <BleKeyboard.h>
#include <FastLED.h>

#define LED_PIN     13

BleKeyboard bleKeyboard("ESP32BLE");

// Information about the LED strip itself
#define NUM_LEDS    8
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define BRIGHTNESS  40

#define CLK 5
#define DATA 21
#define CL_SW 26

int count = 0;

int SwitchState = 0;
long TimeOfLastDebounce = 0;
long DelayOfDebounce = 300;

void IRAM_ATTR detectSwitch() {

    if(SwitchState == 0){

        if((millis() - TimeOfLastDebounce) > DelayOfDebounce){
            SwitchState = 1;
            TimeOfLastDebounce = millis();
            Serial.println("Clicked");
            bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
        }
    }
    else{
        if(SwitchState == 1){
            SwitchState = 0;
        }
    }

}

void setup() {
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  pinMode(CL_SW, INPUT);
  pinMode(CL_SW, INPUT_PULLUP);
  Serial.begin (115200);
  Serial.println("KY-040 Start:");
  bleKeyboard.begin();
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
}

static uint8_t prevNextCode = 0;
static uint16_t store=0;

void loop() {
static int8_t c,val;

   if( val=read_rotary() ) {
      c +=val;
      Serial.print(c);Serial.print(" ");

      if ( prevNextCode==0x0b) {
         Serial.print("eleven ");
         Serial.println(store,HEX);
         fill_solid(leds, NUM_LEDS, CHSV(c*30,255,255));
         FastLED.show();

         if(bleKeyboard.isConnected()){
            bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
         }
         
         //delay(1000);

      }

      if ( prevNextCode==0x07) {
         Serial.print("seven ");
         Serial.println(store,HEX);
         fill_solid(leds, NUM_LEDS, CHSV(c*30,255,255));
         FastLED.show();

         if(bleKeyboard.isConnected()){
            bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
         }
         
         //delay(1000);
      }
   }

   attachInterrupt(CL_SW, detectSwitch, FALLING);
   

}

// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prevNextCode <<= 2;
  if (digitalRead(DATA)) prevNextCode |= 0x02;
  if (digitalRead(CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

   // If valid then store as 16 bit data.
   if  (rot_enc_table[prevNextCode] ) {
      store <<= 4;
      store |= prevNextCode;
      //if (store==0xd42b) return 1;
      //if (store==0xe817) return -1;
      if ((store&0xff)==0x2b) return -1;
      if ((store&0xff)==0x17) return 1;
   }
   return 0;
}
