#include <XBee.h>

XBee xbee = XBee();
ZBRxResponse rx = ZBRxResponse();

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40E66E3E);

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
        ZBTxRequest zbTx = ZBTxRequest(addr64, rx.getData(), rx.getDataLength());
        xbee.send(zbTx);

      }else if (xbee.getResponse().isError()) {
        Serial.println("Error reading packet.  Error code: ");  
        Serial.println(xbee.getResponse().getErrorCode());
      } 
    }
}
