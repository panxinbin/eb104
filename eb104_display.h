
//colors: see http://www.barth-dev.de/online/rgb565-color-picker/
#define LCD_MAIN_BG     0x4A69  //main background color

#define LCD_MSR_FG      0xFFFF  //measure box fg
#define LCD_MSR_BG      0x3186  //measure box main label fg
#define LCD_MAX_MIN_FG  0xD6DB  //measure box min max label

#define LCD_BAR_FG      0x3D1F  //measure bar fg
#define LCD_BAR_BG      0x0000  //measure bar bg

#define LCD_MSG_FG      0x3D1F  //status bar foreground color
#define LCD_MSG_BG      0x3186  //status bar foreground color

#define LCD_BTN_FG      0xFFFF  //button fg
#define LCD_BTN_BG      0x3B57  //button bg
#define LCD_BTN_BG_PRES 0x04BF  //button bg pressed
#define LCD_BTN_FG_DIS  0x3186  //button fg disabled
#define LCD_BTN_ON      0x17E0  //button on
#define LCD_BTN_OFF     0x4288  //button off

//fonts
#define FNT_MAX_MIN     u8g2_font_profont15_tf
#define FNT_MEASURE     u8g2_font_profont22_tf
#define FNT_BUTTONS     u8g2_font_profont29_tf
#define FNT_MESSAGE     u8g2_font_profont22_tf

//positions and dimensions
static const int ORIENTATION=1;
//FWD box
static const int LCD_FWD_X=3;
static const int LCD_FWD_Y=1;
static const int LCD_FWD_W=475;
static const int LCD_FWD_H=40;
//SWR box
static const int LCD_SWR_X=3;
static const int LCD_SWR_Y=45;
static const int LCD_SWR_W=475;
static const int LCD_SWR_H=40;
//REFLECTED box
static const int LCD_REF_X=3;
static const int LCD_REF_Y=90;
static const int LCD_REF_W=475/2-3;
static const int LCD_REF_H=40;
//TEMPERATURE box
static const int LCD_TMP_X=3+475/2;
static const int LCD_TMP_Y=90;
static const int LCD_TMP_W=475/2;
static const int LCD_TMP_H=40;
//VOLTAGE BOX
static const int LCD_V_X=3;
static const int LCD_V_Y=135;
static const int LCD_V_W=475/2-3;
static const int LCD_V_H=40;
//CURRENT BOX
static const int LCD_I_X=3+475/2;
static const int LCD_I_Y=135;
static const int LCD_I_W=475/2;
static const int LCD_I_H=40;
//measure bar configuration
static const int LCD_SPACE_BAR=10;
static const int LCD_BORDER_BAR=2;

//space betwen elements
static const int LCD_SPACING=2;
//number of buttons on the base line
static const int BUTTONS=5;
//buttons height
static const int LCD_BTN_H=60;

//message bar
static const int LCD_MSG_H=30;
static const int LCD_MSG_X=10;
