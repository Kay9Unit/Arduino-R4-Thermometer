#include <Servo.h>

#define TEMPERATURE_PIN 0
#define SERVO_PIN 5
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11

Servo servo;

int pins[4] = {SERVO_PIN, RED_PIN, GREEN_PIN, BLUE_PIN};

void setup()
{ 
  Serial.begin(9600);

  for (auto p : pins) pinMode(p, OUTPUT);
  servo.attach(SERVO_PIN);

  selfTest();
}

void selfTest()
{
  testLED();
  testServo();
}

void testLED()
{
  // show all possible color combinations in a one shot rainbow spectrum
  for (int i = 0; i < 768; i++)
  {
    showRGB(i);
    delay(3);
  }
  
  // reset LED to blank
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
}

// taken from vilros sketch 03
void showRGB(int color)
{
  int redIntensity;
  int greenIntensity;
  int blueIntensity;

  if (color <= 255)
  {
    redIntensity = 255 - color;
    greenIntensity = color;
    blueIntensity = 0;
  }
  else if (color <= 511)
  {
    redIntensity = 0;
    greenIntensity = 255 - (color - 256);
    blueIntensity = (color - 256);
  }
  else 
  {
    redIntensity = (color - 512);
    greenIntensity = 0;
    blueIntensity = 255 - (color - 512);
  }

  analogWrite(RED_PIN, redIntensity);
  analogWrite(BLUE_PIN, blueIntensity);
  analogWrite(GREEN_PIN, greenIntensity);
}

void testServo()
{
  // start at the very right
  servo.write(0);
  delay(100);
  for (int i = 0; i < 360; i++)
  {
    // linear v curve; goes up to 180 and back down to 0.
    int angle = -abs(i - 180) + 180;
    servo.write(angle);
    delay(10);
  }
}


void loop()
{
  float voltage, degreesC, degreesF;

  // get and convert voltage to degrees
  voltage = getVoltage(TEMPERATURE_PIN);
  degreesC = (voltage - 0.5) * 100.0;
  degreesF = degreesC * (9.0/5.0) + 32.0;

  // print out, for debugging purposes
  Serial.print("voltage: ");
  Serial.print(voltage);
  Serial.print("  deg C: ");
  Serial.print(degreesC);
  Serial.print("  deg F: ");
  Serial.println(degreesF);

  // turn on the LED that is designated
  // for its degree range.
  writeToLEDByTemp(degreesF);
  // move servo to point at physical temperature sign
  writeToServoByTemp(degreesF);

  // writing to servo too quickly effects the voltage the tpm receives yielding sporradic results, slow it down!!
  delay(100);
}

// helper function that gets the voltage in from the specified pin
// multiplied by some arbitrary number stolen from the example.
float getVoltage(int pin)
{  
  return (analogRead(pin) * 0.004882814);
}

// the coldest and hottest consts that we clamp our temperature value to.
// useful for calibrating physical visualizers like LED's and servos.
#define COLDEST 65 // coldest temp in the range we care to read.
#define HOTTEST 100 // hottest temp in the range we care to read.

#define HOT_COLOR 0xFF0000 // pure red in RRGGBB
#define COLD_COLOR 0x00FFFF // cool aqua blue in RRGGBB

void writeToLEDByTemp(float degreesF)
{
  // get the fraction of degree we are at between our min and max
  float step = (degreesF - COLDEST) / (HOTTEST - COLDEST);
  // normalize step to 0-1, when our temperatures exceed our min and max.
  step = step < 0? 0 : step > 1? 1 : step;
  // interpolate between our coldest and hottest colors using step.
  // 0 is blue-est, 1 is red-est
  unsigned int color = lerpColor(COLD_COLOR, HOT_COLOR, step);

  // debug step and color lerping
  // Serial.print(step); Serial.print(" "); Serial.println(String(color, HEX));

  // extract individual colors from color hex
  unsigned int red = (color >> 16) & 0xFF;
  unsigned int green = (color >> 8) & 0xFF;
  unsigned int blue = color & 0xFF;

  // send pwm signal to LED pins
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

int lerpColor(int color1, int color2, float x)
{
  // extract individual colors of our first color
  int r1 = (color1 >> 16) & 0xFF;
  int g1 = (color1 >> 8) & 0xFF;
  int b1 = color1 & 0xFF;

  // extract individual colors of our second color
  int r2 = (color2 >> 16) & 0xFF;
  int g2 = (color2 >> 8) & 0xFF;
  int b2 = color2 & 0xFF;

  // interpolate between our colors based on delta x
  int r = lerp(r1, r2, x);
  int g = lerp(g1, g2, x);
  int b = lerp(b1, b2, x);

  // construct new lerped color hex
  return (r << 16) | (g << 8) | b;
}

// servo operates backwards;
// 0 points to direct right
// and 180 points to direct left,
// so we must reverse our temp readings.
#define MAX_TEMP_ANGLE 0
#define MIN_TEMP_ANGLE 180

void writeToServoByTemp(float degreesF)
{
  // get the fraction of degree we are at between our min and max
  float step = (degreesF - COLDEST) / (HOTTEST - COLDEST);
  // normalize step to 0-1, when our temperatures exceed our min and max.
  step = step < 0? 0 : step > 1? 1 : step;
  // interpolate between our min and man angles using step.
  unsigned int guageAngle = lerp(MIN_TEMP_ANGLE, MAX_TEMP_ANGLE, step);

  // debug step and color lerping
  Serial.print(step); Serial.print(" "); Serial.println(guageAngle);

  // write our calculated angle to the servo
  servo.write(guageAngle);
}

int lerp(int start, int end, float x)
{
  // math. I hate math.
  return (1 - x) * start + x * end;
}