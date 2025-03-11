#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN  10

#define SERIAL_BAUD 9600

static MFRC522 s_reader;

void setup()
{
    Serial.begin(SERIAL_BAUD);
    while (!Serial);

    SPI.begin();

    s_reader.PCD_Init(SS_PIN, RST_PIN);
}

void loop()
{
    if (s_reader.PICC_IsNewCardPresent() && s_reader.PICC_ReadCardSerial())
    {
        const uint8_t* buffer = s_reader.uid.uidByte;
        const uint8_t bufferSize = s_reader.uid.size;

        for (uint8_t i = 0; i < bufferSize; ++i)
        {
            char byte_str[8];
            snprintf(byte_str, 8, "%02x", buffer[i]);
            Serial.print(byte_str);
        }
        Serial.println();
        
        s_reader.PICC_HaltA();
        s_reader.PCD_StopCrypto1();
    }
}