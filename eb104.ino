
 /*******************************************************************************
Title: eb104 Controller
Author: Corrado Gerbaldo - IU1BOW

--------------------------------------------------------------------------------
TODO: check temperature
TODO: check current_band
TODO: check PTT
TODO: check band AUTO
TODO: set TRANSMISSION / RECEPTION
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
    U8G2_FOR_ADAFRUIT_GFX *gfx;
    MCUFRIEND_kbv* tft;
    virtual void drawMessageGraphic() {
      tft->fillRect(LCD_SPACING, tft->height()-LCD_BTN_H-LCD_MSG_H-LCD_SPACING,
                    tft->width()-LCD_SPACING*2,LCD_MSG_H-LCD_SPACING,LCD_MSG_BG);
      gfx->setFont(FNT_MESSAGE);
      gfx->setForegroundColor(LCD_MSG_FG);
      gfx->setBackgroundColor(LCD_MSG_BG);
      gfx->setCursor(LCD_MSG_X,
                          tft->height()-
                          LCD_BTN_H-(gfx->getFontAscent()+
                          gfx->getFontDescent()*-1)/2);
    };
  public:
    void begin(MCUFRIEND_kbv* tft,U8G2_FOR_ADAFRUIT_GFX *gfx){
      this->tft=tft;
      this->gfx=gfx;
    };
    void print (char msg[]) {
      drawMessageGraphic();
      gfx->print(msg);
    };
    void print (int msg) {
      drawMessageGraphic();
      gfx->print(msg);
    };

};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLASS: label
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class Label {

 private:
  U8G2_FOR_ADAFRUIT_GFX *gfx;
  int x;
  int y;

 public:
    char *label;

    void begin(char *lbl,U8G2_FOR_ADAFRUIT_GFX *gfx){
      label=lbl;
      this->gfx=gfx;
    };

    void setXY(int x, int y) {
      this->x=x;
      this->y=y;
    };

    void setForegroundColor(int color) {
      gfx->setForegroundColor(color);
    };

    void draw() {
      gfx->setFont(FNT_BUTTONS);
      gfx->setCursor(x, y);
      gfx->print(label);
    };
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLASS: Button and their derived
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class Button {
private:
  MCUFRIEND_kbv *tft;
  U8G2_FOR_ADAFRUIT_GFX *gfx;
  Label lbl;
  int position;
  int x;
  int y;
  int width;
  Messenger msg;

  public:
  enum button_state{off,on,null};
  bool pressed;
  bool enabled;
  button_state state;
  char *label;

  int height;
  int pin;
  /*..........................................................................
    constructor
  ..........................................................................*/
  Button (int x, int y, MCUFRIEND_kbv *tft, U8G2_FOR_ADAFRUIT_GFX *gfx) {
    this->tft=tft;
    this->gfx=gfx;
    this->x=x;
    this->y=y;
    initStates();
  };

  void setMessenger(Messenger m){
    this->msg=m;
  };

  /*..........................................................................
    set width
  ..........................................................................*/
  void setWidth(int w) {
    this->width=w;
  };

  /*..........................................................................
    set HEIGHT
  ..........................................................................*/
  void setHeight(int h) {
    this->height=h;
  };

  /*..........................................................................
    set label
  ..........................................................................*/
  void setLabel(char* l) {
    this->label=l;
    lbl.begin(label,this->gfx);
    gfx->setFont(FNT_BUTTONS);
    lbl.setXY(
      x+(width-this->gfx->getUTF8Width(label))/2,
      this->tft->height()-(LCD_BTN_H-this->gfx->getFontAscent()+
                  this->gfx->getFontDescent()*-1)/2);
  };

  /*..........................................................................
    set coordinates
  ..........................................................................*/
  void setXY(int x, int y) {
    this->x=x;
    this->y=y;
  };

  /*..........................................................................
    return x coordinate
  ..........................................................................*/
  int getX() {
    return x;
  };

  /*..........................................................................
    return y coordinate
  ..........................................................................*/
  int getY() {
    return y;
  };

  /*..........................................................................
    return width
  ..........................................................................*/
  int getWidth() {
    return width;
  };
  /*..........................................................................
    set states
  ..........................................................................*/
  virtual void initStates() {
    state=null;
    enabled=false;
    pressed=false;
  };
  /*..........................................................................
  Draw button
  select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fnt
  TODO: use tft as pointer
  ..........................................................................*/
  void draw() {

    //set font
    gfx->setBackgroundColor(LCD_BTN_BG);

    if (enabled == true) {
      lbl.setForegroundColor(LCD_BTN_FG);
    } else {
      lbl.setForegroundColor(LCD_BTN_FG_DIS);
    }

    if (pressed == true) {
      tft->fillRect(x,y,width,height,LCD_BTN_BG_PRES);
    } else {
      tft->fillRect(x,y,width,height,LCD_BTN_BG);
    }

    //write "led" only on buttons with state on or off
    if (state == button_state::on) {
      tft->fillCircle(x+width/10,y+width/10,width/25,LCD_BTN_ON);
    } else if (state == button_state::off) {
      tft->fillCircle(x+width/10,y+width/10,width/25,LCD_BTN_OFF);
    }

    //write labels
    lbl.draw();
  };
};

class ButtonStdby: public Button  {

  public:
    ButtonStdby (int x, int y, void *tft, void *gfx):Button(x, y, tft, gfx) {
      initStates();
    };
    void initStates() override {
      state=on;
      enabled=true;
    };
    void pinBind(int pin) {
      this->pin = pin;
      digitalWrite(this->pin, HIGH);
    };
};

class ButtonAuto: public Button {
  public:
    ButtonAuto (int x, int y, void *tft, void *gfx):Button(x, y, tft, gfx) {
      initStates();
    };
    void initStates() override {
      state=on;
      enabled=false;
    };
};

class ButtonReset: public Button {
  public:
    ButtonReset (int x, int y, void *tft, void *gfx):Button(x, y, tft, gfx) {
    };
    void pinBind(int pin) {
      this->pin = pin;
      digitalWrite(this->pin, LOW);
    };
};

class ButtonContainer {
  private:

    int x;
    int y;
    ButtonStdby* btnSTDBY;
    Button* btnUP;
    Button* btnDOWN;
    ButtonAuto* btnAUTO;
    ButtonReset* btnRESET;
    Messenger msg;

    MCUFRIEND_kbv *tft;
    U8G2_FOR_ADAFRUIT_GFX *gfx;

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Manage STDBY button
      //TODO: test
    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    void mngSTDBY() {

      btnSTDBY->draw();

      if (btnSTDBY->state==Button::button_state::on) {
        btnSTDBY->state=Button::button_state::off;
        digitalWrite(btnSTDBY->pin,LOW);
        if (btnAUTO->state == Button::button_state::on) {
            btnUP->enabled = false;
            btnDOWN->enabled = false;
        } else {
          btnUP->enabled = true;
          btnUP->draw();
          btnDOWN->enabled = true;
          btnDOWN->draw();
        }
        btnAUTO->enabled = true;
        btnAUTO->draw();
        btnRESET->enabled = true;
        btnRESET->draw();
      } else {
          digitalWrite(btnSTDBY->pin,HIGH);
          btnSTDBY->state=Button::button_state::on;
          btnUP->enabled = false;
          btnDOWN->enabled = false;
          btnAUTO->enabled = false;
          btnRESET->enabled = false;
          this->draw();
      };
    };

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Manage band up button
    //TODO: test
    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    void mngUP() {
      if (btnUP->enabled) {
        current_band=changeBand(up,current_band);
      }
    };

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Manage band down button
    //TODO: test
    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    void mngDOWN() {
      if (btnDOWN->enabled) {
        current_band=changeBand(down,current_band);
      }
    };

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
        btnUP->draw();
        btnDOWN->draw();
        btnAUTO->draw();
        msg.print(btnAUTO->label);
      }
    };

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Manage reset button
    //TODO: test
    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    void mngRESET() {

      if (btnRESET->enabled) {
        msg.print(btnRESET->label);
        btnRESET->pressed=true;
        btnRESET->draw();
        digitalWrite(btnRESET->pin,HIGH);
        delay(1000);
        digitalWrite(btnRESET->pin,LOW);
        btnRESET->pressed=false;
        btnRESET->draw();
      }
    };

  public:
    /*..........................................................................
      Constructor
    ..........................................................................*/
    ButtonContainer() {
    };

    void setMessenger(Messenger m){
      this->msg=m;
    };
    void begin(MCUFRIEND_kbv *tft, U8G2_FOR_ADAFRUIT_GFX *gfx) {
//TODO: eliminate declare of local variables tft

      this->tft=tft;
      this->gfx=gfx;

      //build all buttons
      btnSTDBY=new ButtonStdby(LCD_SPACING+(STDBY*this->tft->width()/BUTTONS),y, this->tft, this->gfx);
      btnSTDBY->setMessenger(this->msg);
      btnSTDBY->setWidth(tft->width()/BUTTONS-LCD_SPACING*2);
      btnSTDBY->setHeight(tft->height()-LCD_BTN_H);
      btnSTDBY->setLabel(LBL_STBY);
      btnSTDBY->pinBind(PIN_STDBY);

      btnUP=new Button(LCD_SPACING+(UP*this->tft->width()/BUTTONS),y,this->tft, this->gfx);
      btnUP->setMessenger(this->msg);
      btnUP->setWidth(tft->width()/BUTTONS-LCD_SPACING*2);
      btnUP->setHeight(tft->height()-LCD_BTN_H);
      btnUP->setLabel(LBL_UP);

      btnDOWN=new Button(LCD_SPACING+(DOWN*this->tft->width()/BUTTONS),y, this->tft, this->gfx);
      btnDOWN->setMessenger(this->msg);
      btnDOWN->setWidth(tft->width()/BUTTONS-LCD_SPACING*2);
      btnDOWN->setHeight(tft->height()-LCD_BTN_H);
      btnDOWN->setLabel(LBL_DOWN);

      btnAUTO=new ButtonAuto(LCD_SPACING+(AUTO*this->tft->width()/BUTTONS),y, this->tft, this->gfx);
      btnAUTO->setMessenger(this->msg);
      btnAUTO->setWidth(tft->width()/BUTTONS-LCD_SPACING*2);
      btnAUTO->setHeight(tft->height()-LCD_BTN_H);
      btnAUTO->setLabel(LBL_AUTO);

      btnRESET=new ButtonReset(LCD_SPACING+(RESET*this->tft->width()/BUTTONS),y,this->tft, this->gfx);
      btnRESET->setMessenger(this->msg);
      btnRESET->setWidth(tft->width()/BUTTONS-LCD_SPACING*2);
      btnRESET->setHeight(tft->height()-LCD_BTN_H);
      btnRESET->setLabel(LBL_RESET);
      btnRESET->pinBind(PIN_RESET);

    };

    /*..........................................................................
      set coordinates
    ..........................................................................*/
    void setXY(int x, int y) {
      this->x=x;
      this->y=y;
    };

    /*..........................................................................
      return y coordinate
    ..........................................................................*/
    int getY() {
      return y;
    };

    /*..........................................................................
      press a button in the container
    ..........................................................................*/
    void pressed(int x, int y) {
      if ((x >= btnSTDBY->getX()) && (x <= btnSTDBY->getX()+btnSTDBY->getWidth())) {
        mngSTDBY();
      } else if ((x >= btnUP->getX()) && (x <= btnUP->getX()+btnUP->getWidth())) {
        mngUP();
      } else if ((x >= btnDOWN->getX()) && (x <= btnDOWN->getX()+btnDOWN->getWidth())) {
        mngDOWN();
      } else if ((x >= btnAUTO->getX()) && (x <= btnAUTO->getX()+btnAUTO->getWidth())) {
        mngAUTO();
      } else {
        mngRESET();
      }
    };

    /*..........................................................................
      draw all buttons
    ..........................................................................*/
    void draw() {

      btnSTDBY->draw();
      btnUP->draw();
      btnDOWN->draw();
      btnAUTO->draw();
      btnRESET->draw();

    };

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+
    Change band manually
    //TODO: test
    //TODO: set filter
    //TODO: externalize and raise an event on button pressed
    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
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
      msg.print(new_band);
      return new_band;
    };

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Change filter
    //TODO: test
    TODO: externaize and raise on event
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

};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLASS: MeasureBar
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class MeasureBar {
  private:
    int x;
    int y;
    int x_curs;
    int h_font;
    int lbl_ln;
    int len;
    int min;
    int max;
    int height;
    int border;
    char *label;
    int space;

    MCUFRIEND_kbv *tft;
    U8G2_FOR_ADAFRUIT_GFX *gfx;

    int sv_value;
    int lc_x;
    int lc_y;
    int lc_len;
    int lc_height;

    int clear_rect_len;

    uint8_t * font;

  public:
    /*..........................................................................
      Constructor
    ..........................................................................*/
    MeasureBar(int x, int y, int len, int height, long min, long max, char *label, char *lbl_min, char *lbl_max, int border, int space, MCUFRIEND_kbv *tft, U8G2_FOR_ADAFRUIT_GFX *gfx) {
      this->tft=tft;
      this->gfx=gfx;
      this->x=x;
      this->y=y;
      this->len=len;
      this->min=min;
      this->max=max;
      this->height=height;
      this->border=border;
      this->label=label;
      this->space=space;
      this->font=FNT_MEASURE;

      sv_value=-9999;

      //draw measure box
      tft->fillRect(x,y,len,height,LCD_MSR_BG);

      //print min and max labels
      gfx->setFont(FNT_MAX_MIN);
      h_font=gfx->getFontAscent()+gfx->getFontDescent()*-1;
      gfx->setForegroundColor(LCD_MAX_MIN_FG);
      gfx->setCursor(x+LCD_SPACING,y+LCD_SPACING*2+h_font);
      gfx->print(lbl_min);
      lbl_ln=gfx->getUTF8Width(lbl_max);
      gfx->setCursor(x+len-lbl_ln,y+LCD_SPACING*2+h_font);
      gfx->print(lbl_max);

      //print main label
      gfx->setFont(this->font);
      h_font=gfx->getFontAscent()+gfx->getFontDescent()*-1;
      gfx->setForegroundColor(LCD_MSR_FG);
      x_curs=x+(len-gfx->getUTF8Width(label))/2;
      gfx->setCursor(x_curs,y+LCD_SPACING*2+h_font);
      gfx->print(label);

      //calculating the lenght of the rectangle we use to clear the measure label
      int number_digits = log10(max)+1;
      char spaces[number_digits+1];
      memset(spaces, ' ', number_digits);
      spaces[number_digits] = '\0';
      this->clear_rect_len = gfx->getUTF8Width(spaces);

      //draw values line
      tft->drawLine(x+LCD_SPACING,
                  2+y+LCD_SPACING*4+h_font+height/3,
                  x+len,
                  2+y+LCD_SPACING*4+h_font+height/3,LCD_MSR_FG);

      //calculating  coordinates for measurement bar
      lc_x = x + LCD_SPACING;
      lc_y = y+LCD_SPACING*4+h_font;
      lc_len = len-LCD_SPACING*2;
      lc_height = height/3;

      lbl_ln=gfx->getUTF8Width(label);
      x_curs=x+lbl_ln+(lc_len-lbl_ln)/2;
      h_font=gfx->getFontAscent()+gfx->getFontDescent()*-1;

      draw(min);

    };

    /*..........................................................................
      draw a measure bar
    ..........................................................................*/
    void draw(long value) {

      if (value!=sv_value) {
        sv_value=value;
        //clear value
        tft->fillRect(x_curs,y+LCD_SPACING*2,clear_rect_len,h_font,LCD_MSR_BG);
        //print value
        gfx->setFont(font);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fnt
        gfx->setBackgroundColor(LCD_MSR_BG);
        gfx->setForegroundColor(LCD_MSR_FG);
        gfx->setCursor(x_curs,y+LCD_SPACING*2+h_font);
        gfx->print(value);

        //draw bar
        int filled =  lc_len*value/max;
        for(int i=0;i<lc_len-space;i=i+space) {
          if (i>=filled) {
            tft->fillRect(lc_x+i,lc_y,space-border,lc_height,LCD_BAR_BG);
          } else
            tft->fillRect(lc_x+i,lc_y,space-border,lc_height,LCD_BAR_FG);
        };
      };
    };
};

Messenger msg;    // base-class pointer
ButtonContainer* btnCont;
MeasureBar* mbFwd;
MeasureBar* mbRef;
MeasureBar* mbSwr;
MeasureBar* mbTmp;
MeasureBar* mbV;
MeasureBar* mbI;

/*------------------------------------------------------------------------------
  Draw main screen
------------------------------------------------------------------------------*/
void drawMain(){

  tft.fillRect(0, 0, tft.width(), tft.height(), LCD_MAIN_BG);

  msg.print(LBL_INITIAL_MSG);

  mbFwd=new MeasureBar(LCD_FWD_X,LCD_FWD_Y,LCD_FWD_W,LCD_FWD_H,0,MAX_FWD,LBL_FWD,LBL_FWD_MIN,LBL_FWD_MAX,LCD_BORDER_BAR,LCD_SPACE_BAR,&tft,&u8g2_for_adafruit_gfx);
  mbRef=new MeasureBar(LCD_REF_X,LCD_REF_Y,LCD_REF_W,LCD_REF_H,0,MAX_REF,LBL_REF,LBL_REF_MIN,LBL_REF_MAX,LCD_BORDER_BAR,LCD_SPACE_BAR,&tft,&u8g2_for_adafruit_gfx);
  mbSwr=new MeasureBar(LCD_SWR_X,LCD_SWR_Y,LCD_SWR_W,LCD_SWR_H,0,MAX_SWR,LBL_SWR,LBL_SWR_MIN,LBL_SWR_MAX,LCD_BORDER_BAR,LCD_SPACE_BAR,&tft,&u8g2_for_adafruit_gfx);
  mbTmp=new MeasureBar(LCD_TMP_X,LCD_TMP_Y,LCD_TMP_W,LCD_TMP_H,0,MAX_TMP,LBL_TMP,LBL_TMP_MIN,LBL_TMP_MAX,LCD_BORDER_BAR,LCD_SPACE_BAR,&tft,&u8g2_for_adafruit_gfx);
  mbV=new MeasureBar(LCD_V_X,LCD_V_Y,LCD_V_W,LCD_V_H,0,MAX_V,LBL_V,LBL_V_MIN,LBL_V_MAX,LCD_BORDER_BAR,LCD_SPACE_BAR,&tft,&u8g2_for_adafruit_gfx);
  mbI=new MeasureBar(LCD_I_X,LCD_I_Y,LCD_I_W,LCD_I_H,0,MAX_I,LBL_I,LBL_I_MIN,LBL_I_MAX,LCD_BORDER_BAR,LCD_SPACE_BAR,&tft,&u8g2_for_adafruit_gfx);
  Serial.println("MeasureBar created");

  btnCont=new ButtonContainer(); //TODO: portarla fuori?
  btnCont->setMessenger(msg);
  btnCont->setXY(0,tft.height()-LCD_BTN_H-LCD_SPACING*2);
  btnCont->begin(&tft,&u8g2_for_adafruit_gfx);
  btnCont->draw();
  Serial.println("ButtonContainer created");
  Serial.println("Main screen drawed");

};
/*------------------------------------------------------------------------------
 * Quadratic function used for convert a non linear
 * value to watt
 -----------------------------------------------------------------------------*/
float quadratic(float x){
  return FWD_QUADRATIC_A*pow(x,2)+FWD_QUADRATIC_B*x+FWD_QUADRATIC_C;
};

/*------------------------------------------------------------------------------
  Check if a button is pressed
-----------------------------------------------------------------------------*/

void checkTouch(){

  static bool tftToRelease;
  static int xpos, ypos;  //screen coordinates
  static byte cntNotPress = MAX_CNT_NOT_PRESS;

  tp = ts.getPoint();
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
      if (ypos > btnCont->getY()) {
        btnCont->pressed(xpos,ypos);
      }
    }
  } else if (cntNotPress > 0) {
    cntNotPress--;
  }
};

/******************************************************************************
SETUP
*******************************************************************************/
void setup() {

  //setup serial
  Serial.begin(9600);
  uint32_t when = millis();
  if (!Serial) delay(5000);
  Serial.println("Serial took " + String((millis() - when)) + "ms to start");

  //setup tft and fonts
  uint16_t ID = tft.readID(); //
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  tft.begin(ID);
  tft.setRotation(ORIENTATION);
  u8g2_for_adafruit_gfx.begin(tft);   // connect u8g2 procedures to Adafruit GFX
  msg.begin(&tft,&u8g2_for_adafruit_gfx);

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
};

/******************************************************************************
MAIN LOOP
*******************************************************************************/
void loop() {

  static int nr;

  checkTouch();
/*  FWD measures
 *
 *  quadratic fit {130,5},{180,10},{330,30},{390,50},{460,70},{580,100}
 *  https://www.wolframalpha.com/input/
 */
  nr++;
  fwd=fwd+analogRead(sensorPinFWD);
  // read reverse voltage
  ref=ref+analogRead(sensorPinREF);
  if (nr>AVG_CNT) {
    fwd=fwd/nr;
    //ref=ref/nr;
    nr=0;
    mbFwd->draw((int)quadratic(fwd));
    //mbFwd->draw(100);
    mbRef->draw(3);
    mbTmp->draw(30);
    mbSwr->draw(3);
    mbV->draw(40);
    mbI->draw(5);
    //mbRef->draw((int)ref);
  };
};




/*
      // compute SWR and return
      float wf;
      if (TMP == 0 || fwd < m_MinPower) {
        wf = 1.0;
      } else if (TMP >= fwd) {
        wf = maxSwr;
      } else {
        #ifdef USE_VOLTAGE_CALC
        wf = (float)(fwd + TMP) / (float)(fwd - TMP);
        #else
        wf = (float)fwd / (float)TMP;
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
    Serial.print(TMP);
    Serial.print("\t");
    //Serial.print(m_SWR);
    Serial.print("\n");
*/
