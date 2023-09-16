#include <arduinoFFT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

//TFT DISPLAY
#define BUTTON_FORWARD 1
#define BUTTON_BACKWARD 2
// #define BUTTON_FFT 0
#define debounce 220
//-----------------TFT-------------------------------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 135;
const int METER_HEIGHT = SCREEN_HEIGHT / 2;

//----------------FFT----------------------------
#define SAMPLES 128
#define SAMPLING_FREQUENCY 659.26 // Nyquist frequency = 2 * E4 = double highest frequency

unsigned int samplingPeriod;
unsigned long microSeconds;

double vReal[SAMPLES];
double vImag[SAMPLES];

arduinoFFT FFT = arduinoFFT();

struct Note {
  const char* name;
  double baseFrequency;
};

Note notes[] = {
  {"C", 16.35},
  {"C#", 17.32},
  {"D", 18.35},
  {"D#", 19.45},
  {"E", 20.60},
  {"F", 21.83},
  {"F#", 23.12},
  {"G", 24.50},
  {"G#", 25.96},
  {"A", 27.50},
  {"A#", 29.14},
  {"B", 30.87}
};

double prevNoteFrequency = 0;
double noteFrequency = 0;
double nextNoteFrequency = 0;
double usedInterval = 0;
int noteIndex = 4;
int octave = 2;


void setup() {
  Serial.begin(115200);

  //D1 and D2 Button on Feather S2 Reverse TFT init
  pinMode(BUTTON_FORWARD, INPUT_PULLUP); 
  pinMode(BUTTON_BACKWARD, INPUT_PULLUP); 

  //turn on TFT Backlight
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  //turn on TFT
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  tft.init(135, 240); 
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY));

  drawRuler(SCREEN_WIDTH); // Use full width for the ruler
  displayNote();
  
  
}

void loop() {
  //Run FFT every 500ms --> draw output, check Button presses --> redraw notes

  static unsigned long previousTime = 0;  // Store the previous time
  unsigned long currentTime = millis();    // Get the current time
  unsigned long interval = 500;

  if (currentTime - previousTime >= interval) {
    
    for (int i = 0; i < SAMPLES; i++) {
    microSeconds = micros();
    vReal[i] = analogRead(A1);
    vImag[i] = 0;

    while (micros() < (microSeconds + samplingPeriod)) {
      // Wait for sampling period
      }
    } 

    // Perform FFT on samples
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_BLACKMAN, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    // Find peak frequency
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

    // Print the peak frequency to the Serial Monitor
    Serial.print("Peak frequency: ");
    Serial.println(peak);

    //draw the peak as red  line
    drawInputLine(peak, 12, usedInterval);
  
    // Update the previous time to the current time
    previousTime = currentTime;
  }

  checkButtonPress();
  
}

void displayNote() {

  //tft set colors, font and cursor
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(SCREEN_WIDTH/2 - 20, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);

  //print the selected note 
  tft.print(notes[noteIndex].name);
  tft.print(octave);
  tft.setTextSize(2);
  drawRuler(SCREEN_WIDTH); // Use full width for the ruler
}

void drawRuler(int screenWidth) {
  //draws 6 short lines, one long line, and 6 short lines

  int intervalWidth = screenWidth / 12; // 12 notes
  int centerX = screenWidth / 2;

  for (int i = 0; i < 12; i++) {
    int x = i * intervalWidth;
    int lineHeight = (i == 6) ? METER_HEIGHT : METER_HEIGHT / 2;

    tft.drawFastVLine(x, SCREEN_HEIGHT - lineHeight, lineHeight, ST77XX_WHITE);
  }
}

void drawInputLine(double inputValue, int intervals, int usedInterval) {

  int rulerWidth = SCREEN_WIDTH;
  double originalMin = usedInterval/2;
  double originalMax = noteFrequency + usedInterval/2;

  int inputX = map(inputValue, prevNoteFrequency, nextNoteFrequency, 0, rulerWidth);
  
  //clear screen after drawing
  tft.fillScreen(ST77XX_BLACK);

  //redraw note and ruler
  displayNote();

  // Draw the input line
  tft.drawFastVLine(inputX, 0, METER_HEIGHT, ST77XX_RED);
  tft.drawFastVLine(inputX + 1, 0, METER_HEIGHT, ST77XX_RED);
}

void checkButtonPress() {
  prevNoteFrequency = notes[noteIndex - 1].baseFrequency * pow(2.0, octave);
  noteFrequency = notes[noteIndex].baseFrequency * pow(2.0, octave); // Calculate note frequency based on octave
  nextNoteFrequency = notes[(noteIndex + 1) % 12].baseFrequency * pow(2.0, octave); // Calculate frequency of the next note
  usedInterval = nextNoteFrequency - noteFrequency;

  if (digitalRead(BUTTON_FORWARD) == LOW) {
    
    noteIndex = (noteIndex + 1) % 12;
    if (noteIndex == 0) {
      octave++;
    }
    displayNote();
    delay(debounce); // Debounce delay
  }

  if (digitalRead(BUTTON_BACKWARD) == LOW) {
      noteIndex = (noteIndex - 1 + 12) % 12;
      if (noteIndex == 11) {
        octave--;
      }
      displayNote();
      delay(debounce); // Debounce delay
    
  }

}

