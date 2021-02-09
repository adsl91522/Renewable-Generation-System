#include <XBee.h>

XBee xbee = XBee();
ZBRxResponse rx = ZBRxResponse();

void setup() {
  // start serial
  Serial.begin(9600);
  xbee.begin(Serial);
  
}

void loop() {
    xbee.readPacket(); 
    if (xbee.getResponse().isAvailable()) 
    {  
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) 
      {
        xbee.getResponse().getZBRxResponse(rx);
        for(int i=0; i<rx.getDataLength(); i++)
        {
          if(rx.getData(i) == 1){
            Serial.print("wind speed,");
            Serial.println(rx.getData(++i));
          }else if(rx.getData(i) == 2){
            Serial.print("illuminance,");
            Serial.println(rx.getData(++i));
          }else if(rx.getData(i) == '3'){
            Serial.print("meter");
            for(i=1; i<rx.getDataLength(); i++){
              Serial.print((char)rx.getData(i));
            }
            Serial.println();
            break;
          }
        }
      }else if (xbee.getResponse().isError()) {
        Serial.println("Error reading packet.  Error code: ");  
        Serial.println(xbee.getResponse().getErrorCode());
      } 
    }
}
