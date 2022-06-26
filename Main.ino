// Блок 1. Подключение библиотек
#include <Adafruit_MLX90614.h> // Библиотека для датчика MLX90614 
#include <WiFi.h> // Библиотека Wi-Fi
#include "secrets.h" // Файл с конфиденциальными данными
#include <Wire.h> // Библиотека для I2C интерфейса
#include <LiquidCrystal_I2C.h> // Библиотека для LCD дисплея, подключаемого через I2C интерфейс
#include "ThingSpeak.h" // Библиотека для отправки данных в ThingSpeak

// Блок 2. Подключение устройств и систем
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal_I2C LCD(0x27, 16, 2);
WiFiClient  client;
const int Button = 4;
const int Buzzer = 14;
const int F = 1000;

// Блок 3. Определение констант и переменных
char ssid[] = SECRET_SSID; // имя сети Wi-Fi
char pass[] = SECRET_PASS; // пароль сети Wi-Fi
unsigned long myChannelNumber = SECRET_CH_ID; // номер канала ThingSpeak
const char * myWriteAPIKey = SECRET_WRITE_APIKEY; // ключ API для записи данных в ThingSpeak
float T_obj = 0; // переменная для хранения измеренной температуры

// Блок 4. Описание функций
void DisplayInit() { // Функция инициализации дисплея
  Serial.print("Display... "); 
  LCD.init(); // Инициализация дисплея
  Wire.beginTransmission(byte(39)); // Начало передачи данных дисплею
  if (Wire.endTransmission() != 0) { // Проверка наличия связи с дисплеем
    Serial.println("ERROR");
    while (1); // Бесконечный цикл в случае ошибки
  }
  Serial.println("OK!");
  LCD.backlight(); // Включение подсветки дисплея
  LCD.display(); // Включение дисплея
  LCD.clear(); // Очистка дисплея
}

void mlxInit() { // Функция инициализации ИК-датчика
  Serial.print("MLX90614... ");
  mlx.begin(); // Инициализация датчика 
  if (!mlx.begin()) { // Проверка наличия связи с датчиком
    Serial.println("ERROR");
    LCD.clear();
    LCD.println("MLX90614 ERROR"); // Вывод на дисплей информации об ошибке
    while (1);
  }
  Serial.println("OK!");
}

void WiFiConnect() { // Функция подключения к сети Wi-Fi
  if (WiFi.status() != WL_CONNECTED) { // Проверка статуса подключения
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    LCD.clear();
    LCD.print("Connecting to");
    LCD.setCursor(0, 1);
    LCD.print(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Подключение к сети Wi-Fi
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
    LCD.clear();
    LCD.setCursor(3, 0);
    LCD.print("Connected");
  }
}

void Buzz(int K){ // Функция воспроизведения звука
  for (byte i = 0; i < K; i++){
    analogWrite(Buzzer, 250);
    delay(300);
    analogWrite(Buzzer, 0);
    delay(100);
  }
}

// Блок 5. Инициализация устройства
void setup() {
  Serial.begin(9600);
  Serial.println("Initialization..");
  DisplayInit();
  LCD.print("Initialization..");
  mlxInit();
  WiFi.mode(WIFI_STA); // Установка режима работы WI-FI модуля
  pinMode(Button, OUTPUT); // Установка режима подключения кнопки
  pinMode(Buzzer, OUTPUT); // Установка режима подключения кнопки
  ThingSpeak.begin(client); // Инициализация библиотеки ThingSpeak
  WiFiConnect();
  Serial.println("READY!");
}

// Блок 6. Основной цикл
void loop() {

  WiFiConnect();

  if (digitalRead(Button)) { // Проверка нажатия кнопки
    T_obj = mlx.readObjectTempC(); // Измерение температуры и запись в переменную
    while (T_obj > 85 || isnan(T_obj)) { // Проверка правильности измерения
      T_obj = mlx.readObjectTempC();
    }
    LCD.clear();
    LCD.setCursor(4, 0);
    LCD.print("T=");
    LCD.print(T_obj);
    LCD.print("C");
    LCD.setCursor(2, 1);
    if (T_obj > 37.5) { // Проверка повышенной температуры
      LCD.setCursor(2, 1);
      LCD.print("!!!WARNING!!!");
      Buzz(3);
    }
    else Buzz(1);

    LCD.setCursor(2, 1);
    ThingSpeak.setField(2, T_obj); // Установка данных для отправки в облако
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // Отправка данных в облако и запись в переменную кода результата отправки
    if (x == 200) { // Проверка успешной отправки
      Serial.println("Channel update successful.");
      LCD.print("Successfull");
    }
    else {
      Serial.println("Problem updating channel. HTTP error code " + String(x)); // Вывод кода ошибки
      LCD.print("Error: ");
      LCD.print(String(x));
    }
  }
  delay(100); // Задержка 0.1с
}
