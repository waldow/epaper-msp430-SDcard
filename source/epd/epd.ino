#include <SPI.h>
#include <pfatfs.h>
#include <pffconf.h>
#include "epd2in7b.h"
#include "SPI.h" 

#define CSSD_PIN      8             // chip select for sd card pin  . 0=on 1=off
#define ENABLE_BOOST_PIN     9      // enable boost to 3.3v  pin . 0=off 1=on
#define ENABLE_SD_PIN      6            // switch on mosfet to power sd. 0=on 1=off
#define read_buffer 128             // size (in bytes) of read buffer 
#define BAT_LEVEL   5
#define TMO_LEVEL   5
#define PHASE_READ_SETTING  0
#define PHASE_READ_VALUE    1
#define PHASE_END_OF_LINE   2
#define PHASE_END_OF_FILE   3
#define PHASE_COMMENT_LINE   4
volatile uint8_t setupDone = 0;
volatile uint8_t lowPowerPic = 0;
volatile uint8_t readyDisplay = 0;
volatile uint8_t showPic = 0;
volatile uint8_t batCntDown = BAT_LEVEL;
volatile unsigned int minCounter = 0;
volatile unsigned int minCntSetPoint = 10;
volatile uint8_t timeoutCntDown = 0;

char bufferData[read_buffer];
int rc;
uint32_t ui32_ReadTemp = 0;
uint8_t StringLength = 0;
volatile uint32_t AccStringLength = 0;
unsigned int currentMinutes, currentSeconds;
unsigned int wdtCounter = 0;
volatile unsigned int picCounter = 0;
volatile unsigned int picMin = 1;
volatile unsigned int picMax = 6;
volatile unsigned int picStart = 1;

char charno[5];
char filename1[12];

Epd epd;
#define F_CPU  8000000

int analogIn;
uint16_t supplyVolts;
bool batLow = false;
bool r1 = false;

volatile bool doLog = true;


void unusePin(int pin)
{
	pinMode(pin, INPUT_PULLUP); 

}
//  To minimize the current draw
void unusedPins()
{
	unusePin(P1_0);
	if (!doLog)
	{
		unusePin(P1_1);
		unusePin(P1_2);
	}
	unusePin(P1_3);
	
	pinMode(P1_4, INPUT_PULLUP);
	
	pinMode(P2_0, INPUT); 
						
	unusePin(P2_2);
	unusePin(P2_3);
	unusePin(P2_4);
	
	pinMode(P2_5, INPUT_PULLUP);
	pinMode(BUSY_PIN, INPUT_PULLDOWN);
}



void setup()
{
  if (doLog)
    Serial.begin(9600);

    pinMode(ENABLE_BOOST_PIN, OUTPUT);// OUTPUT);
  digitalWrite(ENABLE_BOOST_PIN, LOW);
  for (int i = 0; i <= 3; i++)
  {
    BatCheck();
    delay(1000);
  }
	pinMode(ENABLE_BOOST_PIN, OUTPUT);// OUTPUT);
	digitalWrite(ENABLE_BOOST_PIN, HIGH);
	

	if (doLog)
		Serial.println("before enable_boost LOW");
	
	analogIn = analogRead(A0);
	for (int i = 0; i <= analogIn; i++)
	{
		picCounter++;
		if (picCounter > picMax)
			picCounter = 0;
	}
	if (doLog)
	{
		Serial.print("\n\n\n Start Logger \n\r");
		Serial.println("Start");
		delay(1000);
		Serial.println("Start");
		delay(1000);
		Serial.println("Start");
		delay(1000);
		Serial.println(HEX, IFG1);
		delay(1000);
	}
	supplyVolts = Msp430_GetSupplyVoltage();
	if (doLog)
	{
		Serial.print("SupplyVoltage ");
		Serial.println(supplyVolts);
		delay(1000);
	}
	picCounter = 0;
	if (doLog)
		Serial.println("Before unusedPins");
	unusedPins();
	if (doLog)
		Serial.println("After unusedPins");
	delay(1000);
	supplyVolts = Msp430_GetSupplyVoltage();
	if (doLog)
	{
		Serial.print("SupplyVoltage 2 ");
		Serial.println(supplyVolts);
	}
 
 
 /*
	while (r1 == false)
	{
		r1 = TestSDCard();
		delay(1000);
	}
 */
	delay(1000);
	supplyVolts = Msp430_GetSupplyVoltage();
	if (doLog)
	{
		Serial.print("SupplyVoltage 3 ");
		Serial.println(supplyVolts);
		delay(1000);
	}


	minCounter = 0;


	pinMode(PUSH2, INPUT_PULLUP);

	
	showPic = true;

	BCSCTL1 |= DIVA_3;              // ACLK/8
	BCSCTL3 |= XCAP_3;              //12.5pF cap- setting for 32768Hz crystal

	currentMinutes = 0;
	currentSeconds = 0;

	CCTL0 = CCIE;                   // CCR0 interrupt enabled
	CCR0 = 30720 * 2;           // 512 -> 1 sec, 30720 -> 1 min  * 2
	TACTL = TASSEL_1 + ID_3 + MC_1;         // ACLK, /8, upmode


}


/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/
void loop()
{
	
		if (doLog)
			Serial.print("-");
      
      if(setupDone == false)
      {
          if(ReadSetupFromDisk())
          {
            setupDone =true;
            readyDisplay = true;
            lowPowerPic =0;
          }else
          {
            readyDisplay = false;
          }
          if (batLow == true &&   lowPowerPic ==0)
          {
              if (doLog)
                  Serial.println("LowBatPic");
              LowBatPic();
              lowPowerPic =1;
          }
      }

		batCntDown--;
		if (batCntDown == 0)
		{
			batCntDown = BAT_LEVEL;
			BatCheck();
		}

		if (showPic == true && readyDisplay == true)
		{
			BatCheck();
			if (batLow == true)
			{
				LowBatPic();
			}
			else
			{
				NextPic();
			}


			showPic = false;



		}
		else
		{

		}

		unusedPins();
		if (doLog)
		{
			Serial.println("L Enter LPM3 w/ interrupt");
		}
		delay(200);
		__bis_SR_register(LPM3_bits + GIE);           // Enter LPM3 w/ interrupt
		if (doLog)
		{
			Serial.println("L After LPM3 w/ interrupt");
		}

}

void NextPic()
{
	if (doLog)
	{
		Serial.println("N Start");
	}

	if (!readyDisplay)
	{
		if (doLog)
		{
			Serial.println("N Not ready");
		}
		delay(500);

		return;
	}


	delay(500);
	pinMode(ENABLE_SD_PIN, OUTPUT);
	digitalWrite(ENABLE_SD_PIN, LOW);
	delay(500);
	delay(500);
	readyDisplay = false;
	pinMode(CSSD_PIN, OUTPUT);
	digitalWrite(CSSD_PIN, HIGH);

	pinMode(CS_PIN, OUTPUT);
	digitalWrite(CS_PIN, HIGH);
	if (doLog)
	{
		Serial.println("enable_boost Before");
	}
	digitalWrite(ENABLE_BOOST_PIN, HIGH);
	if (doLog)
	{
		Serial.println("enable_boost After");
	}
	delay(500);
	delay(500);
	pinMode(RST_PIN, OUTPUT);
	pinMode(DC_PIN, OUTPUT);
	pinMode(BUSY_PIN, INPUT_PULLDOWN);
	BatCheck();
	digitalWrite(RST_PIN, LOW);
	delay(200);
	digitalWrite(RST_PIN, HIGH);
	delay(200);
	SPI.begin();
	Epd epd;
	FatFs.begin(CSSD_PIN);
	delay(200);

	sprintf(filename1, "pic%0.3db.bin", picCounter);
	if (doLog)
	{
		Serial.println(filename1);
	}
	rc = FatFs.open(filename1);
	if (rc) {

		die(rc);
		readyDisplay = true;
		return;
	}
	epd.SetFrameFatFsBlack();
	rc = FatFs.close();
	sprintf(filename1, "pic%0.3dr.bin", picCounter);
	if (doLog)
	{
		Serial.println(filename1);
	}
	rc = FatFs.open(filename1);
	epd.SetFrameFatFsRed();
	rc = FatFs.close();
	if (doLog)
	{
		Serial.println("FatFs.close");
	}
	delay(200);
	delay(200);
	digitalWrite(ENABLE_SD_PIN, HIGH);
	delay(200);
	digitalWrite(ENABLE_BOOST_PIN, LOW);
	delay(200);
	rc = epd.Init();
	if (rc != 0) {
		if (doLog)
		{
			Serial.print("e-Paper init failed");
		}
		die(rc);
	}
	if (doLog)
	{
		Serial.println("DisplayFrame Before");
	}
	epd.DisplayFrame();
	if (doLog)
	{
		Serial.println("DisplayFrame After");
	}
	/* Deep sleep */

	epd.Sleep();
	SPI.end();


	pinMode(CSSD_PIN, INPUT);
	pinMode(CS_PIN, INPUT);
	pinMode(RST_PIN, INPUT_PULLUP);
	pinMode(DC_PIN, INPUT_PULLUP);
	pinMode(BUSY_PIN, INPUT_PULLDOWN);

	delay(500);
	delay(500);
	
	readyDisplay = true;
  picCounter++;
  if (picCounter >= picMax)
    picCounter = picMin;
	if (doLog)
	{
		Serial.println("N End");
	}
}

void LowBatPic()
{
if (doLog)
  {
    Serial.println("LowBatPic Start");
  }
  delay(500);
 digitalWrite(ENABLE_SD_PIN, HIGH);
  delay(200);
  digitalWrite(ENABLE_BOOST_PIN, LOW);
  delay(500);
  pinMode(CSSD_PIN, OUTPUT);
  digitalWrite(CSSD_PIN, HIGH);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  
  pinMode(RST_PIN, OUTPUT);
  pinMode(DC_PIN, OUTPUT);
  pinMode(BUSY_PIN, INPUT_PULLDOWN);
  digitalWrite(RST_PIN, LOW);
  delay(200);
  digitalWrite(RST_PIN, HIGH);
  delay(200);
  SPI.begin();
  Epd epd;
  
  
  rc = epd.Init();
  if (rc != 0) {
    if (doLog)
    {
      Serial.print("e-Paper init failed");
    }
    die(rc);
  }
  if (doLog)
  {
    Serial.println("DisplayFrame Before");
  }
  epd.ClearFrame();
  epd.DisplayFrame();
  if (doLog)
  {
    Serial.println("DisplayFrame After");
  }
  /* Deep sleep */

  epd.Sleep();
  SPI.end();


  pinMode(CSSD_PIN, INPUT);
  pinMode(CS_PIN, INPUT);
  pinMode(RST_PIN, INPUT_PULLUP);
  pinMode(DC_PIN, INPUT_PULLUP);
  pinMode(BUSY_PIN, INPUT_PULLDOWN);

  delay(500);
  if (doLog)
  {
    Serial.println("LowBatPic End");
  }
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

  if (doLog)
   Serial.println("T");
 
  if(readyDisplay ==false)
  {

    timeoutCntDown++;
    if(timeoutCntDown >= TMO_LEVEL)
    {
      if (doLog)
        Serial.println("D");
      WDTCTL = 0xDEAD;
    }
  }else
  {
    timeoutCntDown = 0;
  }

 minCounter++;

 if(minCounter >= minCntSetPoint)
 {
    minCounter=0;

   if(readyDisplay == true)
   {
     
      showPic =true;
     
   }
 }

  __bic_SR_register_on_exit(LPM3_bits); 
 
}



bool TestSDCard()
{
	bool r = true;
	unsigned short int bw, br;
	char buf[50];
	uint32_t counter = 0;
	uint16_t supplyVoltsA, supplyVoltsB;

	digitalWrite(ENABLE_BOOST_PIN, HIGH);
	supplyVoltsA = Msp430_GetSupplyVoltage();
	if (doLog)
	{
		Serial.print("SupplyVoltage A= ");
		Serial.println(supplyVoltsA);
	}
	pinMode(CSSD_PIN, OUTPUT);
	digitalWrite(ENABLE_SD_PIN, LOW);
	pinMode(ENABLE_BOOST_PIN, OUTPUT);

	digitalWrite(ENABLE_SD_PIN, LOW);


	delay(500);
	delay(500);
	delay(500);
	supplyVoltsB = Msp430_GetSupplyVoltage();
	if (doLog)
	{
		Serial.print("SupplyVoltage B ");
		Serial.println(supplyVoltsB);
		Serial.print("SupplyVoltage Diff ");
		Serial.println(supplyVoltsA - supplyVoltsB);
	}

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


	ui32_ReadTemp = ((uint32_t)analogRead(TEMPSENSOR) * 27069 - 18169625) * 10 >> 16;
	BatCheck();
	if (doLog)
	{
		Serial.println();
		Serial.println("Opening log file to write temperature(LOG.txt).");
	}
	delay(100);


	rc = FatFs.open("LOG.TXT");
	if (rc) {
		r = false;
		die(rc);
	}

	delay(100);
	bw = 0;
	ui32_ReadTemp = ((uint32_t)analogRead(TEMPSENSOR) * 27069 - 18169625) * 10 >> 16;
	sprintf(buf, "%lu Current temperature is %lu.%lu\r\n", counter, ui32_ReadTemp / 10, ui32_ReadTemp % 10);
	counter++;
	StringLength = strlen(buf);
	Serial.println(buf);


#if DEBUG
	Serial.print(StringLength, DEC);
	Serial.println();
	Serial.print(AccStringLength, DEC);
#endif

	rc = FatFs.lseek(AccStringLength);
	if (rc) { r = false; die(rc); }
	AccStringLength = AccStringLength + 512;
	rc = FatFs.write(buf, StringLength, &bw);
	if (rc) { r = false; die(rc); }
	rc = FatFs.write(0, 0, &bw);  //Finalize write
	if (rc) { r = false; die(rc); }
	rc = FatFs.close();  //Close file
	if (rc) { r = false; die(rc); }


	BatCheck();
	return r;
}
void die(int pff_err)
{
	if (doLog)
	{
		Serial.println();
		Serial.print("Failed with rc=");
		Serial.print(pff_err, DEC);
	}
  if(pff_err == 6)
  {
    digitalWrite(ENABLE_BOOST_PIN, LOW);
    delay(500);
    BatCheck();
  }

 
}
void BatCheck(void)
{
	
	supplyVolts = Msp430_GetSupplyVoltage();
	if (doLog)
	{
		Serial.print("Bat Supply Voltage ");
		Serial.println(supplyVolts);
	}
	if (supplyVolts < 2600) {

		batLow = true;
		if (doLog)
			Serial.println("Bat Low");
	}
	else
	{
		batLow = false;
	}


}
void BatCheck2(void)
{
	pinMode(ENABLE_SD_PIN, OUTPUT);
	digitalWrite(ENABLE_SD_PIN, LOW);
	pinMode(CSSD_PIN, OUTPUT);
	digitalWrite(CSSD_PIN, LOW);

	delay(1000);
	supplyVolts = Msp430_GetSupplyVoltage();
	digitalWrite(CSSD_PIN, HIGH);
	digitalWrite(ENABLE_SD_PIN, HIGH);
	if (doLog)
	{
		Serial.print("Bat check Supply Voltage ");
		Serial.println(supplyVolts);
	}
	if (supplyVolts < 2424) {

		batLow = true;
		if (doLog)
		{
			Serial.println("Bat Low");
		}
	}
	else
	{
		batLow = false;
	}
}

bool ReadSetupFromDisk()
{
    bool result=false;
    char settingname[10];
    char settingvalue[10];
    char ch[2];
    short phase=PHASE_READ_SETTING;
    int offset=0;
    unsigned short int br;
    uint16_t supplyVoltsA, supplyVoltsB;

  digitalWrite(ENABLE_BOOST_PIN, HIGH);
  supplyVoltsA = Msp430_GetSupplyVoltage();
  if (doLog)
  {
    Serial.print("SupplyVoltage A= ");
    Serial.println(supplyVoltsA);
  }
  pinMode(CSSD_PIN, OUTPUT);
  digitalWrite(ENABLE_SD_PIN, LOW);
  pinMode(ENABLE_BOOST_PIN, OUTPUT);

  digitalWrite(ENABLE_SD_PIN, LOW);


  delay(500);
  delay(500);
  delay(500);
  supplyVoltsB = Msp430_GetSupplyVoltage();
  if (doLog)
  {
    Serial.print("SupplyVoltage B ");
    Serial.println(supplyVoltsB);
    Serial.print("SupplyVoltage Diff ");
    Serial.println(supplyVoltsA - supplyVoltsB);
  }

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

 
      supplyVoltsA = Msp430_GetSupplyVoltage();
      rc= FatFs.begin(CSSD_PIN);
      if (rc) 
      {
           if(rc ==1)
           {
              batLow = true;
           }
              if (doLog)
              {
                Serial.print("FatFs.begin failed ");
                Serial.println(rc);
              }
              supplyVoltsB = Msp430_GetSupplyVoltage();
              if (doLog)
              {
                if(supplyVoltsA > supplyVoltsB)
                {
                  Serial.print("SupplyVoltage Diff ");
                  Serial.println(supplyVoltsA - supplyVoltsB);
                }
                if(supplyVoltsB > supplyVoltsA)
                {
                  Serial.print("SupplyVoltage Diff ");
                  Serial.println(supplyVoltsB - supplyVoltsA);
                }
              }
      }
 

  delay(200);





 if(rc == 0)  
 {

        supplyVoltsA = Msp430_GetSupplyVoltage();
       
          rc = FatFs.open("SETUP.TXT");
          if(rc == 6)
          {
           //  digitalWrite(ENABLE_SD_PIN, HIGH);
         //    delay(200);
              supplyVoltsB = Msp430_GetSupplyVoltage();
              if (doLog)
              {
                if(supplyVoltsA > supplyVoltsB)
                {
                  Serial.print("SupplyVoltage Diff ");
                  Serial.println(supplyVoltsA - supplyVoltsB);
                }
                if(supplyVoltsB > supplyVoltsA)
                {
                  Serial.print("SupplyVoltage Diff ");
                  Serial.println(supplyVoltsB - supplyVoltsA);
                }
              }
         //      digitalWrite(ENABLE_SD_PIN, LOW);
              delay(200);
          }
    
      
      if (rc) {
          if (doLog)
          {
            Serial.println("Open SETUP.TXT failed");
          }
          die(rc);
          return false;
      }

       
    while(phase != PHASE_END_OF_FILE ) {
             FatFs.read(ch, 1, &br);  
             ch[1] = 0;
              if (doLog)
            {
            Serial.print("Read :");
           Serial.println(br, DEC);
            }
             if(br == 0)
                phase =  PHASE_END_OF_FILE;
              else
              if(ch[0] == '=' && phase == PHASE_READ_SETTING)
                {
                  phase = PHASE_READ_VALUE;
                  offset=0;
                } //PHASE_COMMENT_LINE
              else  if(ch[0] == '#')
              {
                phase = PHASE_COMMENT_LINE;
              }
              else if(ch[0] == 10 || ch[0] == 13)
              {
                
                  if(offset > 0)
                  {
                    SetSetting(settingname,settingvalue);
                  }
                  phase = PHASE_READ_SETTING;
                  offset=0;
             }else                
             if(phase == PHASE_READ_SETTING)
             {
                settingname[offset] = ch[0];
                settingname[offset+1] =0;
                offset++;
                /*
                if (doLog)
                {
                  Serial.print("Setting Name =");
                  Serial.println(settingname);
                }
*/
                
             }else
             if(phase == PHASE_READ_VALUE)
             {
                      settingvalue[offset] = ch[0];
                      settingvalue[offset+1] = 0;
                      offset++;       
                      /*
                      if (doLog)
                      {
                          Serial.print("Setting Value =");
                          Serial.println(settingvalue);
                      }
                      */
             }
           
    }

    rc = FatFs.close();  //Close file
  if (rc) {  die(rc); }

 }
 if(rc)
      return false;
    else
      return true;
   
}

void SetSetting(char *settingname,char *settingvalue)
{
      if (doLog)
            {
              Serial.println("===========");
              Serial.println("Settings");
              Serial.print("Name =");
              Serial.println(settingname);
              Serial.print("Value =");
              Serial.println(settingvalue);
              Serial.println("===========");
            }
      if(strcmp(settingname,"START") ==0)
      {
        picCounter = atoi(settingvalue);
        picStart = picCounter;
       }else if(strcmp(settingname,"MIN") ==0)
      {
          picMin = atoi(settingvalue);
      }else if(strcmp(settingname,"MAX") ==0)
      {
          picMax = atoi(settingvalue);
      }else if(strcmp(settingname,"DELAY") ==0)
      {
          minCntSetPoint = atoi(settingvalue);
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
	while (ADC10CTL1 & ADC10BUSY);
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
		while (ADC10CTL1 & ADC10BUSY);
		raw_value = ADC10MEM;
		// end conversion and turn off ADC
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
		// convert value to mV
		return ((uint32_t)raw_value * 5000) / 1024;
	}
	else
		return ((uint32_t)raw_value * 3000) / 1024;
}
