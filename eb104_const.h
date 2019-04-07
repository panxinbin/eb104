//lcd setup
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

//touchscreen dimensions
static const int XP=8,XM=A2,YP=A3,YM=9; //320x480 ID=0x9486
static const int TS_LEFT=957,TS_RT=93,TS_TOP=909,TS_BOT=124;

//analogic pins
static const int sensorPinFWD = A13;    // GIALLO
static const int sensorPinREF = A12;    // BIANCO

//digital pins
static const int PIN_STDBY = 43;
static const int PIN_RESET = 41;
static const int PIN_BND_40 = 39;
static const int PIN_BND_20 = 37;
static const int PIN_BND_15 = 35;
static const int PIN_BND_10 = 33;

//generic constant
static const int MAX_FWD = 600;
static const int MAX_REF = 100;
static const float MAX_SWR = 5.0;
static const int MAX_TMP = 100;
static const float MAX_V = 52.0;
static const float MAX_I = 26;
static const int MINPRESSURE=30;
static const int MAXPRESSURE=1000;
static const byte MAX_CNT_NOT_PRESS=14;

static const float FWD_QUADRATIC_A=0.000249622;
static const float FWD_QUADRATIC_B=0.0399433;
static const float FWD_QUADRATIC_C=-5.32213;

enum button_index{STDBY,UP,DOWN,AUTO,RESET};
