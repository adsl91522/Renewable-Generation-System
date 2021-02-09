#include "string.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>  
#include <XBee.h>        

LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display         
XBee xbee = XBee();

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40E66E6A);


void setup()
{  
  Serial.begin(9600);
  xbee.begin(Serial);
  lcd.init();
  lcd.backlight();
}

void loop()
{
  double Vrms = (analogRead(0)*5.0)/1023;
  double Irms = Vrms / 10.0;
  double Pow = Irms * Vrms;
  char t_Vrms[7], t_Irms[6], t_Pow[6];
  char temp[25];

//  if(Irms <= 0.2 || Irms > 20){       //filter the arduino self current
//    Vrms = 0;
//    Irms = 0;
//    Pow = 0;
//  }
  
  /* float to char */
  dtostrf(Vrms, 5, 2, t_Vrms);    
  dtostrf(Irms, 4, 2, t_Irms);
  dtostrf(Pow, 4, 2, t_Pow);
  
  /* dev id = 3 */
  sprintf(temp, "3,%s,%s,%s", t_Vrms, t_Irms, t_Pow); 
  
  /* LCD showing */
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(t_Vrms);         
  lcd.print(",I:");
  lcd.print(t_Irms);  
  lcd.setCursor(0, 1);           
  lcd.print("P:");
  lcd.print(t_Pow);  

  /* Xbee serial transmit */
  ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t*)temp, strlen(temp));
  xbee.send(zbTx);	
//  Serial.println(temp);
  
  delay(1000);
}
