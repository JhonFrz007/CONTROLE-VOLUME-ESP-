#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 15
#define SCL_PIN 4
#define POT_PIN 34

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float leituraSuave = 0;

int micPeak = 0;
unsigned long micPeakTime = 0;

int cenaPeak = 0;
unsigned long cenaPeakTime = 0;

const unsigned long PEAK_HOLD = 1200;

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  display.clearDisplay();
  display.display();
}

void loop() {

  long soma = 0;
  for (int i = 0; i < 8; i++) {
    soma += analogRead(POT_PIN);
  }

  int leitura = soma / 8;
  leituraSuave = leituraSuave * 0.8 + leitura * 0.2;

  int mic = map((int)leituraSuave, 0, 4095, 100, 0);
  mic = constrain(mic, 0, 100);

  int cena = 100 - mic;

  // peaks MIC
  if (mic > micPeak) {
    micPeak = mic;
    micPeakTime = millis();
  } else if (millis() - micPeakTime > PEAK_HOLD) {
    micPeak = mic;
  }

  // peaks CENA
  if (cena > cenaPeak) {
    cenaPeak = cena;
    cenaPeakTime = millis();
  } else if (millis() - cenaPeakTime > PEAK_HOLD) {
    cenaPeak = cena;
  }

  // ===== OLED =====
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(35, 0);
  display.print("MIXER OBS");

  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  drawChannel(10, 15, "MIC", mic, micPeak);
  drawChannel(70, 15, "CENA", cena, cenaPeak);

  display.display();

  // ===== ENVIO CORRETO PARA PYTHON =====
  Serial.print("MIC:");
  Serial.print(mic);
  Serial.print(" CENA:");
  Serial.println(cena);

  delay(10);
}

void drawChannel(int x, int y, const char* label, int value, int peak) {

  display.setTextSize(1);
  display.setCursor(x, y);
  display.print(label);

  display.setTextSize(2);
  display.setCursor(x, y + 10);

  if (value < 10) display.print(" ");
  display.print(value);
  display.print("%");

  int dots = map(value, 0, 100, 0, 10);

  for (int i = 0; i < 10; i++) {
    int dx = x + (i * 4);
    int dy = y + 32;

    if (i < dots) {
      display.fillRect(dx, dy, 2, 4, SSD1306_WHITE);
    } else {
      display.drawRect(dx, dy, 2, 4, SSD1306_WHITE);
    }
  }

  int peakPos = map(peak, 0, 100, 0, 40);
  display.drawPixel(x + peakPos, y + 28, SSD1306_WHITE);
}