#include <Arduino.h>

#include <CAN.h>

unsigned long lastSent = millis();
byte counter50b = 0;
byte counter = 0;
int torque = 0;

void parseCAN();

void sendCAN();

static void nissan_crc(byte *data);

void send(int id, size_t size);

bool active = false;

void setup() {
//    pinMode(6, OUTPUT);
//    digitalWrite(6, 0);
    Serial.begin(115200);
    while (!Serial);

    Serial.println("CAN");
    if (!CAN.begin(500e3)) {
        Serial.println("CAN Start Error");
        while (1);
    }
}


void loop() {
    while (Serial.available()) {
        int r = Serial.read();
        if ('0' <= r && r <= '9') {
            torque = (r - '5') * 64;
            Serial.print("Set torque : ");
            Serial.println(torque);
        } else if (r == ' ')
            active = !active;
    }
    parseCAN();
    unsigned long time = millis();
    if (time - lastSent >= 10 && active) {
        sendCAN();
        lastSent = time;
    }
//    digitalWrite(6, torque > 0);
}

byte message[8] = {0};

void sendCAN() {
    message[0] = 0x4E;
    message[1] = 0x40;
    message[2] = 0x00;
    message[3] = 0xAA;
    message[4] = 0xC0;
    message[5] = 0x00;
    message[6] = counter;
    send(0x11A, 8);
    message[0] = 0xF7;
    message[1] = 0x07;
    message[2] = static_cast<byte>(torque >> 8);
    message[3] = static_cast<byte>(torque & 0xFF);
    message[4] = static_cast<byte>(0x07 | (counter << 6));
    message[5] = 0x44;
    message[6] = 0x30;
    send(0x1D4, 8);
    if (counter50b++ >= 9) {
        counter50b = 0;
        message[0] = 0;
        message[1] = 0;
        message[2] = 0x06;
        message[3] = 0xC0;
        message[4] = 0;
        message[5] = 0;
        message[6] = 0;
        send(0x50B, 7);
    }
    counter++;
    if (counter >= 4) counter = 0;
}

void send(int id, size_t size) {
    if (size == 8)
        nissan_crc(message);
    CAN.beginPacket(id);
    CAN.write(message, size);
    CAN.endPacket();
}

unsigned long lastPrint = millis();

void parseCAN() {// try to parse packet
    int packetSize = CAN.parsePacket();

    if (packetSize) {
        if (CAN.packetId() == 0x1DA) {
            if (millis() - lastPrint < 50) {
                while (CAN.read() != -1);
                return;
            }
            lastPrint = millis();
            uint8_t buffer[8];
            CAN.readBytes(buffer, 8);

            unsigned int voltage = buffer[0] * 2;
            unsigned int amperage = (((buffer[2] & 0x7) << 8) | buffer[3]);
            int rpm = ((buffer[4] << 8) | buffer[5]) / 2;

            Serial.println();
            Serial.print("U = ");
            Serial.print(voltage);
            Serial.println("V");
            Serial.print("I = ");
            Serial.print(amperage);
            Serial.println("A");
            Serial.print("V = ");
            Serial.print(rpm);
            Serial.println(" RPM");
            Serial.print("E = ");
            Serial.print(buffer[6] >> 4, 2);
            Serial.println();
            Serial.println();
            return;
        }
        if (CAN.packetId() == 0x55A) {
            CAN.read();
            int motorTemp = CAN.read();
            int inverterTemp = CAN.read();
            Serial.println();
            Serial.print("Temp Motor = ");
            Serial.print(motorTemp);
            Serial.println("F");
            Serial.print("Temp Invtr = ");
            Serial.print(inverterTemp);
            Serial.println("F");
            return;
        }
        // received a packet
        Serial.print("Received ");

        if (CAN.packetExtended()) {
            Serial.print("extended ");
        }

        if (CAN.packetRtr()) {
            // Remote transmission request, packet contains no data
            Serial.print("RTR ");
        }

        Serial.print("packet with id 0x");
        Serial.print(CAN.packetId(), HEX);

        if (CAN.packetRtr()) {
            Serial.print(" and requested length ");
            Serial.println(CAN.packetDlc());
        } else {
            Serial.print(" and length ");
            Serial.println(packetSize);

            // only print packet data for non-RTR packets
            while (CAN.available()) {
                /*Serial.print*/((char) CAN.read());
            }
            Serial.println();
        }

        Serial.println();
    }
}


static void nissan_crc(uint8_t *data) {
    data[7] = 0;
    uint8_t crc = 0;
    for (int b = 0; b < 8; b++) {
        for (int i = 7; i >= 0; i--) {
            uint8_t bit = ((data[b] & (1 << i)) > 0) ? 1 : 0;
            if (crc >= 0x80) crc = (byte) (((crc << 1) + bit) ^ 0x85);
            else crc = (byte) ((crc << 1) + bit);
        }
    }
    data[7] = crc;
}

