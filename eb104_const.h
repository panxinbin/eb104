#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

const int sensorPinFWD = A13;    // GIALLO
const int sensorPinREF = A12;    // BIANCO

const int MAX_FWD = 600;
const int MAX_REF = 100;
const float MAX_SWR = 5.0;
const int MAX_TMP = 100;
const float MAX_V = 52.0;
const float MAX_I = 26;
const int MINPRESSURE=30;
const int MAXPRESSURE=1000;

const float FWD_QUADRATIC_A=0.000249622;
const float FWD_QUADRATIC_B=0.0399433;
const float FWD_QUADRATIC_C=-5.32213;

//touchscreen dimensions
//const int TS_LEFT=909,TS_RT=155,TS_TOP=190,TS_BOT=860;
const int XP=8,XM=A2,YP=A3,YM=9; //320x480 ID=0x9486
const int TS_LEFT=957,TS_RT=93,TS_TOP=909,TS_BOT=124;



/*

10:57:21.459 -> const int XP=8,XM=A2,YP=A3,YM=9; //320x480 ID=0x9486
10:57:21.525 -> const int TS_LEFT=124,TS_RT=909,TS_TOP=957,TS_BOT=93;
10:57:21.657 -> PORTRAIT CALIBRATION     320 x 480
10:57:21.856 -> x = map(p.x, LEFT=124, RT=909, 0, 320)
10:57:22.022 -> y = map(p.y, TOP=957, BOT=93, 0, 480)
10:57:22.188 -> Touch Pin Wiring XP=8 XM=A2 YP=A3 YM=9
10:57:22.354 -> LANDSCAPE CALIBRATION    480 x 320
10:57:22.519 -> x = map(p.y, LEFT=957, RT=93, 0, 480)
10:57:22.685 -> y = map(p.x, TOP=909, BOT=124, 0, 320)


*/
