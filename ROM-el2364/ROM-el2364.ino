#include "ROM-el2364.hpp"

Mode mode = READ;

void printMessage(String message){
  Serial.write(message.length()+2);
  Serial.println(message);
}

void setup() {
  pinMode(ADDROE, OUTPUT);
  digitalWrite(ADDROE, HIGH);
  pinMode(ADDRCLK, OUTPUT);
  digitalWrite(ADDRCLK, LOW);
  pinMode(ADDRSER, OUTPUT);
  digitalWrite(ADDRSER, LOW);
  pinMode(DATAINOE, OUTPUT);
  digitalWrite(DATAINOE, LOW);
  pinMode(DATAOUTOE, OUTPUT);
  digitalWrite(DATAOUTOE, HIGH);
  pinMode(DATACLK, OUTPUT);
  digitalWrite(DATACLK, LOW);
  pinMode(DATAOUTSER, OUTPUT);
  digitalWrite(DATAOUTSER, LOW);
  pinMode(DATAINSER, INPUT_PULLUP);
  pinMode(EEPROMOE, OUTPUT);
  digitalWrite(EEPROMOE, HIGH);
  pinMode(EEPROMWE, OUTPUT);
  digitalWrite(EEPROMWE, HIGH);

  pinMode(RSTROMEL, OUTPUT);
  digitalWrite(RSTROMEL, HIGH);
  pinMode(ADDR16, OUTPUT);
  digitalWrite(ADDR16, HIGH);

  Serial.begin(115200);
  Serial.setTimeout(3000);
  Serial.print("INIT");
  //Let's do some byte transfers!
}

void setReadMode(){
  if( mode == READ )
    return;
  //Disable shift register (write)
  DATAOUTOE_Set(HIGH);//digitalWrite(DATAOUTOE, HIGH);
  mode = READ;
}

void setWriteMode(){
  if( mode == WRITE )
    return;
  //Disable shift-in register (read)
  DATAINOE_Set(LOW);//digitalWrite(DATAINOE, LOW);
  mode = WRITE;
}

//Shift byte out to SN74HC595 (with tied data lines)
void shiftByteOut(byte data, bool noclock, int clkpin, int serpin, int oepin)
{
  if( oepin == DATAOUTOE )
    digitalWrite(DATAOUTOE, HIGH);
  else
    digitalWrite(oepin, HIGH);

  byte tmpdata = data;

  //Shift MSB-first
  for ( int i = 0; i < 8; i++ )
  {
    digitalWrite(serpin, tmpdata >> 7);
    digitalWrite(clkpin, HIGH);
    digitalWrite(clkpin, LOW);
    tmpdata = tmpdata << 1;
  }
  digitalWrite(serpin, LOW);
  //With tied clock lines, an additional clock pulse is necessary after shifting
  //the very last bit to all shift registers in order to output expected data.
  if(! noclock)
  {
    //Additional clock due to tied RCLK/SRCLK lines
    digitalWrite(clkpin, HIGH);
    digitalWrite(clkpin, LOW);
  }
  if( oepin == DATAOUTOE )
    digitalWrite(DATAOUTOE, LOW);
  else
    digitalWrite(oepin, LOW);
}

//Shift byte in from SN74HC165
uint8_t shiftByteIn(int clkpin, int serpin, int latchpin, short bits = 8){//return shiftByteIn(DATACLK, DATAINSER, DATAINOE, bits);
  uint8_t input = 0;
  DATACLK_Set(HIGH);//digitalWrite(clkpin, HIGH);

  if( latchpin == DATAINOE )
    DATAINOE_Set(LOW);//digitalWrite(DATAINOE, LOW);
  else
    digitalWrite(latchpin, LOW);

  if( latchpin == DATAINOE )
    DATAINOE_Set(HIGH);//digitalWrite(DATAINOE, HIGH);
  else
    digitalWrite(latchpin, HIGH);
    
  //Data shift in MSB-first
  for ( int i = 0; i < bits; i++ ){
    DATACLK_Set(HIGH);//digitalWrite(clkpin, HIGH);
    //Don't clock first, 74HC165 outputs first byte as soon as latch goes high
    __builtin_avr_delay_cycles(5);//TODO: Possible less delay
    input |= DATAINSER_Read() << ((bits-1)-i);//input |= digitalRead(serpin) << ((bits-1)-i);
    DATACLK_Set(LOW);//digitalWrite(clkpin, LOW);
  }
  DATAINOE_Set(LOW);//digitalWrite(DATAINOE, LOW);
  return input;
}

//Write to address lines
void shiftAddress(uint32_t addrin){
  uint16_t addr = addrin & 0x0000FFFF;
  uint8_t out[2];

  out[0]=(addr>>8)&0xFF;
  out[1]=0xFF&addr;

  if(addrin > 0xFFFF){
    digitalWrite(ADDR16,HIGH);
  }
  else{
    digitalWrite(ADDR16,LOW);
  }

  shiftByteOut(out[0], true, ADDRCLK, ADDRSER, ADDROE);
  shiftByteOut(out[1], false, ADDRCLK, ADDRSER, ADDROE);
}

//Write to data lines
void shiftData(uint8_t data){
  setWriteMode();
  shiftByteOut(data, false, DATACLK, DATAOUTSER, DATAOUTOE);
}

//Shift data lines in
//Arguments:
//bits: How many bits to shift in (useful in waitForToggleBit() so that cycles
//      aren't wasted shifting in bits we don't care about.)
uint8_t shiftInData(short bits = 8){
  setReadMode();
  return shiftByteIn(DATACLK, DATAINSER, DATAINOE, bits);
}

//Never could get this to work properly, but waitForToggleBit() works, so...
/*void dataPoll(unsigned short addr, byte data)
{
  setReadMode();
  digitalWrite(EEPROMOE, LOW);
  __builtin_avr_delay_cycles(1);
  bool validdata = data & 0x80;
  while( true )
  {
    digitalWrite(EEPROMOE, LOW);
    byte addressread = shiftInData();
    addressread &= 0x80;
    digitalWrite(EEPROMOE, HIGH);
    if( addressread != validdata )
      continue;
    else
      break;
  }
}*/

//Implementation of "AC Load" from AT49F512 datasheet
void ACLoad(uint32_t addrin, uint8_t data){
  setWriteMode();
  EEPROMOE_Set(HIGH);//digitalWrite(EEPROMOE, HIGH);
  shiftAddress(addrin);
  EEPROMWE_Set(LOW);//digitalWrite(EEPROMWE, LOW);
  __builtin_avr_delay_cycles(2); //Delay an extra cycle to satisfy tWP = 90ns
  shiftData(data);
  EEPROMWE_Set(HIGH);//digitalWrite(EEPROMWE, HIGH);
  __builtin_avr_delay_cycles(2); //Delay cycles to satisfy tWPH = 90ns
}

//Implementation of "AC Read" from AT49F512 datasheet
uint8_t ACRead(uint32_t addrin){
  setReadMode();
  EEPROMWE_Set(HIGH);//digitalWrite(EEPROMWE, HIGH);
  shiftAddress(addrin);
  EEPROMOE_Set(LOW);//digitalWrite(EEPROMOE, LOW);
  __builtin_avr_delay_cycles(2);
  uint8_t input = shiftInData();
  EEPROMOE_Set(HIGH);//digitalWrite(EEPROMOE, HIGH);
  return input;
}

uint8_t writeAddress(uint32_t addrin, uint8_t data){
  ACLoad(0x5555, 0xAA);
  ACLoad(0x2AAA, 0x55);
  ACLoad(0x5555, 0xA0);
  ACLoad(addrin, data);
  digitalWrite(EEPROMOE, LOW);
  waitForToggleBit();
  return 0x00;
}

void eraseChip(){
  digitalWrite(EEPROMOE, HIGH);
  ACLoad(0x5555, 0xAA);
  ACLoad(0x2AAA, 0x55);
  ACLoad(0x5555, 0x80);
  ACLoad(0x5555, 0xAA);
  ACLoad(0x2AAA, 0x55);
  ACLoad(0x5555, 0x10);
  waitForToggleBit();
}

//This Atmel chip, during a program or erase cycle will implement a "toggle bit."
//Per the datasheet:
//"During a program or erase operation, successive attempts to read data from the device
//will result in I/O6 toggling between one and zero. Once the program cycle has completed,
//I/O6 will stop toggling and valid data will be read."
void waitForToggleBit(){
  return;
  uint8_t initial = 0;
  uint8_t nextbit = 0;
  setReadMode();
  EEPROMOE_Set(LOW);//digitalWrite(EEPROMOE, LOW);
  //Store bit 7 (I/O 6)
  //byte initial = shiftInData(2) & 0x40;
  initial = shiftInData(2) & 0x40;
  while( true ){
    EEPROMOE_Set(HIGH);//digitalWrite(EEPROMOE, HIGH);

    //Needs to be high for 150ns, One cycle is ~62.5ns
    //At 16MHz this is a delay of ~187.5ns
    //However, a digital write already takes 3.4us = 3,400ns!!
    //dont' wait.
    __builtin_avr_delay_cycles(3);
    EEPROMOE_Set(LOW);//digitalWrite(EEPROMOE, LOW);
    
    //Check bit 7 again
    nextbit = shiftInData(2) & 0x40;
    //If bit 7 is stable, the write/erase has finished.
    if( nextbit == initial )
      break;
    else
      initial = nextbit;
  }
}

void verifyRange(uint32_t startaddr, uint32_t endaddr){
  for (uint32_t i = startaddr; i <= endaddr; i++ )
  {
    Serial.write(0x01);
    Serial.write(ACRead(i));
  }
}

void loop() {
  uint8_t data;
  uint8_t serialbuffer[10];
  memset(serialbuffer, 0x00, sizeof(serialbuffer));
  /*Protocol definition:
  <packet> ::= <length> <command> | <length> <command> <data>
  <data> ::= <address> | <address> <byte>
  <command> ::= 'R' | 'W' | 'E' comment <erase command>
  <byte> ::= <a single byte>
  <address> ::= <4-byte address>
  <length> ::= <1-byte length of packet (excluding length byte itself)>

  This allows two modes of operation:
  In the first mode (i.e. <length>R\x01\x00), the controlling application
  specifies all addresses (and, in write mode, the data to be stored). In the
  second mode (i.e. <length>R) bytes are read/written sequentially, using the
  address variable stored on the Arduino.

  If a program wanted to read 8 bytes starting at 0x0100, either of these
  command sets could be used:
  R\x01\x00
  R\x01\x01
  R\x01\x02
  R\x01\x03
  R\x01\x04
  R\x01\x05
  R\x01\x06
  R\x01\x07

  or

  R\x01\x00 //Read 7 sequential addresses
  R
  R
  R
  R
  R
  R
  R
  */

  while ( !Serial.available() )
    continue;

  Serial.readBytes(serialbuffer, 1);
  uint8_t packetlen = serialbuffer[0];

  while ( Serial.available() != packetlen )
    continue;

  Serial.readBytes(serialbuffer, packetlen);

  //Read mode
  if ( serialbuffer[0] == 'R' ){
    setReadMode();
    //No address given
    if ( packetlen == 1 ){
      data = ACRead(address);
      Serial.write(0x01);
      Serial.write(data);
    }
    //Address given
    else{
      uint32_t packetaddress = serialbuffer[1];
      packetaddress <<= 8;
      packetaddress += serialbuffer[2];
      packetaddress <<= 8;
      packetaddress += serialbuffer[3];
      packetaddress <<= 8;
      packetaddress += serialbuffer[4];
      address = packetaddress;

      data = ACRead(packetaddress);
      Serial.write(0x01);
      Serial.write(data);
    }
    address++;
  }
  //Write mode
  else if ( serialbuffer[0] == 'W' ){
    unsigned short retval;
    setWriteMode();
    //No address given
    if ( packetlen == 2 ){
      retval = writeAddress(address, serialbuffer[1]);
    }
    //Address given
    else{
      uint32_t packetaddress = serialbuffer[1];
      packetaddress <<= 8;
      packetaddress += serialbuffer[2];
      packetaddress <<= 8;
      packetaddress += serialbuffer[3];
      packetaddress <<= 8;
      packetaddress += serialbuffer[4];
      address = packetaddress;

      uint8_t retval = writeAddress(address, serialbuffer[5]);
    }
    Serial.write(0x01);
    Serial.write(retval);
    address++;
  }
  //Erase mode
  else if ( serialbuffer[0] == byte('E') ){
    mode = ERASE;
    eraseChip();
    Serial.write(0x01);
    Serial.write(0x01);
  }
  //Verify mode (UNTESTED)
  else{
    mode = VERIFY;
    unsigned short startAddress = serialbuffer[1] << 8;
    startAddress += serialbuffer[2];
    unsigned short endAddress = serialbuffer[3] << 8;
    endAddress = serialbuffer[4];
    verifyRange(startAddress, endAddress);
  }
}
