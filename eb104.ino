/*******************************************************************************
Title: eb104 Controller
Author: Corrado Gerbaldo - IU1BOW

--------------------------------------------------------------------------------
TODO: .....
--------------------------------------------------------------------------------

Links:
- Quadratic fit:  https://www.wolframalpha.com/input/
- u8g2 fonts:     https://github.com/olikraus/u8g2/wiki/fnt
- color picker:   http://www.barth-dev.de/online/rgb565-color-picker/

*******************************************************************************/

/*------------------------------------------------------------------------------
Includes
------------------------------------------------------------------------------*/
#include "eb104_display.h"
#include "eb104_labels.h"
#include "eb104_const.h"
#include <SPI.h>          // f.k. for Arduino-1.5.2
#include <Adafruit_GFX.h>// Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <U8g2_for_Adafruit_GFX.h>
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

#include <TouchScreen.h>
TouchScreen ts(XP, YP, XM, YM, 300);   //re-initialised after diagnose
TSPoint tp;                            //global point

float fwd;
float ref;


/******************************************************************************
SETUP
*******************************************************************************/
void setup() {

  Serial.begin(9600);

  uint32_t when = millis();
  if (!Serial) delay(5000);           //allow some time for Leonardo
  Serial.println("Serial took " + String((millis() - when)) + "ms to start");
  uint16_t ID = tft.readID(); //
  Serial.print("ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  tft.begin(ID);
  tft.setRotation(ORIENTATION);
  u8g2_for_adafruit_gfx.begin(tft);                 // connect u8g2 procedures to Adafruit GFX
  
  drawMain();
  Serial.println("End setup");
}

/*------------------------------------------------------------------------------
  Draw main screen
------------------------------------------------------------------------------*/
void drawMain(){
  tft.fillRect(0, 0, tft.width(), tft.height(), LCD_MAIN_BG);

  drawMeasure(0,MAX_FWD,LBL_FWD,LCD_FWD_X,LCD_FWD_Y,LCD_FWD_W,LCD_FWD_H,1,LBL_FWD_MIN,LBL_FWD_MAX);
  drawMeasure(0,MAX_REF,LBL_REF,LCD_REF_X,LCD_REF_Y,LCD_REF_W,LCD_REF_H,1,LBL_REF_MIN,LBL_REF_MAX);
  drawMeasure(0,MAX_SWR,LBL_SWR,LCD_SWR_X,LCD_SWR_Y,LCD_SWR_W,LCD_SWR_H,1,LBL_SWR_MIN,LBL_SWR_MAX);
  drawMeasure(0,MAX_TMP,LBL_TMP,LCD_TMP_X,LCD_TMP_Y,LCD_TMP_W,LCD_TMP_H,1,LBL_TMP_MIN,LBL_TMP_MAX);
  drawMeasure(0,MAX_V,LBL_V,LCD_V_X,LCD_V_Y,LCD_V_W,LCD_V_H,1,LBL_V_MIN,LBL_V_MAX);
  drawMeasure(0,MAX_I,LBL_I,LCD_I_X,LCD_I_Y,LCD_I_W,LCD_I_H,1,LBL_I_MIN,LBL_I_MAX);
  drawMessage("Started");
  drawButtons();

}

/*------------------------------------------------------------------------------
  Draw a measure object
------------------------------------------------------------------------------*/
void drawMeasure(long value, long max, char label[],int x, int y, int len, int height,byte reset,char lbl_min[], char lbl_max[]) {

  u8g2_for_adafruit_gfx.setBackgroundColor(LCD_MSR_BG);
  int x_curs;
  int h_font;
  int lbl_ln;


  if (reset==1) {
    //draw measure box
    tft.fillRect(x,y,len,height,LCD_MSR_BG);

    //print min and max labels
    u8g2_for_adafruit_gfx.setFont(FNT_MAX_MIN);
    h_font=u8g2_for_adafruit_gfx.getFontAscent()+u8g2_for_adafruit_gfx.getFontDescent()*-1;
    u8g2_for_adafruit_gfx.setForegroundColor(LCD_MAX_MIN_FG);
    u8g2_for_adafruit_gfx.setCursor(x+LCD_SPACING,y+LCD_SPACING*2+h_font);
    u8g2_for_adafruit_gfx.print(lbl_min);
    lbl_ln=u8g2_for_adafruit_gfx.getUTF8Width(lbl_max);
    u8g2_for_adafruit_gfx.setCursor(x+len-lbl_ln,y+LCD_SPACING*2+h_font);
    u8g2_for_adafruit_gfx.print(lbl_max);

    //print main label
    u8g2_for_adafruit_gfx.setFont(FNT_MEASURE);
    h_font=u8g2_for_adafruit_gfx.getFontAscent()+u8g2_for_adafruit_gfx.getFontDescent()*-1;
    u8g2_for_adafruit_gfx.setForegroundColor(LCD_MSR_FG);
    x_curs=x+(len-u8g2_for_adafruit_gfx.getUTF8Width(label))/2;
    u8g2_for_adafruit_gfx.setCursor(x_curs,y+LCD_SPACING*2+h_font);
    u8g2_for_adafruit_gfx.print(label);

    //draw values line
    tft.drawLine(x+LCD_SPACING,
                2+y+LCD_SPACING*4+h_font+height/3,
                x+len,
                2+y+LCD_SPACING*4+h_font+height/3,LCD_MSR_FG);
  
  }

  //print value
  u8g2_for_adafruit_gfx.setFont(FNT_MEASURE);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fnt
  u8g2_for_adafruit_gfx.setForegroundColor(LCD_MSR_FG);
  lbl_ln=u8g2_for_adafruit_gfx.getUTF8Width(label);
  x_curs=x+lbl_ln+(len-lbl_ln)/2;
  h_font=u8g2_for_adafruit_gfx.getFontAscent()+u8g2_for_adafruit_gfx.getFontDescent()*-1;
  tft.fillRect(x_curs,y+LCD_SPACING*2,u8g2_for_adafruit_gfx.getUTF8Width("    "),h_font,LCD_MSR_BG);
  u8g2_for_adafruit_gfx.setCursor(x_curs,y+LCD_SPACING*2+h_font);
  u8g2_for_adafruit_gfx.print(value);

  //draw bar
  drawMeasureBar(value,max,x+LCD_SPACING,y+LCD_SPACING*4+h_font,len-LCD_SPACING*2,height/3,LCD_SPACE_BAR,LCD_BORDER_BAR);

}

/*..............................................................................
  Draw a measure bar, inside a measure object
 .............................................................................*/
void drawMeasureBar(long value, long max, int x, int y, int len, int height, int space, int border) {

  int filled =  len*value/max;

  for(int i=0;i<len-space;i=i+space) {
    if (i>=filled) {
      tft.fillRect(x+i,y,space-border,height,LCD_BAR_BG);
    } else
      tft.fillRect(x+i,y,space-border,height,LCD_BAR_FG);
  }
}

/*------------------------------------------------------------------------------
  Draw buttons
 -----------------------------------------------------------------------------*/
void drawButtons() {

  byte idxBtn=0;

  u8g2_for_adafruit_gfx.setForegroundColor(LCD_BTN_FG);
  u8g2_for_adafruit_gfx.setBackgroundColor(LCD_BTN_BG);
  u8g2_for_adafruit_gfx.setFont(FNT_BUTTONS);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fnt

  for(int i=LCD_SPACING;i<tft.width();i=i+tft.width()/BUTTONS) {
    int w=tft.width()/BUTTONS-LCD_SPACING*2;
    tft.fillRect(LCD_SPACING+i,tft.height()-LCD_BTN_H-LCD_SPACING*2,w,tft.height()-LCD_BTN_H,LCD_BTN_BG);
    int x=(w-u8g2_for_adafruit_gfx.getUTF8Width(button[idxBtn]))/2;
    int y=tft.height()-(LCD_BTN_H-u8g2_for_adafruit_gfx.getFontAscent()+u8g2_for_adafruit_gfx.getFontDescent()*-1)/2;
    u8g2_for_adafruit_gfx.setCursor(LCD_SPACING*2+i+x, y);
    u8g2_for_adafruit_gfx.print(button[idxBtn]);
    idxBtn++;
  }
}

/*------------------------------------------------------------------------------
  Draw messages
 -----------------------------------------------------------------------------*/
void drawMessage(char msg[]){

  tft.fillRect(LCD_SPACING, tft.height()-LCD_BTN_H-LCD_MSG_H-LCD_SPACING, tft.width()-LCD_SPACING*2,LCD_MSG_H,LCD_MSG_BG);
  u8g2_for_adafruit_gfx.setFont(FNT_MESSAGE);
  u8g2_for_adafruit_gfx.setForegroundColor(LCD_MSG_FG);
  u8g2_for_adafruit_gfx.setBackgroundColor(LCD_MSG_BG);
  u8g2_for_adafruit_gfx.setCursor(LCD_MSG_X,tft.height()-LCD_BTN_H-(u8g2_for_adafruit_gfx.getFontAscent()+u8g2_for_adafruit_gfx.getFontDescent()*-1)/2);
  u8g2_for_adafruit_gfx.print(msg);
}

/******************************************************************************
MAIN LOOP
*******************************************************************************/
void loop() {
  
static int nr;

    getTouch();
/*  FWD measures
 *
 *   130,5
 *  180,10
 *  330,30
 *  390,50
 *  460,70
 *  580,100
 *  quadratic fit {130,5},{180,10},{330,30},{390,50},{460,70},{580,100}
 *  https://www.wolframalpha.com/input/
 */
      nr++;
      fwd=fwd+analogRead(sensorPinFWD);
      // read reverse voltage
      ref=ref+analogRead(sensorPinREF);
      if (nr>100) {
        fwd=fwd/nr;
        ref=ref/nr;
        nr=0;
        

/*
          // compute SWR and return
          float wf;
          if (ref == 0 || fwd < m_MinPower) {
            wf = 1.0;
          } else if (ref >= fwd) {
            wf = maxSwr;
          } else {
            #ifdef USE_VOLTAGE_CALC
            wf = (float)(fwd + ref) / (float)(fwd - ref);
            #else
            wf = (float)fwd / (float)ref;
            wf = sqrt(wf);
            wf = (1.0 + wf) / (1.0 - wf);
            #endif
            wf = abs(wf);
          }

          // clip the SWR at a reasonable value
          if (wf > maxSwr) wf = maxSwr;

          // store the final result
          m_SWR = wf;
   */
   
        Serial.print(fwd);
        Serial.print("\t");
        //quadratic fit
        Serial.print(quadratic(fwd));
        Serial.print("\t");
        Serial.print(ref);
        Serial.print("\t");
        //Serial.print(m_SWR);
        Serial.print("\n");
   
        showFWD(fwd);
      }
}

void showFWD(float value) {
  
  static float sv_value=-9999;

  if (value!=sv_value) {
    drawMeasure((int)quadratic(value),MAX_FWD,LBL_FWD,LCD_FWD_X,LCD_FWD_Y,LCD_FWD_W,LCD_FWD_H,0,LBL_FWD_MIN,LBL_FWD_MAX);
    sv_value=value;
  }
}

/*--------------------------------------------------
 * Quadratic function used for convert a non linear
 * value to watt
 -------------------------------------------------*/
float quadratic(float x){
  return FWD_QUADRATIC_A*pow(x,2)+FWD_QUADRATIC_B*x+FWD_QUADRATIC_C;
}

void getTouch(){

  uint16_t xpos, ypos;  //screen coordinates
  tp = ts.getPoint();   //tp.x, tp.y are ADC values
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //digitalWrite(YP, HIGH);  //because TFT control pins
  //digitalWrite(XM, HIGH);

    
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {
    int tft_height = tft.height();
    int tft_width = tft.width();

    xpos = map(tp.y, TS_LEFT, TS_RT, 0, tft_width);
    ypos = map(tp.x, TS_TOP, TS_BOT, 0, tft_height);
    
    tft.drawPixel(xpos,ypos,LCD_BTN_FG);
    if (ypos > (tft_height -LCD_BTN_H)) {
      int button_width=tft_width/BUTTONS;
      if (xpos > button_width*4){
          drawMessage (">>");
      } else if (xpos > button_width*3) {
          drawMessage ("AUTO");
      } else if (xpos > button_width*2) {
          drawMessage ("DOWN");
      } else if (xpos > button_width*1) {
          drawMessage ("UP");
      } else { //if (tp.x > 200) {
          drawMessage ("STBY");
      };

    };

}
}
