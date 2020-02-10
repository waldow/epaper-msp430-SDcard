#include <SPI.h>
//#include "pfatfs.h"
//#include "pffconf.h"
#include "epd2in7b.h"

#include "SPI.h" 




#define CSSD_PIN      8             // chip select for sd card pin  . 0=on 1=off
#define ENABLE_BOOST_PIN     9      // enable boost to 3.3v  pin . 0=off 1=on
#define ENABLE_SD_PIN      6            // switch on mosfet to power sd. 0=on 1=off
#define read_buffer 128             // size (in bytes) of read buffer 
#define MIN_LEVEL 10 // 720 // 30 // 720 // 1440
#define BAT_LEVEL   5
#define TMO_LEVEL   5
volatile uint8_t readyDisplay=0;
volatile uint8_t showPic =0;
volatile uint8_t batCntDown =BAT_LEVEL;
volatile unsigned int minCntDown =0;
volatile uint8_t timeoutCntDown =0;

char bufferData[read_buffer];
int rc;

uint32_t ui32_ReadTemp = 0;
uint8_t StringLength = 0;


volatile uint32_t AccStringLength = 0;

unsigned int currentMinutes, currentSeconds;
unsigned int wdtCounter = 0;
volatile unsigned int picCounter = 0;
volatile bool doLog = true;
char charno[5];
char filename1[12];
//char filenamer[12];


Epd epd;
#define F_CPU  8000000

int analogIn;
uint16_t supplyVolts;
bool batLow =false;
bool r1=false;


void unusePin(int pin)
{
  pinMode(pin,INPUT_PULLUP); //,INPUT_PULLDOWN);//  INPUT_PULLUP);
  
}
// Set most of the pins to input pullup.  To minimize the current draw
void unusedPins()
{
  unusePin(P1_0);
 if(!doLog)
 {
  unusePin(P1_1);
  unusePin(P1_2);
 }
  unusePin(P1_3);
  //unusePin(P1_4);
  pinMode(P1_4,INPUT_PULLUP);
 // unusePin(P1_5);
  //unusePin(P1_6);
 // unusePin(P1_7);
 // unusePin(P2_0);
  pinMode(P2_0,INPUT); //,INPUT_PULLDOWN);
  //unusePin(P2_1);
  unusePin(P2_2);
  unusePin(P2_3);
  unusePin(P2_4);
  //unusePin(P2_5);
  pinMode(P2_5, INPUT_PULLUP);
 pinMode(BUSY_PIN, INPUT_PULLDOWN);
}



void setup()
{
  
    pinMode(ENABLE_BOOST_PIN,OUTPUT) ;// OUTPUT);
    digitalWrite(ENABLE_BOOST_PIN, HIGH);
    if(doLog)
      Serial.begin(9600); 

    if(doLog)
       Serial.println("before enable_boost LOW");
      // digitalWrite(ENABLE_BOOST_PIN, LOW);
   //    return;
 analogIn= analogRead(A0);
 for(int i=0;i<= analogIn;i++)
 {
    picCounter++;
   if(picCounter > 6)
      picCounter=0;
 }
      if(doLog)
      {
          Serial.print("\n\n\n Start Logger \n\r"); 
          Serial.println("Start");
          delay(1000);
          Serial.println("Start");
          delay(1000);
          Serial.println("Start");
          delay(1000);
          Serial.println(HEX, IFG1  );
          delay(1000);
      }
  supplyVolts =Msp430_GetSupplyVoltage();
  if(doLog)
  {
    Serial.print("SupplyVoltage ");
    Serial.println(supplyVolts);
    delay(1000);
  }
 picCounter=0;
 if(doLog)
    Serial.println("Before unusedPins");
  unusedPins();
  if(doLog)
   Serial.println("After unusedPins");
  delay(1000);
    supplyVolts =Msp430_GetSupplyVoltage();
    if(doLog)
    {
      Serial.print("SupplyVoltage 2 ");
      Serial.println(supplyVolts);
    }
  while(r1 == false)
  {
    r1=   TestSDCard();
     delay(1000);
  }
  delay(1000);
  supplyVolts =Msp430_GetSupplyVoltage();
  if(doLog)
  {
    Serial.print("SupplyVoltage 3 ");
    Serial.println(supplyVolts);
    delay(1000);
  }
    

  minCntDown = 0;
  
 
   pinMode(PUSH2, INPUT_PULLUP);

 readyDisplay=true;
  showPic=true;



    BCSCTL1 |= DIVA_3;              // ACLK/8
    BCSCTL3 |= XCAP_3;              //12.5pF cap- setting for 32768Hz crystal

   
    currentMinutes = 0;
    currentSeconds = 0;

    CCTL0 = CCIE;                   // CCR0 interrupt enabled
    CCR0 =  30720*2;           // 512 -> 1 sec, 30720 -> 1 min
    TACTL = TASSEL_1 + ID_3 + MC_1;         // ACLK, /8, upmode


}


/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/
void loop()
{

}

bool TestSDCard()
{
  bool r= true;
 uint16_t supplyVoltsA,supplyVoltsB; 
 
 digitalWrite(ENABLE_BOOST_PIN, HIGH);
 supplyVoltsA =Msp430_GetSupplyVoltage();
  Serial.print("SupplyVoltage A ");
  Serial.println(supplyVoltsA);
pinMode(CSSD_PIN, OUTPUT);
digitalWrite(ENABLE_SD_PIN, LOW);
pinMode(ENABLE_BOOST_PIN, OUTPUT);

 digitalWrite(ENABLE_SD_PIN, LOW);

 
delay(500);
   delay(500);
   delay(500);
    supplyVoltsB =Msp430_GetSupplyVoltage();
Serial.print("SupplyVoltage B ");
  Serial.println(supplyVoltsB);
 Serial.print("SupplyVoltage Diff ");
  Serial.println(supplyVoltsA - supplyVoltsB);
    
  
   delay(500);
   delay(500);
    delay(500);
    BatCheck();
   delay(500);
    delay(500);
   delay(500);
   BatCheck();
     SPI.begin();
  delay(500);
   delay(500);
   BatCheck();
 
  FatFs.begin(CSSD_PIN);
  delay(500);
   delay(500);
   BatCheck();
 
 delay(200); 


    ui32_ReadTemp = ((uint32_t)analogRead(TEMPSENSOR)*27069 - 18169625) *10 >> 16;
 BatCheck();
   
   Serial.println();
   Serial.println("Opening log file to write temperature(LOG.txt).");
   delay(100);
  
   
   rc = FatFs.open("LOG.TXT");
   if (rc){
    r=false;
    die(rc);
   }

   delay(100);
   bw=0;
   ui32_ReadTemp = ((uint32_t)analogRead(TEMPSENSOR)*27069 - 18169625) *10 >> 16;
   sprintf( buf, "%lu Current temperature is %lu.%lu\r\n", counter, ui32_ReadTemp/10, ui32_ReadTemp%10 );
   counter++;
   StringLength =  strlen(buf);
   Serial.println(buf);        


   #if DEBUG
   Serial.print(StringLength, DEC);
   Serial.println();
   Serial.print(AccStringLength, DEC);
   #endif

   rc = FatFs.lseek(  AccStringLength );
   if (rc){r=false; die(rc);}
   AccStringLength =  AccStringLength + 512;
   rc = FatFs.write(buf, StringLength,&bw);
   if (rc) {r=false; die(rc);}
   rc = FatFs.write(0, 0, &bw);  //Finalize write
   if (rc){r=false; die(rc);}
   rc = FatFs.close();  //Close file
        if (rc){r=false; die(rc);}


 BatCheck();
 return r;
}

void BatCheck(void)
{
     supplyVolts =Msp430_GetSupplyVoltage();
  Serial.print("Bat Supply Voltage ");
  Serial.println(supplyVolts);
  if(supplyVolts < 2424 ) {
   
    batLow = true;
     Serial.println("Bat Low");
  }else
  {
      batLow = false;
  }
}
void BatCheck2(void)
{
  pinMode(ENABLE_SD_PIN, OUTPUT);
   digitalWrite(ENABLE_SD_PIN, LOW);
    pinMode (CSSD_PIN, OUTPUT);
  digitalWrite(CSSD_PIN, LOW);

   delay(1000);
     supplyVolts =Msp430_GetSupplyVoltage();
   digitalWrite(CSSD_PIN, HIGH);
   digitalWrite(ENABLE_SD_PIN, HIGH);
  Serial.print("Bat check Supply Voltage ");
  Serial.println(supplyVolts);
  if(supplyVolts < 2424 ) {
   
    batLow = true;
     Serial.println("Bat Low");
  }else
  {
      batLow = false;
  }
}

uint16_t Msp430_GetSupplyVoltage(void)
{
  uint16_t raw_value;
  // first attempt - measure Vcc/2 with 1.5V reference (Vcc < 3V )
  ADC10CTL0 = SREF_1 | REFON | ADC10SHT_2 | ADC10SR | ADC10ON;
  ADC10CTL1 = INCH_11 | SHS_0 | ADC10DIV_0 | ADC10SSEL_0;
  // start conversion and wait for it
  ADC10CTL0 |= ENC | ADC10SC;
  while (ADC10CTL1 & ADC10BUSY) ;
  // stop conversion and turn off ADC
  ADC10CTL0 &= ~ENC;
  ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
  raw_value = ADC10MEM;
  // check for overflow
  if (raw_value == 0x3ff) {
    // switch range - use 2.5V reference (Vcc >= 3V)
    ADC10CTL0 = SREF_1 | REF2_5V | REFON | ADC10SHT_2 | ADC10SR | ADC10ON;
    // start conversion and wait for it
    ADC10CTL0 |= ENC | ADC10SC;
    while (ADC10CTL1 & ADC10BUSY) ;
    raw_value = ADC10MEM;
    // end conversion and turn off ADC
    ADC10CTL0 &= ~ENC;
    ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
    // convert value to mV
    return ((uint32_t)raw_value * 5000) / 1024;
  } else
    return ((uint32_t)raw_value * 3000) / 1024;
}
