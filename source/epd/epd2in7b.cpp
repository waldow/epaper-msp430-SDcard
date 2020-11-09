#include "pfatfs.h"
#include "pffconf.h"

/**
 *  @filename   :   epd2in9b.cpp
 *  @brief      :   Implements for Dual-color e-paper library
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include "epd2in7b.h"

Epd::~Epd() {
};

Epd::Epd() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
};

int Epd::Init(void) {
     if (IfInit() != 0) {
        return -1;
    }
    /* EPD hardware init start */
    Reset();
     
    SendCommand(POWER_ON);
    if(WaitUntilIdle() ==0)
      return -2;
   
    SendCommand(PANEL_SETTING);
    SendData(0xaf);        //KW-BF   KWR-AF    BWROTP 0f
    
    SendCommand(PLL_CONTROL);
    SendData(0x3a);       //3A 100HZ   29 150Hz 39 200HZ    31 171HZ
    
    SendCommand(POWER_SETTING);
    SendData(0x03);                  // VDS_EN, VDG_EN
    SendData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    SendData(0x2b);                  // VDH
    SendData(0x2b);                  // VDL
    SendData(0x09);                  // VDHR
    
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x07);
    SendData(0x07);
    SendData(0x17);
    
    // Power optimization
    SendCommand(0xF8);
    SendData(0x60);
    SendData(0xA5);
    
    // Power optimization
    SendCommand(0xF8);
    SendData(0x89);
    SendData(0xA5);
    
    // Power optimization
    SendCommand(0xF8);
    SendData(0x90);
    SendData(0x00);
    
    // Power optimization
    SendCommand(0xF8);
    SendData(0x93);
    SendData(0x2A);
    
    // Power optimization
    SendCommand(0xF8);
    SendData(0x73);
    SendData(0x41);
    
    SendCommand(VCM_DC_SETTING_REGISTER);
    SendData(0x12);                   
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0x87);        // define by OTP
    
  //  SetLut();
    SetGrayLut();
    SendCommand(PARTIAL_DISPLAY_REFRESH);
    SendData(0x00);  
    /* EPD hardware init end */
    
    return 0;

}

void Epd::Init_4Gray(void)
{
    Reset();
    SendCommand(0x01);      //POWER SETTING
    SendData (0x03);
    SendData (0x00);    
    SendData (0x2b);                               
    SendData (0x2b);    


    SendCommand(0x06);         //booster soft start
    SendData (0x07);    //A
    SendData (0x07);    //B
    SendData (0x17);    //C 

    SendCommand(0xF8);         //boost??
    SendData (0x60);
    SendData (0xA5);

    SendCommand(0xF8);         //boost??
    SendData (0x89);
    SendData (0xA5);

    SendCommand(0xF8);         //boost??
    SendData (0x90);
    SendData (0x00);

    SendCommand(0xF8);         //boost??
    SendData (0x93);
    SendData (0x2A);

    SendCommand(0xF8);         //boost??
    SendData (0xa0);
    SendData (0xa5);

    SendCommand(0xF8);         //boost??
    SendData (0xa1);
    SendData (0x00);

    SendCommand(0xF8);         //boost??
    SendData (0x73);
    SendData (0x41);

    SendCommand(0x16);
    SendData(0x00); 

    SendCommand(0x04);
    WaitUntilIdle();

    SendCommand(0x00);      //panel setting
    SendData(0xbf);   //KW-BF   KWR-AF  BWROTP 0f

    SendCommand(0x30);      //PLL setting
    SendData (0x90);        //100hz 

    SendCommand(0x61);      //resolution setting
    SendData (0x00);    //176
    SendData (0xb0);       
    SendData (0x01);    //264
    SendData (0x08);

    SendCommand(0x82);      //vcom_DC setting
    SendData (0x12);

    SendCommand(0X50);      //VCOM AND DATA INTERVAL SETTING      
    SendData(0x97);
}



/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
int Epd::WaitUntilIdle(void) {
  unsigned int count;    
  count=0; 
    while(DigitalRead(busy_pin) == 0) {      //0: busy, 1: idle
        DelayMs(100);
        count++;
        if(count > 200)
        {
         // Reset();
          // DelayMs(200);
           
           return 0;
        }
         
    }      
     return 1;
}

/**
 *  @brief: module reset. 
 *          often used to awaken the module in deep sleep, 
 *          see Epd::Sleep();
 */
void Epd::Reset(void) {
    DigitalWrite(reset_pin, LOW);
    DelayMs(200);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(200);   
}
/**
 *  @brief: set the look-up tables
 */
void Epd::SetLut(void) {
    unsigned int count;     
    SendCommand(LUT_FOR_VCOM);                            //vcom
    for(count = 0; count < 44; count++) {
        SendData(lut_vcom_dc[count]);
    }
    
    SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
    for(count = 0; count < 42; count++) {
        SendData(lut_ww[count]);
    }   
    
    SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
    for(count = 0; count < 42; count++) {
        SendData(lut_bw[count]);
    } 

    SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
    for(count = 0; count < 42; count++) {
        SendData(lut_bb[count]);
    } 

    SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
    for(count = 0; count < 42; count++) {
        SendData(lut_wb[count]);
    } 
}


void Epd::SetGrayLut(void) {
    unsigned int count;     
    SendCommand(LUT_FOR_VCOM);                            //vcom
    for(count = 0; count < 44; count++) {
        SendData(EPD_2in7_gray_lut_vcom[count]);
    }
    
    SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
    for(count = 0; count < 42; count++) {
        SendData(EPD_2in7_gray_lut_ww[count]);
    }   
    
    SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
    for(count = 0; count < 42; count++) {
        SendData(EPD_2in7_gray_lut_bw[count]);
    } 

    SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
    for(count = 0; count < 42; count++) {
        SendData(EPD_2in7_gray_lut_bb[count]);
    } 

    SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
    for(count = 0; count < 42; count++) {
        SendData(EPD_2in7_gray_lut_wb[count]);
    } 
}





/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char* frame_buffer_black, const unsigned char* frame_buffer_red) {
    if (frame_buffer_black != NULL) {
        SendCommand(DATA_START_TRANSMISSION_1);
        DelayMs(2);
        for (unsigned int i = 0; i < this->width * this->height / 8; i++) {
         //   SendData(pgm_read_byte(&frame_buffer_black[i]));
        }
        DelayMs(2);
    }
    if (frame_buffer_red != NULL) {
        SendCommand(DATA_START_TRANSMISSION_2);
        DelayMs(2);
        for (unsigned int i = 0; i < this->width * this->height / 8; i++) {
       //     SendData(pgm_read_byte(&frame_buffer_red[i]));
        }
        DelayMs(2);
    }
  //  SendCommand(DISPLAY_REFRESH);
  //  WaitUntilIdle();
}
void Epd::SetFrameFatFsBlack()
{
    unsigned char buffer1[16];
    unsigned short int br;
      SendCommand(DATA_START_TRANSMISSION_1);
        DelayMs(2);
        for (unsigned int i = 0; i < this->width * this->height / 128; i++) {
          FatFs.read(buffer1, sizeof(buffer1), &br);

          for (int q = 0; q < br; q++) {
           SendData(buffer1[q]);
          }
        }
        DelayMs(2);

  
}
void Epd::SetFrameFatFsRed()
{
    unsigned char buffer1[16];
    unsigned short int br;
      SendCommand(DATA_START_TRANSMISSION_2);
        DelayMs(2);
        for (unsigned int i = 0; i < this->width * this->height / 128; i++) {
          FatFs.read(buffer1, sizeof(buffer1), &br);

          for (int q = 0; q < br; q++) {
           SendData(buffer1[q]);
          }
        }
        DelayMs(2);

  
}
/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void Epd::ClearFrame(void) {
    SendCommand(DATA_START_TRANSMISSION_1);           
    DelayMs(2);
    for(unsigned int i = 0; i < width * height / 8; i++) {
        SendData(0x0);  
    }  
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2);           
    DelayMs(2);
    for(unsigned int i = 0; i < width * height / 8; i++) {
        SendData(0x0);  
    }  
    DelayMs(2);
}

/**
 * @brief: This displays the frame data from SRAM
 */
void Epd::DisplayFrame(void) {
    SendCommand(DISPLAY_REFRESH); 
    WaitUntilIdle();
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void Epd::Sleep() {
  //  SendCommand(POWER_OFF);
   // WaitUntilIdle();
  //   DelayMsMs(200);
    SendCommand(DEEP_SLEEP);
    SendData(0xA5);     // check code
}

void Epd::PowerOff() {
   SendCommand(POWER_OFF);
    WaitUntilIdle();
    DelayMs(200);
 //   SendCommand(DEEP_SLEEP);
 //   SendData(0xA5);     // check code
}

const unsigned char lut_vcom_dc[] =
{
0x00, 0x00,
0x00, 0x1A, 0x1A, 0x00, 0x00, 0x01,        
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x08,        
0x00, 0x0E, 0x01, 0x0E, 0x01, 0x10,        
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x08,        
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,        
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,        
0x00, 0x23, 0x00, 0x00, 0x00, 0x01    
};

//R21H
const unsigned char lut_ww[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x40, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x80, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0x00, 0x23, 0x00, 0x00, 0x00, 0x01
};

//R22H    r
const unsigned char lut_bw[] =
{
0xA0, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x90, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0xB0, 0x04, 0x10, 0x00, 0x00, 0x05,
0xB0, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0xC0, 0x23, 0x00, 0x00, 0x00, 0x01
};

//R23H    w
const unsigned char lut_bb[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x40, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x80, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0x00, 0x23, 0x00, 0x00, 0x00, 0x01
};

//R24H    b
const unsigned char lut_wb[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x20, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x10, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0x00, 0x23, 0x00, 0x00, 0x00, 0x01
};

//0~3 gray
const unsigned char EPD_2in7_gray_lut_vcom[] =
{
0x00  ,0x00,
0x00  ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
0x60  ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
0x00  ,0x14 ,0x00 ,0x00 ,0x00 ,0x01,
0x00  ,0x13 ,0x0A ,0x01 ,0x00 ,0x01,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,        
};
//R21
const unsigned char EPD_2in7_gray_lut_ww[] ={
0x40  ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
0x90  ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
0x10  ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
0xA0  ,0x13 ,0x01 ,0x00 ,0x00 ,0x01,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R22H  r
const unsigned char EPD_2in7_gray_lut_bw[] ={
0x40  ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
0x90  ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
0x00  ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
0x99  ,0x0C ,0x01 ,0x03 ,0x04 ,0x01,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R23H  w
const unsigned char EPD_2in7_gray_lut_wb[] ={
0x40  ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
0x90  ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
0x00  ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
0x99  ,0x0B ,0x04 ,0x04 ,0x01 ,0x01,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R24H  b
const unsigned char EPD_2in7_gray_lut_bb[] ={
0x80  ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
0x90  ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
0x20  ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
0x50  ,0x13 ,0x01 ,0x00 ,0x00 ,0x01,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};



/* END OF FILE */
