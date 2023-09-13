//EEPROMCE is always low!
/*-----------------------------*/
const int EEPROMWE = 2;   //PD2
const int EEPROMOE = 3;   //PD3
/*-----------------------------*/
const int ADDROE = 5;     //PD5
const int ADDRCLK = 6;    //PD6
const int ADDRSER = 7;    //PD7
/*-----------------------------*/
const int DATAINOE = 4;   //PD4
const int DATAOUTOE = 9;  //PB1
const int DATACLK = 10;   //PB2
const int DATAOUTSER = 11;//PB3
const int DATAINSER = 8;  //PB0
/*-----------------------------*/
const int RSTROMEL = 12;  //PB6
const int ADDR16 = 13;    //PB6
/*-----------------------------*/
const int FAST = 0;
uint32_t address = 0x00000000;

enum Mode {
  READ = LOW,
  WRITE = HIGH,
  ERASE,
  VERIFY
};

/*--------------EEPROM PINS--------------------*/
inline void EEPROMWE_Set(bool state){/*EEPROMWE = 2 //PD2*/
    if(state!=LOW){
      if(FAST)
        PORTD |= 0b00000100;
      else
        digitalWrite(EEPROMWE, HIGH);
    }
    else{
      if(FAST)
        PORTD &= 0b11111011;
      else
        digitalWrite(EEPROMWE,LOW);
    }
}
inline void EEPROMOE_Set(bool state){/*EEPROMOE = 3;   //PD3*/
   if(state!=LOW){
    if(FAST)
      PORTD |= 0b00001000;
    else
      digitalWrite(EEPROMOE,HIGH);
   }
   else{
    if(FAST)
      PORTD &= 0b11110111;
    else
      digitalWrite(EEPROMOE,LOW);
   }
}
/*--------------Addr PINS----------------------*/
inline void ADDROE_Set(bool state){/*ADDROE = 5;   //PD5*/
   if(state!=LOW){
    if(FAST)
      PORTD |= 0b00100000;
    else
      digitalWrite(ADDROE,HIGH);
   }
   else{
    if(FAST)
      PORTD &= 0b11011111;
    else
      digitalWrite(ADDROE,LOW);
   }
}
inline void ADDRCLK_Set(bool state){/*ADDRCLK = 6;   //PD6*/
   if(state!=LOW){
    if(FAST)
      PORTD |= 0b01000000;
    else
      digitalWrite(ADDRCLK,HIGH);
   }
   else{
    if(FAST)
      PORTD &= 0b10111111;
    else
      digitalWrite(ADDRCLK, LOW);
   }
}
inline void ADDRSER_Set(bool state){/*ADDRSER = 7;   //PD7*/
   if(state!=LOW){
    if(FAST)
      PORTD |= 0b10000000;
    else
      digitalWrite(ADDRSER,HIGH);
   }
   else{
    if(FAST)
      PORTD &= 0b01111111;
    else
      digitalWrite(ADDRSER,LOW);
   }
}
/*--------------Dat PINS------------------------*/
inline void DATAINOE_Set(bool state){/*DATAINOE = 4;   PD4*/
   if(state!=LOW){
    if(FAST)
      PORTD |= 0b00010000;
    else
      digitalWrite(DATAINOE,HIGH);
   }
   else{
    if(FAST)
      PORTD &= 0b11101111;
    else
      digitalWrite(DATAINOE,LOW);
   }
}
inline void DATAOUTOE_Set(bool state){/* DATAOUTOE = 9;  PB1*/
   if(state!=LOW){
    if(FAST)
      PORTB |= 0b00000010;
    else
      digitalWrite(DATAOUTOE,HIGH);
   }
   else{
    if(FAST)
      PORTB &= 0b11111101;
    else
      digitalWrite(DATAOUTOE,LOW);
   }
}
inline void DATACLK_Set(bool state){/*DATACLK = 10;   PB2*/
   if(state!=LOW){
    if(FAST)
      PORTB |= 0b00000100;
    else
      digitalWrite(DATACLK,HIGH);
   }
   else{
    if(FAST)
      PORTB &= 0b11111011;
    else
      digitalWrite(DATACLK,LOW);
   }
}
inline void DATAOUTSER_Set(bool state){/*DATAOUTSER = 11;//PB3*/
   if(state!=LOW){
    if(FAST)
      PORTB |= 0b00001000;
    else
      digitalWrite(DATAOUTSER,HIGH);
   }
   else{
    if(FAST)
      PORTB &= 0b11110111;
    else
      digitalWrite(DATAOUTSER,LOW);
   }
}
inline uint8_t DATAINSER_Read(){/*DATAINSER = 8;  //PB0*/
  uint8_t retVal;
  if(FAST)
    retVal = PINB &= 0b00000001;
  else
    retVal = digitalRead(DATAINSER);

  if(retVal)
    return 1;
  else
    return 0;

}