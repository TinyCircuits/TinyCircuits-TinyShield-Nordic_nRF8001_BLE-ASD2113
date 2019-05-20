/*
  TinyCircuits NRF8001 BLE TinyShield Example Sketch

  This demo sets up the NRF8001 for Nordic's BLE virtual UART connection, and
  can pass data between the Arduino serial monitor and Nordic nRF UART V2.0 app
  or another compatible BLE terminal. Now supports TinyScreen+/SAMD21 boards.

  Written 27 May 2016
  By Ben Rose
  Modified 07 January 2019
  By Hunter Hykes

  https://TinyCircuits.com
*/

//when using this project in the Arduino IDE, set the following to false and rename file UART.h to UART.ino
#define CODEBENDER true

#if defined(ARDUINO_ARCH_SAMD)
  #define SerialMonitorInterface SerialUSB
#else
  #define SerialMonitorInterface Serial
#endif

#define BLE_DEBUG true

#include <SPI.h>
#include "lib_aci.h"
#include "aci_setup.h"
#include "uart_over_ble.h"
#include "services.h"

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;

#if CODEBENDER
#include "UART.h"
#endif


void setup(void)
{
  SerialMonitorInterface.begin(9600);
  //while (!SerialMonitorInterface);//Optional- On TinyScreen+/SAMD21 boards this will block until the serial monitor is opened
  delay(100);
  BLEsetup();
}

void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  if (ble_rx_buffer_len) {//Check if data is available
    SerialMonitorInterface.print(ble_rx_buffer_len);
    SerialMonitorInterface.print(" : ");
    SerialMonitorInterface.println((char*)ble_rx_buffer);
    ble_rx_buffer_len = 0;//clear afer reading
  }
  if (SerialMonitorInterface.available()) {//Check if serial input is available to send
    delay(50);//should catch full input
    uint8_t sendBuffer[21];
    uint8_t sendLength = 0;
    while (SerialMonitorInterface.available() && sendLength < 19) {
      sendBuffer[sendLength] = SerialMonitorInterface.read();
      sendLength++;
    }
    if (SerialMonitorInterface.available()) {
      SerialMonitorInterface.print(F("Input truncated, dropped: "));
      while (SerialMonitorInterface.available()) {
        SerialMonitorInterface.write(SerialMonitorInterface.read());
      }
      SerialMonitorInterface.println();
    }
    sendBuffer[sendLength] = '\0'; //Terminate string
    sendLength++;
    if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)sendBuffer, sendLength))
    {
      SerialMonitorInterface.println(F("TX dropped!"));
    }
  }

}
