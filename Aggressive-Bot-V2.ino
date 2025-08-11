// ===== Aggressive V2 (LCD + 6 servos + 1 ultrasonic, NO motors) =====
// LCD (parallel 4-bit): RS=12, E=11, D4=5, D5=4, D6=3, D7=2, RW=GND, VO=pot, LED+ via ~200Ω -> 5V, LED- -> GND
// Servos: D6, D10, D8, A1, A2, A3  (power from 5–6V regulator, GND common with Arduino)
// Ultrasonic (front): D9  |  Tilt: D7  |  Light: D13  |  TMP36: A0  |  Buzzer: A5

#define DEMO_MODE 1          // 1 = auto cycle states for video; 0 = sensor-driven

#include <Servo.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pins
const int SIG_FRONT = 9;
const int TILT_PIN  = 7;
const int LIGHT_PIN = 13;
const int BUZZER    = A5;
const int TMP_PIN   = A0;

// Servos (6)
const int SERVO_PINS[6] = {6, 10, 8, A1, A2, A3};
Servo SV[6];

// ---- Power-friendly servo driver ----
float curDeg[6]={90,90,90,90,90,90};
float tgtDeg[6]={90,90,90,90,90,90};
unsigned long lastTick[6]={0,0,0,0,0,0};
const float MAX_DEG_STEP = 2.0;          // smaller = less current spikes
const unsigned long SERVO_PERIOD = 18;   // ms per-servo (staggered)
const int MIN_ANGLE=60, MAX_ANGLE=120;

void servoSetTarget(int i, int deg){ tgtDeg[i]=constrain(deg, MIN_ANGLE, MAX_ANGLE); }
void servoService(){
  unsigned long now=millis();
  for(int i=0;i<6;i++){
    if(now - lastTick[i] >= SERVO_PERIOD){
      lastTick[i]=now;
      float d = tgtDeg[i] - curDeg[i];
      if (fabs(d) < MAX_DEG_STEP) curDeg[i] = tgtDeg[i];
      else curDeg[i] += (d>0 ? MAX_DEG_STEP : -MAX_DEG_STEP);
      SV[i].write((int)curDeg[i]);
    }
  }
}

// Ultrasonic helpers (front only)
long pingOnce(uint8_t pin){
  pinMode(pin, OUTPUT); digitalWrite(pin, LOW); delayMicroseconds(2);
  digitalWrite(pin, HIGH); delayMicroseconds(5); digitalWrite(pin, LOW);
  pinMode(pin, INPUT);
  long dur = pulseIn(pin, HIGH, 30000);
  if (dur==0) return 999;
  return dur/58;
}
long pingMed(uint8_t p){ long v[3]; for(int i=0;i<3;i++){ v[i]=pingOnce(p); delay(2);} 
  if(v[0]>v[1]){ long t=v[0]; v[0]=v[1]; v[1]=t; } if(v[1]>v[2]){ long t=v[1]; v[1]=v[2]; v[2]=t; }
  if(v[0]>v[1]){ long t=v[0]; v[0]=v[1]; v[1]=t; } return v[1];
}

// Behavior
enum State { PATROL, TARGET, ATTACK_SPIN, RETREAT, FLANK_L, FLANK_R };
State state = PATROL;

const int SEE_TARGET   = 45;  // cm
const int STRIKE_RANGE = 18;  // cm

// Gentle beeper: short chirp on transitions only
State lastState = PATROL;
void beepOnChange(){
  if (state != lastState){
    tone(BUZZER, state==ATTACK_SPIN ? 1200 : 1800, 120);
    lastState = state;
  }
}

// UI
void lcdPad(const char* s){
  char buf[17]; byte i=0; for(; i<16 && s[i]; ++i) buf[i]=s[i]; while(i<16) buf[i++]=' '; buf[16]='\0'; lcd.print(buf);
}

void setup(){
  lcd.begin(16,2);
  lcd.setCursor(0,0); lcdPad("Aggressive v2");
  lcd.setCursor(0,1); lcdPad("LCD + 6 servos");

  pinMode(TILT_PIN, INPUT_PULLUP);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(BUZZER, OUTPUT); digitalWrite(BUZZER, LOW);

  for(int i=0;i<6;i++){ SV[i].attach(SERVO_PINS[i]); SV[i].write(90); }
  delay(300);
}

// main loop
void loop(){
  // --- Servo motion pattern (nonstop, power-friendly) ---
  static int base=90, dir=1;
  base += dir;
  if (base >= MAX_ANGLE){ base=MAX_ANGLE; dir=-1; }
  if (base <= MIN_ANGLE){ base=MIN_ANGLE; dir= 1; }
  auto mirror = [](int x){ return MIN_ANGLE + MAX_ANGLE - x; };
  // pairs counter-phase: (0,3) (1,4) (2,5)
  servoSetTarget(0, base); servoSetTarget(3, mirror(base));
  servoSetTarget(1, base); servoSetTarget(4, mirror(base));
  servoSetTarget(2, base); servoSetTarget(5, mirror(base));
  servoService();

  // --- State logic ---
  static unsigned long t0=0;
  long dF = 999;

#if DEMO_MODE
  // Auto cycle: PATROL(3s) -> TARGET(3s) -> ATTACK(2s) -> RETREAT(2s) -> FLANK_L(2s) -> FLANK_R(2s)
  static int phase=0;
  unsigned long now=millis();
  if (now - t0 > (phase==2 ? 2000UL : (phase<2 ? 3000UL : 2000UL))){
    t0 = now; phase = (phase+1)%6;
    state = (State)phase;  // order matches enum above
  }
#else
  // Sensor-driven (front only): “attack every ~5s if target close”
  static unsigned long lastSense=0, lastAttack=0;
  unsigned long now=millis();
  if(now - lastSense > 120){ lastSense=now; dF = pingMed(SIG_FRONT); }
  if (digitalRead(TILT_PIN)==LOW){ state=RETREAT; }
  else if (dF <= STRIKE_RANGE && (now-lastAttack>5000)){ state=ATTACK_SPIN; lastAttack=now; }
  else if (dF <= SEE_TARGET){ state=TARGET; }
  else { state=PATROL; }
#endif

  // Beep only on state change
  beepOnChange();

  // --- LCD HUD (no buzzing here) ---
  static unsigned long tlcd=0;
  if (millis()-tlcd > 250){
    tlcd = millis();
    // TMP36 °C
    int raw = analogRead(TMP_PIN);
    float v = raw * 5.0 / 1024.0;
    int tC = (int)((v - 0.5) * 100.0);

    // line 1
    lcd.setCursor(0,0);
    char l0[17];
#if DEMO_MODE
    snprintf(l0, sizeof(l0), "Mode:%-10s",
      state==PATROL?"PATROL":
      state==TARGET?"TARGET":
      state==ATTACK_SPIN?"ATTACK":
      state==RETREAT?"RETREAT":
      state==FLANK_L?"FLANK_L":"FLANK_R");
#else
    dF = pingMed(SIG_FRONT);
    snprintf(l0, sizeof(l0), "F:%02ldcm %-8s", dF,
      (state==ATTACK_SPIN?"ATTACK": state==TARGET?"TARGET":
       state==RETREAT?"RETREAT": state==FLANK_L?"FL_L":
       state==FLANK_R?"FL_R":"PATROL"));
#endif
    lcdPad(l0);

    // line 2
    lcd.setCursor(0,1);
    int sweep = map(base, MIN_ANGLE, MAX_ANGLE, 0, 100);
    char l1[17]; snprintf(l1, sizeof(l1), "T:%2dC S:%3d%%  ", tC, sweep);
    lcdPad(l1);
  }

  delay(3);  // tiny yield
}
