/*******************************************************************************
Title: eb104 Controller
Author: Corrado Gerbaldo - IU1BOW

--------------------------------------------------------------------------------
TODO: check temperature
TODO: check current_band
TODO: check PTT
TODO: check band AUTO
TODO: set TRANSMISSION / RECEPTION
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

/*------------------------------------------------------------------------------
Globals
------------------------------------------------------------------------------*/
enum button_state{off,on,null};

struct button {
  bool pressed;
  bool enabled;
  button_state state;
  char *label;
  int x;
  int y;
  int width;
  int height;
  int x_label;
  int y_label;
};

struct button buttons[BUTTONS];

float fwd;
float ref;
bands current_band;
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OBJ: Messenger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class Messenger {
  private:
    virtual void drawMessageGraphic() {
      tft.fillRect(LCD_SPACING, tft.height()-LCD_BTN_H-LCD_MSG_H-LCD_SPACING,
                    tft.width()-LCD_SPACING*2,LCD_MSG_H,LCD_MSG_BG);
      u8g2_for_adafruit_gfx.setFont(FNT_MESSAGE);
      u8g2_for_adafruit_gfx.setForegroundColor(LCD_MSG_FG);
      u8g2_for_adafruit_gfx.setBackgroundColor(LCD_MSG_BG);
      u8g2_for_adafruit_gfx.setCursor(LCD_MSG_X,
                          tft.height()-
                          LCD_BTN_H-(u8g2_for_adafruit_gfx.getFontAscent()+
                          u8g2_for_adafruit_gfx.getFontDescent()*-1)/2);
    }
  public:
    virtual void print (char msg[]) {
      this->drawMessageGraphic();
      u8g2_for_adafruit_gfx.print(msg);
    };
    virtual void print (int msg) {
      this->drawMessageGraphic();
      u8g2_for_adafruit_gfx.print(msg);
    };
};

Messenger* msg;    // base-class pointer


/*------------------------------------------------------------------------------
  Draw main screen
------------------------------------------------------------------------------*/
void drawMain(){
  tft.fillRect(0, 0, tft.width(), tft.height(), LCD_MAIN_BG);

  drawMeasure(0,MAX_FWD,LBL_FWD,LCD_FWD_X,LCD_FWD_Y,
              LCD_FWD_W,LCD_FWD_H,1,LBL_FWD_MIN,LBL_FWD_MAX);
  drawMeasure(0,MAX_REF,LBL_REF,LCD_REF_X,LCD_REF_Y,
              LCD_REF_W,LCD_REF_H,1,LBL_REF_MIN,LBL_REF_MAX);
  drawMeasure(0,MAX_SWR,LBL_SWR,LCD_SWR_X,LCD_SWR_Y,
              LCD_SWR_W,LCD_SWR_H,1,LBL_SWR_MIN,LBL_SWR_MAX);
  drawMeasure(0,MAX_TMP,LBL_TMP,LCD_TMP_X,LCD_TMP_Y,
              LCD_TMP_W,LCD_TMP_H,1,LBL_TMP_MIN,LBL_TMP_MAX);
  drawMeasure(0,MAX_V,LBL_V,LCD_V_X,LCD_V_Y,
              LCD_V_W,LCD_V_H,1,LBL_V_MIN,LBL_V_MAX);
  drawMeasure(0,MAX_I,LBL_I,LCD_I_X,LCD_I_Y,
              LCD_I_W,LCD_I_H,1,LBL_I_MIN,LBL_I_MAX);

  msg->print(LBL_INITIAL_MSG);
  drawButtons(true);

}

/*------------------------------------------------------------------------------
  Draw a measure object
------------------------------------------------------------------------------*/
void drawMeasure(long value, long max, char label[],int x, int y, int len,
                  int height,byte reset,char lbl_min[], char lbl_max[]) {

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
  drawMeasureBar(value,
                max,
                x+LCD_SPACING,
                y+LCD_SPACING*4+h_font,
                len-LCD_SPACING*2,
                height/3,
                LCD_SPACE_BAR,
                LCD_BORDER_BAR);
}

/*..............................................................................
  Draw a measure bar, inside a measure object
 .............................................................................*/
void drawMeasureBar(long value, long max, int x, int y, int len, int height,
                    int space, int border) {

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
void drawButtons(bool init) {

  byte idxBtn=0;

  int tft_height = tft.height();
  int tft_width = tft.width();

  for(int i=LCD_SPACING;i<tft_width;i=i+tft_width/BUTTONS) {
    if (init) {
      if (idxBtn==STDBY) {  //enabling only stdby button
        buttons[idxBtn].state=on;
        buttons[idxBtn].enabled=true;
      } else if (idxBtn==AUTO) {  //for auto button set the led to off
        buttons[idxBtn].state=on;
        buttons[idxBtn].enabled=false;
      } else {
       buttons[idxBtn].state=null;
       buttons[idxBtn].enabled=false;
      }
      buttons[idxBtn].pressed=false;
      buttons[idxBtn].width=tft_width/BUTTONS-LCD_SPACING*2;
      buttons[idxBtn].height=tft_height-LCD_BTN_H;
      buttons[idxBtn].x=LCD_SPACING+i;
      buttons[idxBtn].y=tft_height-LCD_BTN_H-LCD_SPACING*2;
      buttons[idxBtn].label=LBL_BUTTON[idxBtn];
      buttons[idxBtn].x_label=LCD_SPACING*2+i+
                    (buttons[idxBtn].width-
                    u8g2_for_adafruit_gfx.getUTF8Width(buttons[idxBtn].label))/2;
      buttons[idxBtn].y_label=tft.height()-
                    (LCD_BTN_H-u8g2_for_adafruit_gfx.getFontAscent()+
                    u8g2_for_adafruit_gfx.getFontDescent()*-1)/2;
    };
    drawSingleButton(buttons[idxBtn]);
    idxBtn++;
  }

}

/*.............................................................................
  Draw single button
   select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fnt
..............................................................................*/
void drawSingleButton(button b) {

  //set font
  u8g2_for_adafruit_gfx.setBackgroundColor(LCD_BTN_BG);
  u8g2_for_adafruit_gfx.setFont(FNT_BUTTONS);

  //draw buttons
  if (b.enabled == true) {
    u8g2_for_adafruit_gfx.setForegroundColor(LCD_BTN_FG);
  } else {
    u8g2_for_adafruit_gfx.setForegroundColor(LCD_BTN_FG_DIS);
  }

  if (b.pressed == true) {
    tft.fillRect(b.x,b.y,b.width,b.height,LCD_BTN_BG_PRES);
  } else {
    tft.fillRect(b.x,b.y,b.width,b.height,LCD_BTN_BG);
  }

  //write "led" only on buttons with state on or off
  if (b.state == on) {
    tft.fillCircle(b.x+b.width/10,b.y+b.width/10,b.width/25,LCD_BTN_ON);
  } else if (b.state == off) {
    tft.fillCircle(b.x+b.width/10,b.y+b.width/10,b.width/25,LCD_BTN_OFF);
  }

  //write labels
  u8g2_for_adafruit_gfx.setCursor(b.x_label, b.y_label);
  u8g2_for_adafruit_gfx.print(b.label);
}

void showFWD(float value) {

  static float sv_value=-9999;

  if (value!=sv_value) {
    drawMeasure((int)quadratic(value),
                MAX_FWD,
                LBL_FWD,
                LCD_FWD_X,
                LCD_FWD_Y,
                LCD_FWD_W,
                LCD_FWD_H,
                0,
                LBL_FWD_MIN,
                LBL_FWD_MAX);
    sv_value=value;
  }
}

/*------------------------------------------------------------------------------
 * Quadratic function used for convert a non linear
 * value to watt
 -----------------------------------------------------------------------------*/
float quadratic(float x){
  return FWD_QUADRATIC_A*pow(x,2)+FWD_QUADRATIC_B*x+FWD_QUADRATIC_C;
}

/*------------------------------------------------------------------------------
  Check if a button is pressed
-----------------------------------------------------------------------------*/

void getTouch(){

  static bool tftToRelease;
  static int xpos, ypos;  //screen coordinates
  static byte cntNotPress = MAX_CNT_NOT_PRESS;

  tp = ts.getPoint();   //tp.x, tp.y are ADC values
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (tp.z > MINPRESSURE) {
    if (cntNotPress == 0) {
      cntNotPress = MAX_CNT_NOT_PRESS;
      tftToRelease = 1;
      xpos = map(tp.y, TS_LEFT, TS_RT, 0, tft.width());
      ypos = map(tp.x, TS_TOP, TS_BOT, 0, tft.height());
    } else if (tftToRelease == 1) {
      tftToRelease = 0;
      if (ypos > (buttons[0].y)) {
        for(int i=0;i<BUTTONS;i++) {
          if ((xpos >= buttons[i].x) && (xpos <= buttons[i].x+buttons[i].width)) {
            if (i==STDBY) {
              mngSTDBY(&buttons[i]);
            } else if (i==UP) {
              mngUP(&buttons[i]);
            } else if (i==DOWN) {
              mngDOWN(&buttons[i]);
            } else if (i==AUTO) {
              mngAUTO(&buttons[i]);
            } else if (i==RESET) {
              mngRESET(&buttons[i]);
            }
          }
        }
      }
    }
  } else if (cntNotPress > 0) {
    cntNotPress--;
  }
}
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage STDBY button
  //TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngSTDBY(button *b) {

  msg->print(b->label);

  if (b->state==on) {
    b->state=off;
    digitalWrite(PIN_STDBY,LOW);
    if (buttons[AUTO].state == on) {
        buttons[UP].enabled = false;
        buttons[DOWN].enabled = false;
    } else {
      buttons[UP].enabled = true;
      buttons[DOWN].enabled = true;
    }
    buttons[AUTO].enabled = true;
    buttons[RESET].enabled = true;
  } else {
    digitalWrite(PIN_STDBY,HIGH);
    b->state=on;
    buttons[UP].enabled = false;
    buttons[DOWN].enabled = false;
    buttons[AUTO].enabled = false;
    buttons[RESET].enabled = false;
  }

  drawButtons(false);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage band up button
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngUP(button *b) {
  if (b->enabled) {
    current_band=changeBand(up,current_band);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage band down button
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngDOWN(button *b) {
  if (b->enabled) {
    current_band=changeBand(down,current_band);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage band auto button
//TODO: test / disable up and down
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngAUTO(button *b) {
  if (b->enabled) {
    if (b->state==on) {
      b->state=off;
      buttons[UP].enabled=true;
      buttons[DOWN].enabled=true;
    } else {
      b->state=on;
      buttons[UP].enabled=false;
      buttons[DOWN].enabled=false;
    }
    drawButtons(false);
    msg->print(b->label);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage reset button
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngRESET(button *b) {

  if (b->enabled) {
    msg->print(b->label);
    b->pressed=true;
    drawSingleButton(*b);
    digitalWrite(PIN_RESET,HIGH);
    delay(1000);
    digitalWrite(PIN_RESET,LOW);
    b->pressed=false;
    drawSingleButton(*b);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Change band manually
//TODO: test
//TODO: set filter
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
bands changeBand(direction dir,bands band) {

  bands new_band;
  if (dir==up) {
    if (band==BND10) {
      new_band=0;
    } else {
      new_band=band+1;
    }
  } else {
    if (band==BND40) {
      new_band=BND10;
    } else {
      new_band=band-1;
    }
  }

  setFilter(new_band);
  return new_band;
};


/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Change filter
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int setFilter(bands band) {

  int filter=PIN_FILTER_10;

  if (band==BND40) {
    filter=PIN_FILTER_40;
  } else if (band<=BND20) {
    filter=PIN_FILTER_20;
  } else if (band<=BND15) {
    filter=PIN_FILTER_15;
  };

  digitalWrite(PIN_FILTER_40,LOW);
  digitalWrite(PIN_FILTER_20,LOW);
  digitalWrite(PIN_FILTER_15,LOW);
  digitalWrite(PIN_FILTER_10,LOW);
  digitalWrite(filter,HIGH);

  return filter;
};
/******************************************************************************
SETUP
*******************************************************************************/
void setup() {
msg = new Messenger();
  //setup serial
  Serial.begin(9600);
  uint32_t when = millis();
  if (!Serial) delay(5000);           //allow some time for Leonardo
  Serial.println("Serial took " + String((millis() - when)) + "ms to start");

  //setup tft and fonts
  uint16_t ID = tft.readID(); //
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  tft.begin(ID);
  tft.setRotation(ORIENTATION);
  u8g2_for_adafruit_gfx.begin(tft);   // connect u8g2 procedures to Adafruit GFX

  //setup pins
  pinMode(PIN_STDBY,OUTPUT);
  pinMode(PIN_RESET,OUTPUT);
  pinMode(PIN_FILTER_40,OUTPUT);
  pinMode(PIN_FILTER_20,OUTPUT);
  pinMode(PIN_FILTER_15,OUTPUT);
  pinMode(PIN_FILTER_10,OUTPUT);

  digitalWrite(PIN_STDBY,HIGH);
  digitalWrite(PIN_RESET,LOW);
  digitalWrite(PIN_FILTER_40,LOW);
  digitalWrite(PIN_FILTER_20,HIGH);
  digitalWrite(PIN_FILTER_15,LOW);
  digitalWrite(PIN_FILTER_10,LOW);

  Serial.println("digital pins setted");

  //setup inital screen
  drawMain();
  Serial.println("End setup");
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
/*
    Serial.print(fwd);
    Serial.print("\t");
    //quadratic fit
    Serial.print(quadratic(fwd));
    Serial.print("\t");
    Serial.print(ref);
    Serial.print("\t");
    //Serial.print(m_SWR);
    Serial.print("\n");
*/
    showFWD(fwd);
  }
}
