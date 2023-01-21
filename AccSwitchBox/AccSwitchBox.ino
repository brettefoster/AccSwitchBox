#include <Keyboard.h>

// Set characters
static uint8_t ctrlLeftKey = KEY_LEFT_CTRL;
static uint8_t altLeftKey = KEY_LEFT_ALT;
static uint8_t shiftLeftKey = KEY_LEFT_SHIFT;
static uint8_t ctrlRightKey = KEY_RIGHT_CTRL;
static uint8_t altRightKey = KEY_RIGHT_ALT;
static uint8_t shiftRightKey = KEY_RIGHT_SHIFT;

// Set analog pin inputs
const int dial_01_Pin = A3;
const int dial_02_Pin = A2;
const int dial_03_Pin = A1;

// Set digital pin inputs
const int button_01_Pin = 2;
const int button_02_Pin = 3;
const int button_03_Pin = 4;
const int button_04_Pin = 5;

// Set global variables for persiting last known value
bool button_01_Current = false;
bool button_02_Current = false;
bool button_03_Current = false;
bool button_04_Current = false;
int dail_01_Current = 0;
int dail_02_Current = 0;
int dail_03_Current = 0;

// Set global variable for forcing a resend of current values.
bool batchTrigger = false;

// Rotary switch input class
// The hardware for this input is a 10 position, single pole rotary switch with 1k ohm resistors 
// connecting each position, in sequence. The effect is potentiometer-like device with 1k ohm additive 
// resistance applied as the dial is turned. This resulting output is ready by the microcontroller 
// as stepped analong values that are easy to distinguish for interpreting the swith position.

class Dial {
  private:
    int pin;
    int special;
    uint8_t specialValue;
    String name;
    int value;
    int newValue;
    
  public:
    Dial(int setPin, int setSpecial, String setName) {
      pin = setPin;
      special = setSpecial;
        // 0 = nothing
        // 1 = ctrl left
        // 2 = alt left
        // 3 = shift left
      name = setName;
      value = 0;
      newValue = 0;

      // Set if the dial value submission will include a special key combination
      // For this project, each dial defaults to key presses of 1-0 on the keyboard
      // and the special configuration determines if Ctrl/Alt/Shift is combined with 
      // the key press.
      // e.g. "Ctrl-9"
      if (special > 0) {
        if (special == 1) {
          specialValue = ctrlLeftKey;
        } else if (special == 2) {
          specialValue = altLeftKey;
        } else if (special == 3) {
          specialValue = shiftLeftKey;
        } else if (special == 4) {
          specialValue = ctrlRightKey;
        } else if (special == 5) {
          specialValue = altRightKey;
        } else if (special == 6) {
          specialValue = shiftRightKey;
        }
      }
    }

    // Keyboard command is sent to the computer. 
    void Publish() {
      char publishValue = (value == 10) ? '0' : value + '0';
      //Serial.print("Publish dial "); 
      //Serial.println(publishValue);
      if (special > 0) {
        Keyboard.press(specialValue);
      }
      Keyboard.press(publishValue);
      delay(100);
      Keyboard.releaseAll(); 
    }

    // Input is compared to last known value and publish action is determined. 
    void CheckValue() {
      // The arduino analog pins measure input between 0v and 5v which is represented
      // as values 0 to 1023. The following equasion adjusts and rounds out the stepped analog 
      // values of the rotary dail to values of 1-10.
      newValue = round(((analogRead(pin)+100)*.9)/100);
      if (newValue != value) {
        value = newValue;
        this->Publish();
        //Serial.print("Dail event ");
        //Serial.print(name);
        //Serial.print(" ");
        //Serial.println(value);

        // This was added to address input bouncing
        delay(100);
      }
    }

    // Serial output is disabled when not debugging (see setup function). 
    void TestRawValue() {
      //Serial.println(analogRead(pin));
    }

    void TestRoundValue() {
      //Serial.println(round(((analogRead(pin)+100)*.9)/100));
    }
  
};


// Simple button input class for INPUT_PULLUP pins. 
class Button {
  private:
    int pin;
    char value;
    int special;
    uint8_t specialValue;
    String name;
    bool currentState;
    bool newState;

  public:
    Button(int setPin, char setValue, int setSpecial, String setName) {
      pin = setPin;
      value = setValue;
      special = setSpecial;
        // 0 = nothing
        // 1 = ctrl left
        // 2 = alt left
        // 3 = shift left
        // 100 = custom function 1
      name = setName;
      currentState = false;
      newState = false;

      // Set if the button press will include a special key combination
      // e.g. "Ctrl-x"
      if (special > 0) {
        if (special == 1) {
          specialValue = ctrlLeftKey;
        } else if (special == 2) {
          specialValue = altLeftKey;
        } else if (special == 3) {
          specialValue = shiftLeftKey;
        } else if (special == 4) {
          specialValue = ctrlRightKey;
        } else if (special == 5) {
          specialValue = altRightKey;
        } else if (special == 6) {
          specialValue = shiftRightKey;
        }
      }
    }

    void Publish() {
      if (special > 0) {
        Keyboard.press(specialValue);
      }
      Keyboard.press(value);
      delay(100);
      Keyboard.releaseAll(); 
    }

    void CheckState() {
      newState = (digitalRead(pin) == LOW) ? true : false;

      // The following logic ensures only the initial button press detection is
      // processed. Holding the button in will be ignored until released. 
      if (newState != currentState) {
        if (newState == true) {
          currentState = true;

          if (special == 100) {
            batchTrigger = true; // update the global value to be picked up by an external funciton
          } else {
            this->Publish();
            //Serial.print("Button pressed ");
            //Serial.println(name);
            delay(100);
          }
        }
        else {
          currentState = false;
          delay(100);
        }
      }
    }
};

// BOX SETUP
Button button_01 = Button(button_01_Pin, 'i', 0, "button 1");
Button button_02 = Button(button_02_Pin, 's', 0, "button 2");
Button button_03 = Button(button_03_Pin, 'c', 0, "button 3");
Button button_04 = Button(button_04_Pin, 'x', 100, "button 4");
Dial dial_01 = Dial(dial_01_Pin, 1, "dial 1");
Dial dial_02 = Dial(dial_02_Pin, 2, "dial 2");
Dial dial_03 = Dial(dial_03_Pin, 3, "dial 3");

void batchPublish() {
  //Serial.println("Batch publish executing");
  dial_01.Publish();
  dial_02.Publish();
  dial_03.Publish();
}

void setup() {
  analogReference(DEFAULT); 
  Keyboard.begin();
  //Serial.begin(9600);
  //Serial.println('Setup completed');

  // INPUT_PULLUP utilizes internal resistors allowing for a simplified 
  // button curcuit design. 
  pinMode(button_01_Pin, INPUT_PULLUP);
  pinMode(button_02_Pin, INPUT_PULLUP);
  pinMode(button_03_Pin, INPUT_PULLUP);
  pinMode(button_04_Pin, INPUT_PULLUP);
}

void loop() {
  button_01.CheckState();
  button_02.CheckState();
  button_03.CheckState();
  button_04.CheckState();

  dial_01.CheckValue();
  dial_02.CheckValue();
  dial_03.CheckValue();

  // Test methods for analog input smoothing
  //dial_01.TestRawValue();
  //dial_01.TestRoundValue();

  // Function for acting on batch button trigger.
  if (batchTrigger == true) {
    batchPublish();
    batchTrigger = false;
  }
}
