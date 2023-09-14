#include <SPI.h>
#include "ER-EPM042-1B.h"
#include "epdpaint.h"

#define WHITE 1
#define BLACK 0

// 1cm ≈ 48px
// font size, ~ characters per line: 8,80  12,57  16,36  20,28  24,23

// button prompts
#define POWER_BUTTON  btn_prompt(80, 80 - 32, "STANDBY");
#define VENT_BUTTON   btn_prompt(160, 160 - 13, "VENT");
#define PUMP_BUTTON   btn_prompt(240, 240 - 13, "PUMP");
#define HEAT_BUTTON   btn_prompt(320, 320 - 13, "HEAT");

// buttons shown on display
bool power_button_shown = false;
bool vent_button_shown = false;
bool pump_button_shown = false;
bool heat_button_shown = false;

// PINS
/// sensors
const int vacuumSensorPin          = A15;
const int heatingIndicatorAndRelay = 48;
const int heatingTimerSensorPin    = 47; // 50 -> 47 because interference with display
const int temperatureSensorPin     = 49; // 52 -> 49 because interference with display
/// pumps and their indicator LEDs
const int turboPumpPower           = 42; // toggle power to the relay, which toggles power to the turbo (fire - spin up, fire again - spin down)
const int turboPumpStatusIndicator = 44; // turbo status LED
const int scrollPumpRelay          = 46; // relay and on LED
/// valves
///   OPEN: hold on, trigger on, trigger off
///  CLOSE: hold off
const int valve_A_trigger = 26;
const int valve_A_hold    = 28;
const int valve_B_trigger = 30;
const int valve_B_hold    = 32;
const int valve_C_trigger = 34;
const int valve_C_hold    = 36;
const int valve_D_trigger = 38;
const int valve_D_hold    = 40;
/// switches: HIGH by default, LOW when pressed
const int switch_power = 7;
const int switch_vent  = 6;
const int switch_pump  = 5;
const int switch_heat  = 4;

int vacuumSensorValue; // create a blank variable to hold the value from the Vacuum Sensor Pin that can be assigned a value later
// voltage is vacuumSensorValue * (5.0 / 1023.0)

// once pump() has run, the turbo won't turn off unless the machine is turned off
// therefore, we need to prevent further triggers, which would turn the pump off again
bool turboPumpOn = false;

Epd epd; // create an epd object to interact with the display

// create canvases to put objects on (memory size in bytes, width in pixels (multiple of 8), height in pixels)
Paint title_canvas (1700, 8*50, 37);
Paint info_canvas  (1700, 8*50, 11);
Paint timer_canvas (2000, 8*35, 40);
Paint btn_canvas   (2000, 8*50, 41);

// compacted functions to avoid duplication/redundancy/repitition and to save space
void ClearTitleCanvas() {title_canvas.Clear(WHITE);}
void PlaceTitleCanvas(int y_pos) {epd.SetPartialWindow(title_canvas.GetImage(), 0, y_pos, title_canvas.GetWidth(), title_canvas.GetHeight());}
void ClearInfoCanvas() {info_canvas.Clear(WHITE);}
void PlaceInfoCanvas(int y_pos) {epd.SetPartialWindow(info_canvas.GetImage(), 0, y_pos, info_canvas.GetWidth(), info_canvas.GetHeight());}
void ClearTimerCanvas() {timer_canvas.Clear(WHITE);}
void PlaceTimerCanvas(int y_pos) {epd.SetPartialWindow(timer_canvas.GetImage(), 0, y_pos, timer_canvas.GetWidth(), timer_canvas.GetHeight());}
void ClearBtnCanvas() {btn_canvas.Clear(WHITE);}
void PlaceBtnCanvas(int y_pos) {epd.SetPartialWindow(btn_canvas.GetImage(), 0, y_pos, btn_canvas.GetWidth(), btn_canvas.GetHeight());}

void btn_prompt(int arrow_x_offset, int x_pos, char message[], int x_pos_line2 = 0, char message_line2[] = "") {
  if (message_line2 == "") {btn_canvas.DrawStringAt(x_pos, 12, message, &Font16, BLACK);}
  else {
    btn_canvas.DrawStringAt(x_pos, 0, message, &Font16, BLACK);
    btn_canvas.DrawStringAt(x_pos_line2, 12, message_line2, &Font16, BLACK);
  }
  btn_canvas.DrawStringAt(arrow_x_offset, 24, "V", &Font24, BLACK);
}
void reset_btns() {
  power_button_shown = false;
  vent_button_shown = false;
  pump_button_shown = false;
  heat_button_shown = false;
}
void initPinOutput(int pin, bool startState) {pinMode(pin, OUTPUT); digitalWrite(pin, startState);} // compact way to set a pin as an output and its starting state (ONLY FOR DIGITAL OUTPUT)
bool inRange(int minimum, int value, int maximum) {return ((value >= minimum) && (value < maximum));} // return a boolean if a value is within a range

void ClearDisplay() {
  epd.ClearFrame(); // remove everything from the display

  ClearTitleCanvas();
  title_canvas.DrawStringAt(124, 0, "Auto-HENRY", &Font24, BLACK);
  title_canvas.DrawStringAt(120, 22, "By *** and ***", &Font12, BLACK);
  title_canvas.DrawHorizontalLine(0, 36, 400, BLACK);
  PlaceTitleCanvas(0);

  ClearBtnCanvas();
  if (power_button_shown) {POWER_BUTTON}
  if (vent_button_shown) {VENT_BUTTON}
  if (pump_button_shown) {PUMP_BUTTON}
  if (heat_button_shown) {HEAT_BUTTON}
  PlaceBtnCanvas(250);

  reset_btns();
}

// function to push a new message to the log - new messages are the bottom line and push previous messages upwards
#define start_string "                                                         " // max 57 chars per line for the info messages, this needs to be initalissed to the maximum length a message could be
// char previous_msg9[] = start_string;
// char previous_msg8[] = start_string;
// char previous_msg7[] = start_string;
// char previous_msg6[] = start_string;
// char previous_msg5[] = start_string;
char previous_msg4[] = start_string;
char previous_msg3[] = start_string;
char previous_msg2[] = start_string;
char previous_msg1[] = start_string;
void display(char message[]) {
  Serial.println(message);

  ClearDisplay();

  int y = 45; const int increment = 20;
  // ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg9, &Font12, BLACK); PlaceInfoCanvas(y);
  // ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg8, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  // ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg7, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  // ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg6, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  // ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg5, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg4, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg3, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg2, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, previous_msg1, &Font12, BLACK); PlaceInfoCanvas(y += increment);
  ClearInfoCanvas(); info_canvas.DrawStringAt(0, 0, message,       &Font12, BLACK); PlaceInfoCanvas(y += increment);

  // write to the display
  epd.DisplayFrame();

  // move each message up a line
  // memmove(TO, FROM, sizeof(TO))
  // memmove(previous_msg9, previous_msg8, sizeof(previous_msg9));
  // memmove(previous_msg8, previous_msg7, sizeof(previous_msg8));
  // memmove(previous_msg7, previous_msg6, sizeof(previous_msg7));
  // memmove(previous_msg6, previous_msg5, sizeof(previous_msg6));
  // memmove(previous_msg5, previous_msg4, sizeof(previous_msg5));
  memmove(previous_msg4, previous_msg3, sizeof(previous_msg4));
  memmove(previous_msg3, previous_msg2, sizeof(previous_msg3));
  memmove(previous_msg2, previous_msg1, sizeof(previous_msg2));
  memmove(previous_msg1, message, sizeof(previous_msg1));
}

// a loop that pauses the program until a certain value from the analog input has been reached
// pass HIGH or LOW as startState; HIGH means the value starts high and comes down to the targetReading, LOW means the value starts low and increases to the targetReading
void waitForVacuumSensorReading(int targetReading, bool startState, bool show_message = true) {
  if (show_message) {display("- Waiting for vacuum to reach the target ...");}
  while (true) {
    vacuumSensorValue = analogRead(vacuumSensorPin);
    if (startState == HIGH && vacuumSensorValue <= targetReading) {break;}
    if (startState == LOW && vacuumSensorValue >= targetReading) {break;}
    delay(100);
  }
}

void pulseTurboActivateSwitch() {digitalWrite(turboPumpPower, LOW); delay(500); digitalWrite(turboPumpPower, HIGH); delay(500); digitalWrite(turboPumpPower, LOW);} // for the turbo pump relay just toggle the button "HIGH,short delay,LOW" to simulate a button press

void openValve(int hold, int trigger) {
  digitalWrite(hold, HIGH);
  delay(1000);
  digitalWrite(trigger, HIGH);
  delay(1000);
  digitalWrite(trigger, LOW);
}

void closeValve(int pin) {digitalWrite(pin, LOW);}

void closeAllValves() {
  closeValve(valve_A_hold); closeValve(valve_A_trigger);
  closeValve(valve_B_hold); closeValve(valve_B_trigger);
  closeValve(valve_C_hold); closeValve(valve_C_trigger);
  closeValve(valve_D_hold); closeValve(valve_D_trigger);
}

// close all the valves and then turn off power to both pumps; the chamber stays at vacuum / air but the system is ready for the next person
void turnOff() {
  display("Turning Off Pumps ...");
  closeAllValves(); // close all valves
  digitalWrite(scrollPumpRelay, LOW); pulseTurboActivateSwitch(); digitalWrite(turboPumpStatusIndicator, LOW); // turn off both pumps
  Timer(); // let the turbo spool down
}

// handle the switching of the relay that handles power to the heating circuitry
void heat() {
  display("Heating");
  display("- Initialising Heating Module ...");
  digitalWrite(heatingIndicatorAndRelay, HIGH);

  // wait until the voltage from timer sensor is high (the heating timer is being set or finished being set and is turning on)
  display("- Waiting for heating timer to start ...");
  while (true) {if (digitalRead(heatingTimerSensorPin) == HIGH) {break;} delay(1000);}
  display("- Heating timer started");
  display("- Waiting for heating timer to stop ...");

  // wait until the voltage from timer sensor is low (the heating has finished)
  while (true) {if (digitalRead(heatingTimerSensorPin) == LOW) {break;} delay(10000);}
  display("- Heating finished");
  display("- Waiting for safe temperature (<30C) ...");

  // wait until thermal cutout is triggered (i.e. it is <=25ºC)
  while (true) {if (digitalRead(temperatureSensorPin) == HIGH) {break;} delay(10000);}
  display("- Temperature has reached a safe level (<30C)");
  display("- Ready to be vented");
  digitalWrite(heatingIndicatorAndRelay, LOW);

  // prompts the user to select a function: VENT
  int selectedFunction = 0;
  reset_btns();
  vent_button_shown = true;
  display("PLEASE SELECT A FUNCTION");
  while (true) {
    if (digitalRead(switch_vent) == LOW) {selectedFunction = 1; break;}
    delay(10);
  }
  if (selectedFunction == 1) {vent();}
}

// vent the chamber (vacuum -> atmosphere)
void vent() {
  display("Venting");
  closeAllValves();
  delay(2000);
  openValve(valve_D_hold, valve_D_trigger); // close valves, open D
  waitForVacuumSensorReading(950, LOW); // wait until the vacuum reaches a level low enough where the door is able to be opened
  closeAllValves(); // reset all valves back to closed
  display("- Finished Venting");

  // prompts the user to select a function: VENT or PUMP
  int selectedFunction = 0;
  reset_btns();
  pump_button_shown = true;
  display("PLEASE SELECT A FUNCTION");
  while (true) {
    if (digitalRead(switch_pump) == LOW) {selectedFunction = 1; break;}
    delay(10);
  }

  if (selectedFunction == 1) {pump();}
}

// pump the chamber (atmosphere -> vacuum)
void pump() {
  display("Pumping");
  closeAllValves(); // make sure all the valves are closed
  // open valves A and C
  openValve(valve_A_hold, valve_A_trigger);

  display("- Pumping to level 1 vacuum ...");
  digitalWrite(scrollPumpRelay, HIGH); // turn on the scroll pump

  waitForVacuumSensorReading(600, HIGH); // wait until the vacuum reaches a level low enough where the door is able to be opened
  openValve(valve_C_hold, valve_C_trigger);

  waitForVacuumSensorReading(600, HIGH, false);
  display("- Level 1 vacuum reached");
  display("- Waiting to pump to level 2 vacuum ...");

  // close A, open B, turn turbo relay on
  closeValve(valve_A_hold);
  openValve(valve_B_hold, valve_B_trigger);
  // if the turbo is already on, don't pulse the switch again otherwise it will turn off
  if (!turboPumpOn) {
    pulseTurboActivateSwitch();
    turboPumpOn = true;
  }

  // code to detect when the turbo is at the right speed (by measuring when it starts to pull a greater vacuum)
  while (analogRead(vacuumSensorPin) > 450) {
    digitalWrite(turboPumpStatusIndicator, HIGH); delay(300);
    digitalWrite(turboPumpStatusIndicator, LOW); delay(300);
  }
  digitalWrite(turboPumpStatusIndicator, HIGH); delay(300);
  display("- Turbo Pump At Sufficient Speed");
  display("- Pumping to level 2 vacuum ...");

  waitForVacuumSensorReading(100, HIGH);
  display("- Level 2 vacuum reached");

  // chamber is fully pumped to maximum vacuum, prompts the user to select another function: VENT or HEAT
  int selectedFunction = 0;
  reset_btns();
  vent_button_shown = true;
  heat_button_shown = true;
  power_button_shown = true;
  display("PLEASE SELECT A FUNCTION");
  while (true) {
    if (digitalRead(switch_vent) == LOW) {selectedFunction = 1; break;}
    if (digitalRead(switch_heat) == LOW) {selectedFunction = 2; break;}
    if (digitalRead(switch_power) == LOW) {selectedFunction = 3; break;}
    delay(100);
  }

  if (selectedFunction == 1) {vent();}
  if (selectedFunction == 2) {heat();}
  if (selectedFunction == 3) {turnOff();}
}

void ShowTimer(int mins, int secs) {
  String timer_string = ""; // create a string to hold the timer
  if (mins < 10) {timer_string += "0";} // if mins is 0-9, add a leading zero
  // add the minutes and a colon seperator
  timer_string += mins;
  timer_string += ":";
  if (secs < 10) {timer_string += "0";} // if mins is 0-9, add a leading zero
  timer_string += secs; // add the seconds
  char timer_char_array[6]; // create a character array to hold the converted string (6 because 2 numbers, colon, 2 numbers, null terminator)
  timer_string.toCharArray(timer_char_array, 6); // convert the string to a character array
  // display on the screen
  ClearDisplay();
  ClearTimerCanvas();
  timer_canvas.DrawStringAt(158, 0, timer_char_array, &Font24, BLACK);
  timer_canvas.DrawStringAt(151, 25, "Turbo Spindown", &Font12, BLACK);
  PlaceTimerCanvas(100);
  epd.DisplayFrame();
}

void Timer() {
  const float display_refresh_time = 5.5;
  const int update_interval = 10;
  int timer_mins = 15;
  int timer_secs = 0;
  while (timer_mins >= 0) {
    while (timer_secs >= 0) {
      delay(1000);
      Serial.print(timer_mins);
      Serial.print(":");
      Serial.println(timer_secs);
      if (timer_secs % update_interval == 0 || timer_secs == 0) {
        ShowTimer(timer_mins, timer_secs);
        timer_secs -= display_refresh_time;
      }
      timer_secs --;
    }
    timer_mins --;
    timer_secs = 59;
  }
}

// run this first, once, when the arduino recieves power or the reset button is pressed
void setup() {
  // check the display has initialised properly
  Serial.begin(9600);
  if (epd.Init() != 0) {
    // if it fails to detect or initialise the display, output an error message to serial monitor and hang in an empty infinite loop until the program is restarted
    Serial.println("Display initialisation failed");
    while (true) {}
  }
  // Serial.end();

  Serial.println("START");

  display("Starting ...");
  // Initialising I/O
  // Valves Hold
  initPinOutput(valve_A_hold, LOW);
  initPinOutput(valve_B_hold, LOW);
  initPinOutput(valve_C_hold, LOW);
  initPinOutput(valve_D_hold, LOW);
  // Valves Trigger
  initPinOutput(valve_A_trigger, LOW);
  initPinOutput(valve_B_trigger, LOW);
  initPinOutput(valve_C_trigger, LOW);
  initPinOutput(valve_D_trigger, LOW);
  // Heating Indicator
  initPinOutput(heatingIndicatorAndRelay, LOW);
  // Scroll pump and Turbo Pump
  initPinOutput(scrollPumpRelay, LOW);
  initPinOutput(scrollPumpOffIndicator, HIGH);
  initPinOutput(turboPumpStatusIndicator, LOW);
  initPinOutput(turboPumpPower, LOW);
  // Buttons
  pinMode(switch_power, INPUT);
  pinMode(switch_vent, INPUT);
  pinMode(switch_pump, INPUT);
  pinMode(switch_heat, INPUT);
  // Sensors
  pinMode(vacuumSensorPin, INPUT);
  pinMode(temperatureSensorPin, INPUT);
  pinMode(heatingTimerSensorPin, INPUT);
  // Done
  delay(200);
  display("Ready");
}

void loop() {
  // prompts the user to select a function: VENT or PUMP
  int selectedFunction = 0;
  reset_btns();
  pump_button_shown = true;
  vent_button_shown = true;
  display("PLEASE SELECT A FUNCTION");
  while (true) {
    if (digitalRead(switch_pump) == LOW) {selectedFunction = 1; break;}
    if (digitalRead(switch_vent) == LOW) {selectedFunction = 2; break;}
    delay(10);
  }

  if (selectedFunction == 1) {pump();}
  if (selectedFunction == 2) {vent();}
}