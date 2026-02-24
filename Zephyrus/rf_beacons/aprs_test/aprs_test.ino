#include <Arduino.h>
#include <math.h>

/******** USER SETTINGS ********/

#define CALLSIGN "KR4GAM"
#define DEST     "APRS"

#define APRS_MSG "!4221.39N/07105.71W>STM32 APRS Tracker"

#define SAMPLE_RATE 9600//38400
#define BAUD 1200
#define BEACON_INTERVAL 5000   // ms

/********************************/

HardwareSerial radio(PA10, PA9); // RX, TX

void configDRA818()
{
  radio.begin(9600);
  delay(500);

  radio.println(
    "AT+DMOSETGROUP=0,144.3900,144.3900,0000,1,0000"
  );

  delay(200);
}

HardwareTimer *timer;

/******** DDS AUDIO ********/

#define SINE_SIZE 256
uint16_t sineTable[SINE_SIZE];

volatile uint32_t phase=0;
uint32_t incMark, incSpace;
volatile uint32_t currentInc;

volatile int samplesPerBit=SAMPLE_RATE/BAUD;
volatile int sampleCount=0;

/******** BITSTREAM ********/

#define MAX_BITS 6000
volatile uint8_t bitstream[MAX_BITS];
volatile int bitLen=0;
volatile int bitPos=0;

volatile bool transmitting=false;
volatile int ones=0;

/******** CRC ********/

uint16_t crc_ccitt(uint8_t *data,int len)
{
  uint16_t crc=0xFFFF;

  for(int i=0;i<len;i++){
    crc^=data[i];
    for(int j=0;j<8;j++)
      crc=(crc&1)?(crc>>1)^0x8408:crc>>1;
  }
  return ~crc;
}

/******** AX25 ADDRESS ********/

void encodeCall(uint8_t *out,
                const char *call,
                uint8_t ssid,
                bool last)
{
  int len = strlen(call);

  for(int i=0;i<6;i++){
    char c = (i < len) ? toupper(call[i]) : ' ';
    out[i] = c << 1;
  }

  out[6] = ((ssid & 0x0F) << 1) | 0x60;

  if(last)
    out[6] |= 0x01;
}

/******** BIT ADD ********/

void addRawBit(uint8_t b)
{
  if(bitLen<MAX_BITS)
    bitstream[bitLen++]=b;
}

void addBit(uint8_t b)
{
  addRawBit(b);

  if(b){
    ones++;
    if(ones==5){
      addRawBit(0);
      ones=0;
    }
  } else ones=0;
}

void addByte(uint8_t b)
{
  for(int i=0;i<8;i++)
    addBit((b>>i)&1);
}

void addFlag()
{
  uint8_t f = 0x7E;

  ones = 0;

  for(int i=0;i<8;i++)
    addRawBit((f>>i)&1);
}

/******** BUILD APRS ********/

void buildPacket()
{
  Serial.println("BP");
  uint8_t frame[256];
  int idx=0;

  encodeCall(frame+idx,"APRS",0,false);
  idx+=7;

  encodeCall(frame+idx,CALLSIGN,0,false);
  idx+=7;

  encodeCall(frame+idx,"WIDE1",1,false);
  idx+=7;

  encodeCall(frame+idx,"WIDE2",1,true);
  idx+=7;

  Serial.println("BP1");

  frame[idx++]=0x03;
  frame[idx++]=0xF0;

  const char *p=APRS_MSG;
  while(*p) frame[idx++]=*p++;

  uint16_t crc=crc_ccitt(frame,idx);
  frame[idx++]=crc&0xFF;
  frame[idx++]=crc>>8;

  bitLen=0;
  ones=0;

  Serial.println("BP2");

  for(int i=0;i<60;i++)
    addFlag();

  ones = 0;
  for(int i=0;i<idx;i++)
    addByte(frame[i]);

  addFlag();

  Serial.println("BP3");

  //Serial.write(frame, idx);
  for(int i=0;i<idx;i++){
  if(frame[i] < 16) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  bitPos=0;
  digitalWrite(PB2, 0);
  delay(200);
  transmitting=true;
}

/******** NRZI ********/

void nextBit()
{
  if(bitPos>=bitLen){
    transmitting=false;
    digitalWrite(PB2, 1);
    return;
  }

  uint8_t bit=bitstream[bitPos++];

  if(bit==0)
    currentInc=
      (currentInc==incMark)?
      incSpace:incMark;
}

/******** AUDIO ISR ********/

void audioISR()
{
  if(transmitting){
    if(++sampleCount >= samplesPerBit){
      sampleCount = 0;
      nextBit();
    }

    phase += currentInc;
    uint8_t idx = phase >> 24;
    analogWrite(PA4, sineTable[idx] >> 4);
  }
  else{
    analogWrite(PA4, 128);   // silence (midpoint)
  }
}

/******** SETUP ********/

void setup()
{
  pinMode(PB2, OUTPUT);
  digitalWrite(PB2, 1);
  pinMode(PB3, OUTPUT);
  digitalWrite(PB3, 1);

  Serial.begin(115200);
  delay(5000);
  Serial.println("BOOT");

  configDRA818();
  delay(1000);
  while(radio.available()) {
    Serial.write(radio.read());
  }
  analogWriteResolution(8);

  for(int i=0;i<SINE_SIZE;i++)
    sineTable[i]=(sin(2*PI*i/SINE_SIZE)*2047)+2048;

  incMark=(uint32_t)((1200.0*4294967296.0)/SAMPLE_RATE);
  incSpace=(uint32_t)((2200.0*4294967296.0)/SAMPLE_RATE);

  currentInc=incMark;

  timer=new HardwareTimer(TIM2);
  timer->setOverflow(SAMPLE_RATE,HERTZ_FORMAT);
  timer->attachInterrupt(audioISR);
  timer->resume();
  //Serial.begin(115200);
  //delay(5000);
  //Serial.println("BOOT");
  //transmitting = true;
  buildPacket();
}

/******** LOOP ********/

uint32_t lastBeacon=0;

void loop()
{
  if(millis()-lastBeacon>BEACON_INTERVAL){
    lastBeacon=millis();
    buildPacket();
  }
}