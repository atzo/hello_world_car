# include <Servo.h>

#define servo_pin D4
#define brzina_naprijed D2
#define brzina_nazad D3
#define rikverc D8
#define stop_svjetlo D5
#define farovi D1
#define zmigavac_l D6
#define zmigavac_d D7
int old_brzina = 0;
int zmigavac_speed = 500;
int old_volan_stupanj = 90;
unsigned long volan_exec_time = millis();
unsigned long stop_exec_time = millis();
unsigned long zmigavac_exec_time = millis();
bool servo_active = false;
bool stop_active = false;
bool zmigavac_l_active = false;
bool zmigavac_d_active = false;
Servo servo1;

void servo_detach() {
  if (servo_active) {
    if (millis() - volan_exec_time > 500) {
      servo1.detach();
      servo_active = false;
    }
  }
}

void iskjuci_stop() {
  if (stop_active) {
    if (millis() - stop_exec_time > 1000) {
      digitalWrite(stop_svjetlo, LOW);
      stop_active = false;
    }
  }
}

void volan(int stupanj) {
  if (!servo_active) {
    servo1.attach(servo_pin);
    servo_active = true;
  }
  Serial.println(stupanj);
  servo1.write(stupanj);
  volan_exec_time = millis();
  if (stupanj > 100 && old_volan_stupanj > stupanj) {
    zmigavac_stop(0);
  }
  if (stupanj < 80 && old_volan_stupanj < stupanj) {
    zmigavac_stop(1);
  }
  old_volan_stupanj = stupanj;
}

void send_farovi(int jacina) {
  analogWrite(farovi, jacina);
}

void setup_brzina() {
  pinMode(brzina_naprijed, OUTPUT);
  analogWrite(brzina_naprijed, 0);
  pinMode(brzina_nazad, OUTPUT);
  analogWrite(brzina_nazad, 0);
  pinMode(stop_svjetlo, OUTPUT);
  digitalWrite(stop_svjetlo, LOW);
  pinMode(rikverc, OUTPUT);
  digitalWrite(rikverc, LOW);
  pinMode(farovi, OUTPUT);
  analogWrite(farovi, 0);
  pinMode(zmigavac_l, OUTPUT);
  digitalWrite(zmigavac_l, LOW);
  pinMode(zmigavac_d, OUTPUT);
  digitalWrite(zmigavac_d, LOW);
}

void send_brzina(int pin, int brzina) {
  if (brzina == 10) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  } else {
    analogWrite(pin, brzina * 113);
  }
}

void brzina(int brzina) {
  Serial.print("promjena brzine: ");
  Serial.println(brzina);
  analogWrite(brzina_naprijed, brzina);
  if (brzina == 0) {
    analogWrite(brzina_naprijed, 0);
    analogWrite(brzina_nazad, 0);
    digitalWrite(rikverc, LOW);
    Serial.println("Iskjucili brzine!");
  } else if (brzina > 0) {
    analogWrite(brzina_naprijed, 0);
    digitalWrite(rikverc, HIGH);
    send_brzina(brzina_nazad, brzina);
  } else {
    analogWrite(brzina_nazad, 0);
    digitalWrite(rikverc, LOW);
    if (brzina > old_brzina) {
      digitalWrite(stop_svjetlo, HIGH);
      stop_active = true;
      stop_exec_time = millis();
    }
    send_brzina(brzina_naprijed, abs(brzina));
  }
  old_brzina = brzina;
}

void send_zmigavac(int strana) {
  if (strana == 0) {
    zmigavac_stop(1);
    zmigavac_l_active = true;
    digitalWrite(zmigavac_l, HIGH);;
  } else {
    zmigavac_stop(0);
    zmigavac_d_active = true;
    digitalWrite(zmigavac_d, HIGH);;
  }
  zmigavac_exec_time = millis();
}

void zmigavac_togle() {
  if (zmigavac_l_active && millis() - zmigavac_exec_time > zmigavac_speed) {
    digitalWrite(zmigavac_l, !digitalRead(zmigavac_l));
    zmigavac_exec_time = millis();
  }
  if (zmigavac_d_active && millis() - zmigavac_exec_time > zmigavac_speed) {
    digitalWrite(zmigavac_d, !digitalRead(zmigavac_d));
    zmigavac_exec_time = millis();
  }
}

void zmigavac_stop(int strana) {
  if (strana == 0) {
    zmigavac_l_active = false;
    digitalWrite(zmigavac_l, LOW);
  } else {
    zmigavac_d_active = false;
    digitalWrite(zmigavac_d, LOW);
  }
}
