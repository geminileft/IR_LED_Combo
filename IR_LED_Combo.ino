#define HEADER_LEAD 9000
#define HEADER_TRAIL 4500
#define VALUE_THRESHOLD 750

#define VALUE_GOOD(orig, val) (abs(orig - val) < VALUE_THRESHOLD)

#define SIZE_OF_DATA 16
#define BAUD_RATE 115200
#define PIN2_INT 0
#define PIN3_INT 1
#define HEADER_PULSES 2
#define ADDRESS_PULSES 32
#define COMMAND_PULSES 32
#define END_PULSES 1
#define NUM_PULSES HEADER_PULSES + ADDRESS_PULSES + COMMAND_PULSES + END_PULSES
#define MAX_WAIT 15000

#define BTN_LEFT 8
#define BTN_RIGHT 10

#define BTN_1 16
#define LED_1_INDEX 0
#define BTN_2 17
#define LED_2_INDEX 1
#define BTN_3 18
#define LED_3_INDEX 2

#define LED_COUNT 3
#define LEFT_LED_PIN 12
#define MIDDLE_LED_PIN 10
#define RIGHT_LED_PIN 8

int leds[LED_COUNT] = {LEFT_LED_PIN, MIDDLE_LED_PIN, RIGHT_LED_PIN};
int index = 0;

volatile boolean interruptCalled = false;
boolean incomingInput = false;
int pulses[NUM_PULSES];
int pulseIndex = 0;
unsigned long startMicros;

void setup() {
  attachInterrupt(PIN2_INT, interruptCallback, CHANGE);
  for (int i = 0;i < LED_COUNT; ++i) {
    pinMode(leds[i], OUTPUT);
  }
  Serial.begin(BAUD_RATE);
  Serial.println("Starting");
  int pin = leds[index];
  digitalWrite(pin, HIGH);
}

void loop() {
  unsigned long currentMicros = micros();
  if (interruptCalled) {
    if (!incomingInput) {
      resetInput();
    } else {
      if (pulseIndex < NUM_PULSES) {
        pulses[pulseIndex++] = currentMicros - startMicros;
      }
    }
    startMicros = currentMicros;
    interruptCalled = false;
  } else {
    if (incomingInput && ((currentMicros - startMicros) > MAX_WAIT)) {
      if (pulseIndex > 3) {
        if (headerGood(pulses[0], pulses[1])) {
          Serial.println("header passed");
          //getCode(&pulses[HEADER_PULSES]);
          int btnPressed = getCode(&pulses[HEADER_PULSES + ADDRESS_PULSES]);
          handleButton(btnPressed);
        } else {
          Serial.print(pulseIndex, DEC);
          Serial.println("header failed");
          printData("HEADER", HEADER_PULSES, &pulses[0]);
          printData("ADDRESS", ADDRESS_PULSES, &pulses[HEADER_PULSES]);
          printData("COMMAND", COMMAND_PULSES, &pulses[HEADER_PULSES + ADDRESS_PULSES]);
        }
        printData("END", END_PULSES, &pulses[HEADER_PULSES + ADDRESS_PULSES + COMMAND_PULSES]);
      }
      incomingInput = false;
    }
  }
}

void printData(const char* label, int count, int* data) {
  Serial.println(label);
  for (int i = 0;i < count; ++i) {
    if ((i % 2) == 1) {
      Serial.print(",");
    }
    Serial.print(data[i], DEC);
    if ((i % 2) == 1) {
      Serial.println("");
    }
  }
  Serial.println("");
}

void resetInput() {
  incomingInput = true;
  memset(pulses, 0, sizeof(pulses));
  pulseIndex = 0;
}

void interruptCallback() {
    interruptCalled = true;
}

bool headerGood(int lead, int trail) {
  //return (abs(HEADER_LEAD - lead) < VALUE_THRESHOLD &&
  //  (abs(HEADER_TRAIL - trail) < VALUE_THRESHOLD));
  return (VALUE_GOOD(HEADER_LEAD, lead) &&
    VALUE_GOOD(HEADER_TRAIL, trail));
}

int getCode(int* codeData) {
  int firstResult = 0;
  int secondResult = 0;
  for (int i = 0;i < 8; ++i) {
    firstResult += (codeData[(i * 2) + 1] > 1000 ? 1 : 0) << i;
    secondResult += (codeData[(i * 2) + 17] > 1000 ? 1 : 0) << i;
  }
  Serial.print(firstResult, DEC);
  Serial.print(",");
  Serial.print(secondResult, DEC);
  Serial.print((firstResult ^ secondResult == 255) ? "pass" : "fail");
  Serial.println("Done with code");
  return firstResult;
}

void handleButton(int code) {
  int pin = leds[index];
  switch (code) {
    case BTN_RIGHT:
          digitalWrite(pin, LOW);
          index = (index + 1) % LED_COUNT;
          pin = leds[index];
          digitalWrite(pin, HIGH);
      break;
    case BTN_LEFT:
          digitalWrite(pin, LOW);
          index = (index + (LED_COUNT - 1)) % LED_COUNT;
          pin = leds[index];
          digitalWrite(pin, HIGH);
      break;
    case BTN_1:
          if (index != LED_1_INDEX) {
            digitalWrite(pin, LOW);
            pin = leds[LED_1_INDEX];
            digitalWrite(pin, HIGH);
            index = LED_1_INDEX;
          }
      break;
    case BTN_2:
          if (index != LED_2_INDEX) {
            digitalWrite(pin, LOW);
            pin = leds[LED_2_INDEX];
            digitalWrite(pin, HIGH);
            index = LED_2_INDEX;
          }
      break;
    case BTN_3:
          if (index != LED_3_INDEX) {
            digitalWrite(pin, LOW);
            pin = leds[LED_3_INDEX];
            digitalWrite(pin, HIGH);
            index = LED_3_INDEX;
          }
      break;
  }
}
