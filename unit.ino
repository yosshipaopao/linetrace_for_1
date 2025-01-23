#include <Arduino.h>

const int M1_L = 3, M1_R = 4, M1_P = 5, M2_L = 9, M2_R = 8, M2_P = 10;
int sp[4] = {A0, A1, A2, A3};


class Motor
{
public:
  Motor(int left, int right, int pwm);
  void setup();
  void run(int speed);
  void stop();

private:
  void forward(int speed);
  void backward(int speed);
  int left, right, pwm;
};

Motor::Motor(int left, int right, int pwm)
{
  this->left = left;
  this->right = right;
  this->pwm = pwm;
}

void Motor::setup()
{
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);
  pinMode(pwm, OUTPUT);
}

void Motor::run(int speed)
{
  speed = constrain(speed, -255, 255);
  if (speed > 0)
    forward(speed);
  else if (speed < 0)
    backward(-speed);
  else
    stop();
}

void Motor::forward(int speed)
{
  digitalWrite(left, HIGH);
  digitalWrite(right, LOW);
  analogWrite(pwm, speed);
}

void Motor::backward(int speed)
{
  digitalWrite(left, LOW);
  digitalWrite(right, HIGH);
  analogWrite(pwm, speed);
}

void Motor::stop()
{
  digitalWrite(left, LOW);
  digitalWrite(right, LOW);
  analogWrite(pwm, 0);
}


class Sensor
{
public:
    Sensor(int *pins, int pin_size, int threshold);
    void setup();
    void read();
    bool isBlack(int pin);
    bool get(int pin);
    int val(int pin);
    int state();
    void print_raw();

private:
    int *pins;
    int pin_size;
    int values[100];
    int threshold;
};

Sensor::Sensor(int *pins, int pin_size, int threshold)
{
    this->pins = pins;
    this->pin_size = pin_size;
    this->threshold = threshold;
}

void Sensor::setup()
{
    for (int i = 0; i < pin_size; i++)
    {
        pinMode(pins[i], INPUT);
    }
}

void Sensor::read()
{
    for (int i = 0; i < pin_size; i++)
    {
        values[i] = analogRead(pins[i]);
    }
}

bool Sensor::isBlack(int pin)
{
    return analogRead(pin) > threshold;
}

bool Sensor::get(int pin)
{
    return values[pin] > threshold;
}

int Sensor::val(int pin)
{
    return values[pin];
}

int Sensor::state()
{
    int state = 0;
    for (int i = 0; i < pin_size; i++)
    {
        if (isBlack(pins[i]))
        {
            state |= 1 << i;
        }
    }
    return state;
}

void Sensor::print_raw()
{
    for (int i = 0; i < pin_size; i++)
    {
        Serial.print(values[i]);
        Serial.print(" ");
    }
    Serial.println();
}
void cal_motor_power(int *l_power, int *r_power, int pow_l, int pow_m, int pow_s, int state, int *pre_dir)
{
    switch (state)
    {
    case 0b0000:
        switch (*pre_dir)
        {
        case -1:
            *l_power = pow_s;
            *r_power = pow_l;
            break;
        case 0:
            *l_power = pow_m;
            *r_power = pow_m;
            break;
        case 1:
            *l_power = pow_l;
            *r_power = pow_s;
            break;
        }
        break;
    case 0b0110:
        *l_power = pow_l;
        *r_power = pow_l;
        break;
    case 0b0010:
        *l_power = pow_l;
        *r_power = pow_m;
        break;
    case 0b0100:
        *l_power = pow_m;
        *r_power = pow_l;
        break;
    case 0b0011:
    case 0b0001:
        *l_power = pow_l;
        *r_power = pow_s;
        *pre_dir = 1;
        break;
    case 0b1100:
    case 0b1000:
        *l_power = pow_s;
        *r_power = pow_l;
        *pre_dir = -1;
        break;
    case 0b0111:
        *l_power = pow_l;
        *r_power = pow_m;
        *pre_dir = 1;
        break;
    case 0b1110:
        *l_power = pow_m;
        *r_power = pow_l;
        *pre_dir = -1;
        break;
    case 0b1111:
        *l_power = pow_l;
        *r_power = pow_l;
        break;
    default:
        *l_power = 0;
        *r_power = 0;
        break;
    }
}

Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Sensor s(sp, 4, 300);

void setup()
{
    Serial.begin(9600);
    m1.setup();
    m2.setup();
    s.setup();
}
const int pow_l = 255, pow_m = 153, pow_s = -51;
int pre_dir = 0;

void loop()
{
    s.read();
    int l_power, r_power;
    cal_motor_power(&l_power, &r_power, pow_l, pow_m, pow_s, s.state(), &pre_dir);
    m1.run(l_power);
    m2.run(r_power);
    delay(10);
}
