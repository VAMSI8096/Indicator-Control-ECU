// ================== PIN DEFINITIONS ==================
#define LEFT_BUTTON 2
#define RIGHT_BUTTON 3

#define LEFT_LED 9
#define RIGHT_LED 10

// ================== GLOBAL VARIABLES ==================
typedef enum
{
  STATE_IDLE,
  STATE_LEFT,
  STATE_RIGHT,
  STATE_HAZARD
} State_t;

State_t state = STATE_IDLE;

unsigned long system_time = 0;

// Scheduler flags
uint8_t flag_100ms = 0;
uint8_t flag_300ms = 0;
uint8_t counter_300ms = 0;

// Button handling
uint8_t left_count = 0;
uint8_t right_count = 0;

uint8_t left_event = 0;
uint8_t right_event = 0;
uint8_t both_event = 0;

// LED state
uint8_t led_state = 0;

// Timer
unsigned long last_tick = 0;

// ================== SETUP ==================
void setup()
{
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);

  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);

  Serial.begin(9600);
}

// ================== TIMER ==================
void Timer_Update()
{
  if (millis() - last_tick >= 100)
  {
    last_tick = millis();
    system_time += 100;
    flag_100ms = 1;
  }
}

// ================== BUTTON TASK ==================
void Button_Task()
{
  uint8_t left = digitalRead(LEFT_BUTTON) == LOW;
  uint8_t right = digitalRead(RIGHT_BUTTON) == LOW;

  // LEFT BUTTON
  if (left)
  {
    left_count++;
    if (left_count == 10)
    {
      left_event = 1;
      Log("LEFT_BUTTON_PRESSED");
    }
  }
  else
  {
    left_count = 0;
  }

  // RIGHT BUTTON
  if (right)
  {
    right_count++;
    if (right_count == 10)
    {
      right_event = 1;
      Log("RIGHT_BUTTON_PRESSED");
    }
  }
  else
  {
    right_count = 0;
  }

  // BOTH BUTTONS
  if (left && right && left_count >= 10 && right_count >= 10)
  {
    both_event = 1;
  }
}

// ================== INDICATOR LOGIC ==================
void Indicator_Task()
{
  switch (state)
  {
  case STATE_IDLE:
    if (both_event)
    {
      state = STATE_HAZARD;
      Log("HAZARD_ON");
    }
    else if (left_event)
    {
      state = STATE_LEFT;
      Log("LEFT_INDICATOR_ON");
    }
    else if (right_event)
    {
      state = STATE_RIGHT;
      Log("RIGHT_INDICATOR_ON");
    }
    break;

  case STATE_LEFT:
    if (left_event)
    {
      state = STATE_IDLE;
      Log("LEFT_INDICATOR_OFF");
    }
    else if (right_event)
    {
      state = STATE_RIGHT;
      Log("RIGHT_INDICATOR_ON");
    }
    break;

  case STATE_RIGHT:
    if (right_event)
    {
      state = STATE_IDLE;
      Log("RIGHT_INDICATOR_OFF");
    }
    else if (left_event)
    {
      state = STATE_LEFT;
      Log("LEFT_INDICATOR_ON");
    }
    break;

  case STATE_HAZARD:
    if (left_event || right_event)
    {
      state = STATE_IDLE;
      Log("HAZARD_OFF");
    }
    break;
  }

  left_event = 0;
  right_event = 0;
  both_event = 0;
}

// ================== LED TASK ==================
void LED_Task()
{
  led_state = !led_state;

  switch (state)
  {
  case STATE_LEFT:
    digitalWrite(LEFT_LED, led_state);
    digitalWrite(RIGHT_LED, LOW);
    Log("LEFT_LED_TOGGLE");
    break;

  case STATE_RIGHT:
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, led_state);
    Log("RIGHT_LED_TOGGLE");
    break;

  case STATE_HAZARD:
    digitalWrite(LEFT_LED, led_state);
    digitalWrite(RIGHT_LED, led_state);
    Log("HAZARD_LED_TOGGLE");
    break;

  default:
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
    break;
  }
}

// ================== LOGGER ==================
void Log(const char *msg)
{
  Serial.print("[");
  Serial.print(system_time);
  Serial.print(" ms] ");
  Serial.println(msg);
}

// ================== LOOP ==================
void loop()
{
  Timer_Update();

  if (flag_100ms)
  {
    flag_100ms = 0;

    Button_Task();
    Indicator_Task();

    counter_300ms++;
    if (counter_300ms >= 3)
    {
      counter_300ms = 0;
      flag_300ms = 1;
    }
  }

  if (flag_300ms)
  {
    flag_300ms = 0;
    LED_Task();
  }
}
