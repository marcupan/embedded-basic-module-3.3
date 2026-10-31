// ═══════════════════════════════════════════════════════════════════════════
// Домашка 3.3 · потенціометр керує яскравістю LED і швидкістю мотора
// Плата: YD-ESP32-S3 (ESP32-S3-WROOM-1-N16R8) · PlatformIO · Arduino framework
// Карта: map.svg
//
// LED і мотор сидять на різних каналах і різних таймерах LEDC, тому не заважають одне одному.
//
// Клавіші в моніторі порту:
//   m — мотор увімк/вимк: LED далі йде за потенціометром
//   l — LED увімк/вимк: мотор далі йде за потенціометром
//   ? — підказка
// ═══════════════════════════════════════════════════════════════════════════
#include <Arduino.h>

namespace {
    struct Config {
        static constexpr uint8_t POT_PIN = 5; // повзунок, ADC1
        static constexpr uint8_t LED_PIN = 18;
        static constexpr uint8_t MOTOR_PIN = 13; // база 2N2222A через 1 кОм

        // Канали 0 і 2 сидять на різних таймерах LEDC, тому частоти незалежні
        static constexpr uint8_t LED_CH = 0;
        static constexpr uint8_t MOTOR_CH = 2;
        static constexpr uint32_t LED_FREQ_HZ = 5000;
        static constexpr uint32_t MOTOR_FREQ_HZ = 10000; // LMS: 10–20 кГц, щоб мотор не пищав
        static constexpr uint8_t PWM_BITS = 10; // duty 0…1023

        static constexpr uint32_t PRINT_MS = 500;
    };

    bool motorOn = true;
    bool ledOn = true;

    void printHelp() {
        Serial.println("m — мотор увімк/вимк · l — LED увімк/вимк · ? — підказка");
    }

    void handleKeys() {
        while (Serial.available() > 0) {
            const char key = static_cast<char>(Serial.read());
            if (key == 'm') {
                motorOn = !motorOn;
            } else if (key == 'l') {
                ledOn = !ledOn;
            } else if (key == '?') {
                printHelp();
            }
        }
    }
} // namespace

void setup() {
    ledcSetup(Config::LED_CH, Config::LED_FREQ_HZ, Config::PWM_BITS);
    ledcAttachPin(Config::LED_PIN, Config::LED_CH);
    ledcSetup(Config::MOTOR_CH, Config::MOTOR_FREQ_HZ, Config::PWM_BITS);
    ledcAttachPin(Config::MOTOR_PIN, Config::MOTOR_CH);
    ledcWrite(Config::MOTOR_CH, 0);

    analogSetAttenuation(ADC_11db); // повний діапазон до ~3,1 В

    Serial.begin(115200);
    delay(1500); // час на підключення монітора (рідний USB)
    Serial.println("Домашка 3.3 · потенціометр → LED і мотор");
    printHelp();
}

void loop() {
    const uint32_t now = millis();
    static uint32_t lastPrint = 0;

    handleKeys();

    const uint32_t raw = analogRead(Config::POT_PIN); // 0…4095
    const uint32_t duty = raw * 1023 / 4095;
    ledcWrite(Config::LED_CH, ledOn ? duty : 0);
    ledcWrite(Config::MOTOR_CH, motorOn ? duty : 0);

    if (now - lastPrint >= Config::PRINT_MS) {
        lastPrint = now;
        Serial.printf("пот %4lu | duty %3lu%% | LED %-4s | мотор %-4s\n",
                      static_cast<unsigned long>(raw),
                      static_cast<unsigned long>(duty * 100 / 1023),
                      ledOn ? "УВІМ" : "вимк",
                      motorOn ? "УВІМ" : "вимк");
    }
}
