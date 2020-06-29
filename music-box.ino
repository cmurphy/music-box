#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_NeoPixel.h>

AudioPlaySdWav           playSdWav1;     //xy=235,227
AudioAnalyzePeak         peak1;          //xy=423,268
AudioAnalyzePeak         peak2;          //xy=425,314
AudioMixer4              mixer1;         //xy=563,221
AudioOutputI2S           i2s1;           //xy=790,186
AudioConnection          patchCord1(playSdWav1, 0, peak1, 0);
AudioConnection          patchCord2(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord3(playSdWav1, 1, peak2, 0);
AudioConnection          patchCord4(playSdWav1, 1, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=290,410

#define PIN1 24
#define PIN2 30
#define NUMPIXELS 8
#define DELAYVAL 500

Adafruit_NeoPixel pixelsL(NUMPIXELS, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsR(NUMPIXELS, PIN2, NEO_GRB + NEO_KHZ800);

// Teensy Audio Shield pins
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

char songfile[12];

void setup() {
  Serial.begin(9600);
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.7f);
  mixer1.gain(0, 0.7f);
  mixer1.gain(1, 0.0f);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  delay(50);
  File root = SD.open("/");
  while(true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    String filename = entry.name();
    if (filename.endsWith(".WAV") || filename.endsWith(".wav")) {
        filename.toCharArray(songfile, filename.length()+1);
    }
    entry.close();
  }

  pixelsL.begin();
  pixelsR.begin();
}

// for best effect make your terminal/monitor a minimum of 62 chars wide and as high as you can.

elapsedMillis msecs;
int colors[][3] = {
  {75, 0, 130},
  {148, 0, 211},
  {0, 0, 225},
  {0, 110, 110},
  {0, 100, 0},
  {225, 200, 0},
  {225, 69, 0},
  {225, 0, 0}
};

void loop() {
  if (playSdWav1.isPlaying() == false) {
    Serial.printf("Playing %s\n", songfile);
    playSdWav1.play(songfile);
    delay(10); // wait for library to parse WAV info
  }

  pixelsL.clear();
  pixelsR.clear();

  if (msecs > 40) {
    if (peak1.available() && peak2.available()) {
      msecs = 0;
      float leftNumber = peak1.read();
      float rightNumber = peak2.read();
      int leftPeak = leftNumber * 8.0;
      int rightPeak = rightNumber * 8.0;
      int count;
      for (count=0; count < leftPeak; count++) {
        pixelsL.setPixelColor(count, pixelsL.Color(colors[count][0], colors[count][1], colors[count][2]));
      }
      for (count=0; count < rightPeak; count++) {
        pixelsR.setPixelColor(count, pixelsL.Color(colors[count][0], colors[count][1], colors[count][2]));
      }
      pixelsL.show();
      pixelsR.show();
    }
  }
}
