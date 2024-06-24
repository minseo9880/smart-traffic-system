include <LiquidCrystal_I2C.h>

// I2C 주소 0x27, 16열, 2행으로 LCD 디스플레이 초기화
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 노란색과 빨간색 LED 핀 정의
const int yellowPin = 9;
const int redPin = 10;

// 초음파 센서 1의 트리거와 에코 핀 정의
const int trigPin1 = 2;
const int echoPin1 = 3;

// 초음파 센서 2의 트리거와 에코 핀 정의
const int trigPin2 = 13;
const int echoPin2 = 12;

// RGB LED 1의 빨간색, 녹색, 파란색 핀 정의
const int ledPinRed1 = 4;
const int ledPinGreen1 = 5;
const int ledPinBlue1 = 6;

// RGB LED 2의 빨간색, 녹색, 파란색 핀 정의
const int ledPinRed2 = 11;
const int ledPinGreen2 = 7;
const int ledPinBlue2 = 8;

// 보행자 감지 시작 시간과 상태를 추적하는 변수
unsigned long pedestrianDetectedStartTime1 = 0;
unsigned long pedestrianDetectedStartTime2 = 0;
bool pedestrianDetected1 = false;
bool pedestrianDetected2 = false;
bool pedestrianDetected2Sec1 = false;
bool pedestrianDetected2Sec2 = false;

// 녹색 신호등 상태와 시작 시간을 추적하는 변수
unsigned long greenLightStartTime = 0;
bool isGreenLightOn = false;

// 마지막으로 시리얼 데이터를 출력한 시간을 추적하는 변수
unsigned long serialPrintLastTime = 0;

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);

  // 초음파 센서의 핀 모드 설정
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // 노란색과 빨간색 LED의 핀 모드 설정
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);

  // RGB LED의 핀 모드 설정
  pinMode(ledPinRed1, OUTPUT);
  pinMode(ledPinGreen1, OUTPUT);
  pinMode(ledPinBlue1, OUTPUT);
  pinMode(ledPinRed2, OUTPUT);
  pinMode(ledPinGreen2, OUTPUT);
  pinMode(ledPinBlue2, OUTPUT);

  // LCD 초기화
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("No pedestrian   ");
}

void loop() {
  // 두 초음파 센서에서 거리 읽기
  float distance1 = getDistance(trigPin1, echoPin1);
  float distance2 = getDistance(trigPin2, echoPin2);

  unsigned long currentMillis = millis();

  // 거리 정보를 1초마다 시리얼 모니터에 출력
  if (currentMillis - serialPrintLastTime >= 1000) {
    serialPrintLastTime = currentMillis;
    Serial.print("Distance1: ");
    Serial.print(distance1);
    Serial.println(" cm");
    Serial.print("Distance2: ");
    Serial.print(distance2);
    Serial.println(" cm");

    if (pedestrianDetected1) {
      Serial.print("Pedestrian1 detected for: ");
      Serial.print((currentMillis - pedestrianDetectedStartTime1) / 1000);
      Serial.println(" seconds");
    } else {
      Serial.println("Pedestrian1 not detected");
    }

    if (pedestrianDetected2) {
      Serial.print("Pedestrian2 detected for: ");
      Serial.print((currentMillis - pedestrianDetectedStartTime2) / 1000);
      Serial.println(" seconds");
    } else {
      Serial.println("Pedestrian2 not detected");
    }
  }

  // 초음파 센서 1에서 10cm 이내 거리에서 보행자 감지 확인
  if (distance1 <= 10) {
    if (!pedestrianDetected1) {
      pedestrianDetected1 = true;
      pedestrianDetectedStartTime1 = currentMillis;
    }
  } else {
    pedestrianDetected1 = false;
    pedestrianDetectedStartTime1 = 0;
  }

  // 초음파 센서 2에서 10cm 이내 거리에서 보행자 감지 확인
  if (distance2 <= 10) {
    if (!pedestrianDetected2) {
      pedestrianDetected2 = true;
      pedestrianDetectedStartTime2 = currentMillis;
    }
  } else {
    pedestrianDetected2 = false;
    pedestrianDetectedStartTime2 = 0;
  }

  // 보행자가 2초 이상 감지되었는지 확인하여 LCD 업데이트
  if (pedestrianDetected1 && (currentMillis - pedestrianDetectedStartTime1 >= 2000)) {
    pedestrianDetected2Sec1 = true;
  } else {
    pedestrianDetected2Sec1 = false;
  }

  if (pedestrianDetected2 && (currentMillis - pedestrianDetectedStartTime2 >= 2000)) {
    pedestrianDetected2Sec2 = true;
  } else {
    pedestrianDetected2Sec2 = false;
  }

  // 보행자가 7초 이상 감지되었는지 확인하여 신호등 상태 업데이트
  if ((pedestrianDetected1 && (currentMillis - pedestrianDetectedStartTime1 >= 7000)) || (pedestrianDetected2 && (currentMillis - pedestrianDetectedStartTime2 >= 7000))) {
    if (!isGreenLightOn) {
      isGreenLightOn = true;
      greenLightStartTime = currentMillis;
      lcd.setCursor(0, 0);
      lcd.print("Ped crossing    ");
    }
  }

  // 녹색 신호등이 15초 이상 켜져있다면 끄기
  if (isGreenLightOn && (currentMillis - greenLightStartTime >= 15000)) {
    isGreenLightOn = false;
    lcd.setCursor(0, 0);
    lcd.print("No pedestrian   ");
  }

  // 보행자 감지 및 녹색 신호등 상태에 따라 LED 상태 업데이트
  if (isGreenLightOn) {
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, HIGH);
    setColor1(0, 255, 0); // 보행자 신호등1을 녹색으로 설정
    setColor2(0, 255, 0); // 보행자 신호등2을 녹색으로 설정
  } else {
    digitalWrite(redPin, LOW);
    setColor1(255, 0, 0); // 보행자 신호등2을 빨간색으로 설정
    setColor2(255, 0, 0); // 보행자 신호등2을 빨간색으로 설정
    blinkYellowLight();
  }

  // 보행자 감지 상태에 따라 LCD 디스플레이 업데이트
  if ((pedestrianDetected2Sec1 || pedestrianDetected2Sec2) && !isGreenLightOn) {
    lcd.setCursor(0, 0);
    lcd.print("Pedestrian      ");
  } else if ((!pedestrianDetected2Sec1 && !pedestrianDetected2Sec2) && !isGreenLightOn) {
    lcd.setCursor(0, 0);
    lcd.print("No pedestrian   ");
  }

  delay(100);
}

// 초음파 센서에서 거리를 측정하는 함수
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  return ((float)(340 * duration) / 10000) / 2;
}

// RGB LED 1의 색상을 설정하는 함수
void setColor1(int red, int green, int blue) {
  analogWrite(ledPinRed1, red);
  analogWrite(ledPinGreen1, green);
  analogWrite(ledPinBlue1, blue);
}

// RGB LED 2의 색상을 설정하는 함수
void setColor2(int red, int green, int blue) {
  analogWrite(ledPinRed2, red);
  analogWrite(ledPinGreen2, green);
  analogWrite(ledPinBlue2, blue);
}

// 노란색 LED를 깜빡이게 하는 함수
void blinkYellowLight() {
  static unsigned long previousMillis = 0;
  static bool yellowState = false;

  if (millis() - previousMillis >= 500) {
    previousMillis = millis();
    yellowState = !yellowState;
    digitalWrite(yellowPin, yellowState);
  }
}