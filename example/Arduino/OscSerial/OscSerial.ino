#include <OSCBundle.h>
#include <PacketSerial.h>
#include <BufferPrinter.h>


PacketSerial_<SLIP, SLIP::END, 8192> serial;


const byte LED_PIN = 13;
unsigned long lastMessage = 0;


struct Route
{
    const char* pattern;
    void (*callback)(OSCMessage&);
};


Route routes[1];


void setup() {
  serial.setPacketHandler(&onPacket);
  serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  routes[0] = { "/led",  LEDcontrol };

}


void loop() {
  serial.update();

  auto now = millis();

  if (lastMessage + 1000 < now)
  {
      static uint8_t b[256];
      BufferPrinter printer(b, 256);

      OSCBundle bundle;
      bundle.add("/hello").add(int32_t(now));

      bundle.send(printer);
      serial.send(b, printer.getOffset());

      lastMessage = now;
  }
}

void onPacket(const uint8_t* buffer, size_t size){
  OSCBundle bundleIN;

  for (size_t i = 0; i < size; i++){
    bundleIN.fill(buffer[i]);
  }

  size_t numRoutes = sizeof(routes) / sizeof(routes[0]);

  if (!bundleIN.hasError())
  {
    for (size_t i = 0; i < numRoutes; i++)
    {
        bundleIN.dispatch(routes[i].pattern, routes[i].callback);
    }
  }
}


void LEDcontrol(OSCMessage& msg)
{
  if (msg.isInt(0))
  {
    digitalWrite(LED_PIN, msg.getInt(0));
  }
}

