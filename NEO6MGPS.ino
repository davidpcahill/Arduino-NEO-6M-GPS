#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

// Define constants for clarity
const int GPS_TX_PIN = 10;
const int GPS_RX_PIN = 11;

SoftwareSerial mySerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPSPlus gps;  // Use the upgraded library object type

void displayGPSInfo();
void printFormattedFloat(double number, int digits = 2);

// Configurable settings
const int timeZoneOffset = -8;  // Default for Pacific Time
const char* timeZoneName = "UTC -08:00 Pacific";  // Name or description of the time zone
const unsigned long UPDATE_INTERVAL_MS = 1000;  // Update rate in milliseconds. Default: 1 second

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(5000);
  Serial.println("uBlox Neo 6M with TinyGPS++");
  Serial.println("----------------------------");
}

void loop() {
  bool newData = false;  
  unsigned long start = millis();

  while (millis() - start < UPDATE_INTERVAL_MS) {
    while (mySerial.available()) {
      char c = mySerial.read();
      if (gps.encode(c)) {
        if (gps.location.isValid() && !newData) {  
          displayGPSInfo();
          newData = true;
        }
      }
    }
  }

  if (!newData) {
    Serial.println("Waiting for GPS signal...");
  }
}

byte daysInMonth(byte month, int year) {
  switch(month) {
    case 2:
      if(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) return 29; // Leap year check
      return 28;
    case 4: case 6: case 9: case 11:
      return 30;
    default:
      return 31;
  }
}

void displayGPSInfo() {
  Serial.println("GPS Data:");
  Serial.println("---------");

  // Latitude and Longitude
  if (gps.location.isValid()) {
    Serial.print("Latitude:  "); printFormattedFloat(gps.location.lat(), 5); Serial.println("°");
    Serial.print("Longitude: "); printFormattedFloat(gps.location.lng(), 5); Serial.println("°");
  } else {
    Serial.println("Location: Not available");
  }

  // Date and Time
  if (gps.date.isValid() && gps.time.isValid()) {
    int year = gps.date.year();
    byte month = gps.date.month();
    byte day = gps.date.day();
    byte hour = gps.time.hour();
    byte minute = gps.time.minute();
    byte second = gps.time.second();
    byte hundredths = 0; // TinyGPS++ does not provide hundredths, so default to zero

    int localHour = hour + timeZoneOffset;

    if (localHour < 0) {
      localHour += 24;
      day--;
      if (day == 0) {
        month--;
        if (month == 0) {
          month = 12;
          year--;
        }
        day = daysInMonth(month, year);
      }
    } else if (localHour >= 24) {
      localHour -= 24;
      day++;
      if (day > daysInMonth(month, year)) {
        day = 1;
        month++;
        if (month > 12) {
          month = 1;
          year++;
        }
      }
    }

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Date: %02d/%02d/%d", month, day, year);
    Serial.println(buffer);
    snprintf(buffer, sizeof(buffer), "Time (%s): %02d:%02d:%02d.%02d", timeZoneName, localHour, minute, second, hundredths);
    Serial.println(buffer);
  } else {
    Serial.println("Date & Time: Not available");
  }


  // Satellites and HDOP (if your module supports these features)
  if (gps.satellites.isValid()) {
    Serial.print("Satellites: "); Serial.println(gps.satellites.value());
  }
  if (gps.hdop.isValid()) {
    Serial.print("HDOP: "); Serial.println(gps.hdop.value());
  }

  // Altitude
  if (gps.altitude.isValid()) {
    Serial.print("Altitude: "); printFormattedFloat(gps.altitude.meters(), 2); Serial.println(" meters");
  } else {
    Serial.println("Altitude: Not available");
  }

  // Speed
  if (gps.speed.isValid()) {
    Serial.print("Speed: "); printFormattedFloat(gps.speed.kmph(), 2); Serial.println(" km/h");
  } else {
    Serial.println("Speed: Not available");
  }

  Serial.println("---------");
  Serial.println();
}

void printFormattedFloat(double number, int digits) {
  if (number < 0.0) {
    Serial.print('-');
    number = -number;
  }

  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i) {
    rounding /= 10.0;
  }

  number += rounding;

  unsigned long intPart = (unsigned long) number;
  double remainder = number - (double) intPart;
  Serial.print(intPart);

  if (digits > 0) {
    Serial.print(".");
  }

  while (digits-- > 0) {
    remainder *= 10.0;
    int toPrint = int(remainder);
    Serial.print(toPrint);
    remainder -= toPrint;
  }
}