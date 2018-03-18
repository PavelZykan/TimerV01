#include <Wire.h> 
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidMenu.h>
#include <buttons.h>

#define swVersion "1.0"
#define buzzerPin 3
#define settingsAddress 0
#define buzzerToneLength 500
#define elapsedTimeMenuUpdateDelay 200

struct BuzzerSettings {
  unsigned short timerDelayMin;
  unsigned short timerDelayMax;
};

bool countdownRunning = false;
unsigned long countdownStart = 0L;
unsigned int buzzerDelay;
bool buzzerRunning = false;
bool startStopModeRunning = false;
unsigned long buzzerStarted = 0L;
float elapsedTime = 0.0f;
unsigned long lastElapsedTimeMenuUpdate = 0L;
char* secondsConstant;

BuzzerSettings currentSettings;

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Set the LCD I2C address

LiquidLine welcome_line1(0, 0, "ArTimer ", swVersion);
LiquidLine welcome_line2(0, 1, "Run: UP or DOWN");

LiquidScreen welcome_screen(welcome_line1, welcome_line2);

LiquidLine startStopLine1(0, 0, "Start/Stop mode");
LiquidLine startStopLine2(0, 1, "Time: ", elapsedTime, secondsConstant);
LiquidScreen startStopScreen(startStopLine1, startStopLine2);

LiquidLine timerDelayMinLine1(0, 0, "Timer delay");
LiquidLine timerDelayMinLine2(0, 1, "Min: ", currentSettings.timerDelayMin);
LiquidScreen timerDelayMinScreen(timerDelayMinLine1, timerDelayMinLine2);

LiquidLine timerDelayMaxLine1(0, 0, "Timer delay ");
LiquidLine timerDelayMaxLine2(0, 1, "Max: ", currentSettings.timerDelayMax);
LiquidScreen timerDelayMaxScreen(timerDelayMaxLine1, timerDelayMaxLine2);

LiquidMenu menu(lcd);

Button left(OneShot);
Button right(OneShot);
Button up(OneShot);
Button down(OneShot);
Button start(OneShot);

void setup() {
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight 

  pinMode(buzzerPin, OUTPUT);
  
  setupMenu();
  
  setupButtons();

  setupSettings();

  startPressed = 0L;
}

void setupSettings() {
  EEPROM.get(settingsAddress, currentSettings);  

  if (currentSettings.timerDelayMin == 0 || currentSettings.timerDelayMin == 65535) {
    currentSettings.timerDelayMin = 4;
  }
  
  if (currentSettings.timerDelayMax == 0 || currentSettings.timerDelayMax == 65535) {
    currentSettings.timerDelayMax = 6;
  }
}

void saveSettings() {
  EEPROM.put(settingsAddress, currentSettings);
}

void setupMenu() {
  // This methid needs to be called when using an I2C display library.
  menu.init();

  menu.add_screen(welcome_screen);
  menu.add_screen(timerDelayMinScreen);
  menu.add_screen(timerDelayMaxScreen);

  timerDelayMinLine1.attach_function(1, decMinDelay);
  timerDelayMinLine1.attach_function(2, incMinDelay);

  timerDelayMaxLine1.attach_function(1, decMaxDelay);
  timerDelayMaxLine1.attach_function(2, incMaxDelay);

   menu.update();
}

void incMinDelay() {
  if (currentSettings.timerDelayMin < 30 && currentSettings.timerDelayMin < currentSettings.timerDelayMax) {
    currentSettings.timerDelayMin++;
    saveSettings();
    menu.update();
  }
}

void decMinDelay() {
  if (currentSettings.timerDelayMin > 0) {
    currentSettings.timerDelayMin--;
    saveSettings();
    menu.update();
  }
}

void incMaxDelay() {
  if (currentSettings.timerDelayMax < 30) {
    currentSettings.timerDelayMax++;
    saveSettings();
    menu.update();
  }
}

void decMaxDelay() {
  if (currentSettings.timerDelayMax > 0 && currentSettings.timerDelayMax > currentSettings.timerDelayMin) {
    currentSettings.timerDelayMax--;
    saveSettings();
    menu.update();
  }
}

void setupButtons() {
  start.assign(6);
  start.turnOnPullUp();
  start.check();
  left.assign(7);
  left.turnOnPullUp();
  left.check();
  up.assign(9);
  up.turnOnPullUp();
  up.check();
  down.assign(8);
  down.turnOnPullUp();
  down.check();
  right.assign(10);
  right.turnOnPullUp();  
  right.check();
}


void loop() {
  if (left.check()) {
    menu.previous_screen();
  } else if (right.check()) {
    menu.next_screen();
  } else if (up.check()) {
    menu.call_function(0, 2);
  } else if (down.check()) {
    menu.call_function(0, 1);
  } else if (start.check()) {
    startPressed = millis();
    buzzerDelay = random(currentSettings.timerDelayMin * 1000, currentSettings.timerDelayMax * 1000);
  } else if (startPressed > 0) {
    unsigned long timeDiff = millis() - startPressed;
    if (timeDiff >= buzzerDelay) {
      startPressed = 0L;
      tone(buzzerPin, 1000); 
      delay(1000);
      noTone(buzzerPin);
    }
  }
}
