const int IR_sensor1 = 32;
const int IR_sensor2 = 39;

const int threshold = 2000;

// Ultrasonic Pins
const int trig_pin1 = 16;
const int echo_pin1 = 34;

const int trig_pin2 = 17;
const int echo_pin2 = 35;

const int trig_pin3 = 18;
const int echo_pin3 = 36;

int motor1pin1 = 21;
int motor1pin2 = 33;
int pwm1 = 25;
int motor2pin1 = 19;
int motor2pin2 = 27;
int pwm2 = 26;

const int freq = 500;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int resolution = 8;
int dutyCycle = 138;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // IR sensor
  pinMode(IR_sensor1, INPUT);
  pinMode(IR_sensor2, INPUT);
  
  // ultrasonic sensor
  pinMode(trig_pin1, OUTPUT);
  pinMode(echo_pin1, INPUT);
  pinMode(trig_pin2, OUTPUT);
  pinMode(echo_pin2, INPUT);
  pinMode(trig_pin3, OUTPUT);
  pinMode(echo_pin3, INPUT);

  // motor control
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(pwm2, OUTPUT);

  ledcAttachChannel(pwm1, freq, resolution, pwmChannel1);
  ledcAttachChannel(pwm2, freq, resolution, pwmChannel2);
}

int readIR(int pin) {
  const int samples = 5;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / samples;
}

long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  if(duration == 0) return -1;
  long dist = duration * 0.0343 / 2;
  return dist;
}

//–– Helper to fully stop the car ––
void stopCar() {
  // cut all direction lines and PWM
  ledcWrite(pwm1, 0);
  ledcWrite(pwm2, 0);
}

void loop() { // put your main code here, to run repeatedly:

  // Ultrasonic Sensor
  long d1 = readUltrasonic(trig_pin1, echo_pin1);
  long d2 = readUltrasonic(trig_pin2, echo_pin2);
  long d3 = readUltrasonic(trig_pin3, echo_pin3);

  bool obstacle = (d1 > 0 && d1 <= 20)
               || (d2 > 0 && d2 <= 20)
               || (d3 > 0 && d3 <= 20);

  if (obstacle) {
    Serial.println(">>> Obstacle too close! STOPPING.");
    stopCar();

    // block until clear
    do {
      delay(50);
      d1 = readUltrasonic(trig_pin1, echo_pin1);
      d2 = readUltrasonic(trig_pin2, echo_pin2);
      d3 = readUltrasonic(trig_pin3, echo_pin3);
    } while ((d1 > 0 && d1 <= 20)
          || (d2 > 0 && d2 <= 20)
          || (d3 > 0 && d3 <= 20));

    Serial.println(">>> Path clear. Resuming.");
    return;  // start next loop with fresh readings
  }

  // IR Sensor Pin
  int leftVal = readIR(IR_sensor1);
  int rightVal = readIR(IR_sensor2);
  Serial.print("IR L: "); Serial.print(leftVal);
  Serial.print("  IR R: "); Serial.println(rightVal);

  bool stop = false;
  int leftSpeed = dutyCycle;
  int rightSpeed = dutyCycle;

  if(leftVal > threshold && rightVal > threshold) {
    // Motor speed remains the same or return original motor speed
    stop = true;
  } else if (leftVal > threshold) {  // Adjust threshold based on your sensor's output
    // Turn Left
    leftSpeed = dutyCycle / 1.8;
    rightSpeed = dutyCycle;
  } else if(rightVal > threshold) {
    // Turn Right
    leftSpeed = dutyCycle;
    rightSpeed = dutyCycle / 1.8;
    // Serial.println("Object is Black");
  } else {
    leftSpeed = dutyCycle;
    rightSpeed = dutyCycle;
  }

  if(stop) {
    ledcWrite(pwm1, 0);
    ledcWrite(pwm2, 0);
  } else {
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);

    // Apply PWM speeds:
    ledcWrite(pwm1, leftSpeed);
    ledcWrite(pwm2, rightSpeed);
  }
}