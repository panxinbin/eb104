
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

//fonts
/*
#define FNT_MAX_MIN     u8g2_font_7x13B_mr
#define FNT_MEASURE     u8g2_font_8x13B_tr
#define FNT_BUTTONS     u8g2_font_helvR14_tf
#define FNT_MESSAGE     u8g2_font_helvR14_tf
*/


#define FNT_MAX_MIN     u8g2_font_profont15_tf
#define FNT_MEASURE     u8g2_font_profont22_tf
#define FNT_BUTTONS     u8g2_font_profont29_tf
#define FNT_MESSAGE     u8g2_font_profont22_tf

/*

#define FNT_MAX_MIN     u8g2_font_inr16_mf
#define FNT_MEASURE     u8g2_font_inr19_mf
#define FNT_BUTTONS     u8g2_font_inr21_mf
#define FNT_MESSAGE     u8g2_font_inr19_mf
*/
//positions and dimensions
const int ORIENTATION=1;
//FWD box
const int LCD_FWD_X=3;
const int LCD_FWD_Y=1;
const int LCD_FWD_W=475;
const int LCD_FWD_H=40;
//SWR box
const int LCD_SWR_X=3;
const int LCD_SWR_Y=45;
const int LCD_SWR_W=475;
const int LCD_SWR_H=40;
//REFLECTED box
const int LCD_REF_X=3;
const int LCD_REF_Y=90;
const int LCD_REF_W=475/2-3;
const int LCD_REF_H=40;
//TEMPERATURE box
const int LCD_TMP_X=3+475/2;
const int LCD_TMP_Y=90;
const int LCD_TMP_W=475/2;
const int LCD_TMP_H=40;
//VOLTAGE BOX
const int LCD_V_X=3;
const int LCD_V_Y=135;
const int LCD_V_W=475/2-3;
const int LCD_V_H=40;
//CURRENT BOX
const int LCD_I_X=3+475/2;
const int LCD_I_Y=135;
const int LCD_I_W=475/2;
const int LCD_I_H=40;
//measure bar configuration
const int LCD_SPACE_BAR=10;
const int LCD_BORDER_BAR=2;

//space betwen elements
const int LCD_SPACING=2;
//number of buttons on the base line
const int BUTTONS=5;
//buttons height
const int LCD_BTN_H=60;

//message bar
const int LCD_MSG_H=30;
const int LCD_MSG_X=10;
