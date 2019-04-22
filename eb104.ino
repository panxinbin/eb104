
 /*******************************************************************************
Title: eb104 Controller
Author: Corrado Gerbaldo - IU1BOW

--------------------------------------------------------------------------------
TODO: check temperature
TODO: check current_band
TODO: check PTT
TODO: check band AUTO
TODO: set TRANSMISSION / RECEPTION
TODO: inherit and extend class button
TODO: create object of measure bars
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
float fwd;
float ref;
bands current_band;
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLASS: Messenger
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
      drawMessageGraphic();
      u8g2_for_adafruit_gfx.print(msg);
    };
    virtual void print (int msg) {
      drawMessageGraphic();
      u8g2_for_adafruit_gfx.print(msg);
    };
};

Messenger* msg;    // base-class pointer


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLASS: Button and their derived
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class Button {
 public:
    enum button_state{off,on,null};
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
    int pin;
/*.............................................................................
  constructor
..............................................................................*/
    Button (int pos, MCUFRIEND_kbv *tft, char *lbl) {
      state=null;
      enabled=false;
      pressed=false;
      tft_height = tft->height();
      tft_width = tft->width();
      position=pos*tft_width/BUTTONS;
      width=tft_width/BUTTONS-LCD_SPACING*2;
      height=tft_height-LCD_BTN_H;
      x=LCD_SPACING+position;
      y=tft_height-LCD_BTN_H-LCD_SPACING*2;
      setLabel(lbl);
    };

/*.............................................................................
  Set label of the button
..............................................................................*/
    setLabel (char *lbl) {
      label=lbl;
      x_label=LCD_SPACING*2+position+
                    (width-
                    u8g2_for_adafruit_gfx.getUTF8Width(label))/2;
      y_label=tft_height-
                    (LCD_BTN_H-u8g2_for_adafruit_gfx.getFontAscent()+
                    u8g2_for_adafruit_gfx.getFontDescent()*-1)/2;
    };

/*.............................................................................
  Draw button
   select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fnt
   TODO: use tft as pointer
..............................................................................*/
    draw() {
      //set font
      u8g2_for_adafruit_gfx.setBackgroundColor(LCD_BTN_BG);
      u8g2_for_adafruit_gfx.setFont(FNT_BUTTONS);

      //draw buttons
      if (enabled == true) {
        u8g2_for_adafruit_gfx.setForegroundColor(LCD_BTN_FG);
      } else {
        u8g2_for_adafruit_gfx.setForegroundColor(LCD_BTN_FG_DIS);
      }

      if (pressed == true) {
        tft.fillRect(x,y,width,height,LCD_BTN_BG_PRES);
      } else {
        tft.fillRect(x,y,width,height,LCD_BTN_BG);
      }

      //write "led" only on buttons with state on or off
      if (state == button_state::on) {
        tft.fillCircle(x+width/10,y+width/10,width/25,LCD_BTN_ON);
      } else if (state == button_state::off) {
        tft.fillCircle(x+width/10,y+width/10,width/25,LCD_BTN_OFF);
      }

      //write labels
      u8g2_for_adafruit_gfx.setCursor(x_label, y_label);
      u8g2_for_adafruit_gfx.print(label);
    };

  private:
    int position;
    int tft_height;
    int tft_width;
};

class ButtonStdby: public Button  {
  public:
    ButtonStdby (int position, void *tft, char *lbl):Button(position, tft, lbl)  {
      state=on;
      enabled=true;
    };
    pinInit(int pin) {
      this->pin = pin;
      digitalWrite(this->pin, HIGH);
    };
};

class ButtonAuto: public Button {
  public:
    ButtonAuto (int position, void *tft, char *lbl):Button(position, tft, lbl) {
      state=on;
      enabled=false;
    };
};

class ButtonReset: public Button {
  public:
    ButtonReset (int position, void *tft,char *lbl ):Button(position, tft, lbl) {
    };
    pinInit(int pin) {
      this->pin = pin;
      digitalWrite(this->pin, LOW);
    };
};

ButtonStdby* btnSTDBY;
Button* btnUP;
Button* btnDOWN;
ButtonAuto* btnAUTO;
ButtonReset* btnRESET;

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
  drawButtons();

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
  };
};

/*------------------------------------------------------------------------------
  Draw buttons
 -----------------------------------------------------------------------------*/
void drawButtons() {

  btnSTDBY->draw();
  btnUP->draw();
  btnDOWN->draw();
  btnAUTO->draw();
  btnRESET->draw();
};

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
      if (ypos > btnSTDBY->y) {
        if ((xpos >= btnSTDBY->x) && (xpos <= btnSTDBY->x+btnSTDBY->width)) {
            mngSTDBY();
        } else if ((xpos >= btnUP->x) && (xpos <= btnUP->x+btnUP->width)) {
          mngUP();
        } else if ((xpos >= btnDOWN->x) && (xpos <= btnDOWN->x+btnDOWN->width)) {
          mngDOWN();
        } else if ((xpos >= btnAUTO->x) && (xpos <= btnAUTO->x+btnAUTO->width)) {
          mngAUTO();
        } else {
          mngRESET();
        }
      }
    }
  } else if (cntNotPress > 0) {
    cntNotPress--;
  }
};
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage STDBY button
  //TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngSTDBY() {

  if (btnSTDBY->state==Button::button_state::on) {
    btnSTDBY->state=Button::button_state::off;
    digitalWrite(btnSTDBY->pin,LOW);
    if (btnAUTO->state == Button::button_state::on) {
        btnUP->enabled = false;
        btnDOWN->enabled = false;
    } else {
      btnUP->enabled = true;
      btnDOWN->enabled = true;
    }
    btnAUTO->enabled = true;
    btnRESET->enabled = true;
  } else {
    digitalWrite(btnSTDBY->pin,HIGH);
    btnSTDBY->state=Button::button_state::on;
    btnUP->enabled = false;
    btnDOWN->enabled = false;
    btnAUTO->enabled = false;
    btnRESET->enabled = false;
  }
  drawButtons();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage band up button
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngUP() {
  if (btnUP->enabled) {
    current_band=changeBand(up,current_band);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage band down button
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngDOWN() {
  if (btnDOWN->enabled) {
    current_band=changeBand(down,current_band);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage band auto button
//TODO: test / disable up and down
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngAUTO() {
  if (btnAUTO->enabled) {
    if (btnAUTO->state==Button::button_state::on) {
      btnAUTO->state=Button::button_state::off;
      btnUP->enabled=true;
      btnDOWN->enabled=true;
    } else {
      btnAUTO->state=Button::button_state::on;
      btnUP->enabled=false;
      btnDOWN->enabled=false;
    }
    drawButtons();
    msg->print(btnAUTO->label);
  }
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Manage reset button
//TODO: test
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void mngRESET() {

  if (btnRESET->enabled) {
    msg->print(btnRESET->label);
    btnRESET->pressed=true;
    btnRESET->draw();
    digitalWrite(btnRESET->pin,HIGH);
    delay(1000);
    digitalWrite(btnRESET->pin,LOW);
    btnRESET->pressed=false;
    btnRESET->draw();
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

  msg->print(new_band);
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

  //init objects
  msg = new Messenger();
  btnSTDBY=new ButtonStdby(STDBY, &tft, LBL_STBY);
  btnSTDBY->pinInit(PIN_STDBY);
  btnUP=new Button(UP, &tft, LBL_UP);
  btnDOWN=new Button(DOWN, &tft, LBL_DOWN);
  btnAUTO=new ButtonAuto(AUTO, &tft, LBL_AUTO);
  btnRESET=new ButtonReset(RESET, &tft, LBL_RESET);
  btnRESET->pinInit(PIN_RESET);

  //setup pins
  pinMode(PIN_FILTER_40,OUTPUT);
  pinMode(PIN_FILTER_20,OUTPUT);
  pinMode(PIN_FILTER_15,OUTPUT);
  pinMode(PIN_FILTER_10,OUTPUT);

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
