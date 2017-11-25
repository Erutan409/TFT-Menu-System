#include <Adafruit_TFTLCD.h> 
#include <TouchScreen.h>
#include <EEPROM.h>

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

// touch calibrated
#define TS_MINX 190
#define TS_MINY 190
#define TS_MAXX 920
#define TS_MAXY 920

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Color definitions - in 5:6:5
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

int i = 0;
int page = 0;
int sleep = 0;
int pulsev = 0;
int redflag = 0;
int greenflag = 0;

unsigned long sleeptime;

int prevpage;
int sleepnever;
int esleep;

int antpos = 278;
unsigned long awakeend;
unsigned long currenttime;
unsigned long ssitime;

void setup(void) {
  // pinMode(3, OUTPUT);

  esleep = EEPROM.read(1);
//esleep = 3; // uncomment this and run once if you have not used the EEPROM before on your Arduino! Comment and reload after that.
//blv = 255; // uncomment this and run once if you have not used the EEPROM before on your Arduino! Comment and reload after that.c
  if (esleep == 1) {
    sleeptime = 10000;
  }
  if (esleep == 2) {
    sleeptime = 20000;
  }
  if (esleep == 3) {
    sleeptime = 30000;
  }
  if (esleep == 4) {
    sleeptime = 60000;
  }
  if (esleep == 5) {
    sleeptime = 120000;
  }
  if (esleep == 6) {
    sleeptime = 300000;
  }
  if (esleep == 7) {
    sleeptime = 600000;
  }
  if (esleep == 8) {
    sleeptime = 1200000;
  }
  if (esleep == 9) {
    sleeptime = 1800000;
  }
  if (esleep == 10) {
    sleeptime = 3600000;
  }
  if (esleep == 11) {
    sleeptime = 14400000;
  }
  if (esleep == 12) {
    sleepnever = 1;
  }
  awakeend = sleeptime + 1000; // set the current sleep time based on what the saved settings in EEPROM were
  Serial.begin(9600);
  Serial.println("TFT Menu");
  
  tft.reset();
  uint16_t identifier = 0x9341; 
  tft.begin(identifier); 
  tft.fillScreen(BLACK);
  tft.setRotation(1);
  tft.fillRect(71, 70, 50, 100, CYAN);
  tft.fillRect(134, 70, 50, 100, CYAN);
  tft.fillRect(197, 70, 50, 100, CYAN);
  tft.drawRect(46, 45, 228, 150, WHITE);
  
  delay(250);
  tft.setCursor(85, 100);
  tft.setTextSize(5);
  tft.setTextColor(WHITE);
  tft.print("J");
  delay(250);
  tft.setCursor(147, 100);
  tft.print("O");
  delay(250);
  tft.setCursor(210, 100);
  tft.print("S");
  delay(500);
  tft.setCursor(84, 210);
  tft.setTextSize(1);
  tft.print("Jeremy Saglimbeni  -  2012");
  tft.setCursor(108, 230);
  tft.print("thecustomgeek.com");
  delay(500);
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, 320, 10, WHITE); // status bar
  drawhomeicon(); // draw the home icon
  tft.setCursor(1, 1);
  tft.print("Your status bar message here.    JOS 1.5 Beta");
  tft.drawRect(297, 1, 20, 8, WHITE); //battery body
  tft.fillRect(317, 3, 2, 4, WHITE); // battery tip
  tft.fillRect(298, 2, 18, 6, BLACK); // clear the center of the battery

  ant(); // draw the bas "antenna" line without the "signal waves"
  signal(); // draw the "signal waves" around the "antenna"
  homescr(); // draw the homescreen
  tft.drawRect(0, 200, 245, 40, WHITE); // message box
  pinMode(13, OUTPUT);
}
#define MINPRESSURE 10
#define MAXPRESSURE 1000
void loop() {
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  // if you're sharing pins, you'll need to fix the directions of the touchscreen pins!
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);
  currenttime = millis();
  unsigned long currentawake = millis();
  if((currentawake > awakeend) && (sleepnever == 0)) {
    if (sleep == 0) {
      sleep = 1;
    }
  }
 
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    awakeend = currenttime + sleeptime; //set the sleep time when screen is pressed
    if (sleep == 1) { // if asleep, then fade the backlight up
      sleep = 0; // change the sleep mode to "awake"
      return;
    }
    /*
    Serial.print("X = "); 
     Serial.print(p.x);
     Serial.print("\tY = "); 
     Serial.print(p.y);
     Serial.print("\tPressure = "); 
     Serial.println(p.z);
     */
    // turn from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    
    Serial.print("p.y:"); // this code will help you get the y and x numbers for the touchscreen
    Serial.print(p.y);
    Serial.print("   p.x:");
    Serial.println(p.x);

    // area 1
    if (p.y > 0 && p.y < 146 && p.x > 178 && p.x < 226) { // if this area is pressed
      if (page == 5) { // and if page 5 is drawn on the screen
        m5b1action(); // do whatever this button is
        tft.setTextColor(RED);
        tft.setTextSize(2);
        tft.setCursor(12, 213);
        tft.print("Menu 5 B1"); // display the command in the "message box"
        //yled(550); // flash the LED yellow for a bit - change the 550 value to change LED time on
        clearmessage(); // after the LED goes out, clear the message
      }
      if (page == 4) {
        m4b1action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B1");
        //yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b1action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B1");
        //yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b1action();
        tft.setCursor(12, 213);
        tft.print("Menu 2 B1");
        //yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b1action();
        tft.setCursor(12, 213);
        tft.print("Menu 1 B1");
        //yled(550);
        clearmessage();
      }
      if (page == 0) { // if you are on the "home" page (0)
        page = 1; // then you just went to the first page
        redraw(); // redraw the screen with the page value 1, giving you the page 1 menu
      }
    }
    // area 2
    if (p.y > 168 && p.y < 320 && p.x > 180 && p.x < 226) {
      if (page == 5) {
        m5b2action();
        tft.setCursor(12, 213);
        tft.print("Menu 5 B2");
        //yled(550);
        clearmessage();
      }
      if (page == 4) {
        m4b2action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B2");
        //yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b2action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B2");
        //yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b2action();
        tft.setCursor(12, 213);
        tft.print("Menu 2 B2");
        //yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b2action();
        tft.setCursor(12, 213);
        tft.print("Menu 1 B2");
        //yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 2;
        redraw();
      }
    }
    // area 3
    if (p.y > 0 && p.y < 146 && p.x > 120 && p.x < 168) {
      if (page == 5) {
        m5b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 5 B3");
        //yled(550);
        clearmessage();
      }
      if (page == 4) {
        m4b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B3");
        //yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B3");
        //yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 2 B3");
        //yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 1 B3");
        //yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 3;
        redraw();
      }
    }
    // area 4
    if (p.y > 167 && p.y < 320 && p.x > 120 && p.x < 168) {
      if (page == 5) {
        m5b4action();
        tft.setCursor(12, 213);
        tft.print("Menu 5 B4");
        //yled(550);
        clearmessage();
      }
      if (page == 4) {
        m4b4action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B4");
        //yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b4action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B4");
        //yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b4action();
        tft.setCursor(12, 213);
        tft.print("Menu 2 B4");
        //yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b4action();
        tft.setCursor(12, 213);
        tft.print("Menu 1 B4");
        //yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 4;
        redraw();
      }
    }
    // area 5
    if (p.y > 0 && p.y < 146 && p.x > 54 && p.x < 104) {
      if (page == 5) {
        m5b5action();
        tft.setCursor(12, 213);
        tft.print("Menu 5 B5");
        //yled(550);
        clearmessage();
      }
      if (page == 4) {
        m4b5action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B5");
        //yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b5action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B5");
        //yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b5action();
        tft.setCursor(12, 213);
        tft.print("Menu 2 B5");
        //yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b5action();
        tft.setCursor(12, 213);
        tft.print("Menu 1 B5");
        //yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 5;
        redraw();
      }
    }
    // area 6
    if (p.y > 168 && p.y < 320 && p.x > 54 && p.x < 104) {
      if (page == 5) {
        m5b6action();
        tft.setCursor(12, 213);
        tft.print("Menu 5 B6");
        //yled(550);
        clearmessage();
      }
      if (page == 4) {
        m4b6action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B6");
        //yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b6action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B6");
        //yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b6action();
        tft.setCursor(12, 213);
        tft.print("Menu 2 B6");
        //yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b6action();
        tft.setCursor(12, 213);
        tft.print("Menu 1 B6");
        //yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 6;
        redraw();
      }
    }
    // home
    if (p.y > 280 && p.y < 340 && p.x > 0 && p.x < 48) { // if the home icon is pressed
      if (page == 6) { // if you are leaving the settings page
        clearmessage(); // clear the battery voltage out of the message box
        tft.setTextSize(2);
        tft.setTextColor(YELLOW);
        tft.setCursor(12, 213);
        tft.print("Settings Saved"); // display settings saved in message box
        EEPROM.write(1, esleep); // write the sleep value to EEPROM, so it will not lose settings without power

        clearsettings(); // erase all the drawings on the settings page
      }
      if (page == 0) { // if you are already on the home page
        drawhomeiconred(); // draw the home icon red
        delay(250); // wait a bit
        drawhomeicon(); // draw the home icon back to white
        return; // if you were on the home page, stop.
      }
      else { // if you are not on the settings, home, or keyboard page
        page = prevpage; // a value to keep track of what WAS on the screen to redraw/erase only what needs to be
        page = 0; // make the current page home
        redraw(); // redraw the page
      }
    }
    // message area
    if (p.y > 0 && p.y < 246 && p.x > 4 && p.x < 44) {
      clearmessage(); // erase the message
    }
    // sleep buttons
    if (p.y > 0 && p.y < 56 && p.x > 116 && p.x < 166) {
      if (page == 6) {
        sleepdec();
      }
    }
    if (p.y > 260 && p.y < 320 && p.x > 116 && p.x < 166) {
      if (page == 6) {
        sleepinc();
      }
    }
    /*
    // optional buttons
     if (p.y > 3 && p.y < 66 && p.x > 72 && p.x < 126) {
     if (page == 6) {
     option3down();
     }
     }
     if (p.y > 269 && p.y < 324 && p.x > 72 && p.x < 126) {
     if (page == 6) {
     option3up();
     }
     }
     */
  }

}

void redraw() { // redraw the page
  if ((prevpage != 6) || (page !=7)) {
    clearcenter();
  }
  if (page == 0) {
    homescr();
  }
  if (page == 1) {
    menu1();
  }
  if (page == 2) {
    menu2();
  }
  if (page == 3) {
    menu3();
  }
  if (page == 4) {
    menu4();
  }
  if (page == 5) {
    menu5();
  }
  if (page == 6) {
    settingsscr();
  }
}
void clearcenter() { // the reason for so many small "boxes" is that it's faster than filling the whole thing
  tft.drawRect(0, 20, 150, 50, BLACK);
  tft.drawRect(170, 20, 150, 50, BLACK);
  tft.drawRect(0, 80, 150, 50, BLACK);
  tft.drawRect(170, 80, 150, 50, BLACK);
  tft.drawRect(0, 140, 150, 50, BLACK);
  tft.drawRect(170, 140, 150, 50, BLACK);
  tft.fillRect(22, 37, 106, 16, BLACK);
  tft.fillRect(192, 37, 106, 16, BLACK);
  tft.fillRect(22, 97, 106, 16, BLACK);
  tft.fillRect(192, 97, 106, 16, BLACK);
  tft.fillRect(22, 157, 106, 16, BLACK);
  tft.fillRect(192, 157, 106, 16, BLACK);
}
void clearsettings() { // this is used to erase the extra drawings when exiting the settings page
  tft.fillRect(0, 20, 320, 110, BLACK);
  delay(500);
  clearmessage();
}
void homescr() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(41, 37);
  tft.print("Menu 1");
  tft.setCursor(210, 37);
  tft.print("Menu 2");
  tft.setCursor(41, 97);
  tft.print("Menu 3");
  tft.setCursor(210, 97);
  tft.print("Menu 4");
  tft.setCursor(41, 157);
  tft.print("Menu 5");
  tft.setCursor(200, 157);
  tft.print("Settings");
}
void menu1() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Menu 1 B1");
  tft.setCursor(192, 37);
  tft.print("Menu 1 B2");
  tft.setCursor(22, 97);
  tft.print("Menu 1 B3");
  tft.setCursor(192, 97);
  tft.print("Menu 1 B4");
  tft.setCursor(22, 157);
  tft.print("Menu 1 B5");
  tft.setCursor(192, 157);
  tft.print("Menu 1 B6");
}
void menu2() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Menu 2 B1");
  tft.setCursor(192, 37);
  tft.print("Menu 2 B2");
  tft.setCursor(22, 97);
  tft.print("Menu 2 B3");
  tft.setCursor(192, 97);
  tft.print("Menu 2 B4");
  tft.setCursor(22, 157);
  tft.print("Menu 2 B5");
  tft.setCursor(192, 157);
  tft.print("Menu 2 B6");
}
void menu3() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Menu 3 B1");
  tft.setCursor(192, 37);
  tft.print("Menu 3 B2");
  tft.setCursor(22, 97);
  tft.print("Menu 3 B3");
  tft.setCursor(192, 97);
  tft.print("Menu 3 B4");
  tft.setCursor(22, 157);
  tft.print("Menu 3 B5");
  tft.setCursor(192, 157);
  tft.print("Menu 3 B6");
}
void menu4() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Menu 4 B1");
  tft.setCursor(192, 37);
  tft.print("Menu 4 B2");
  tft.setCursor(22, 97);
  tft.print("Menu 4 B3");
  tft.setCursor(192, 97);
  tft.print("Menu 4 B4");
  tft.setCursor(22, 157);
  tft.print("Menu 4 B5");
  tft.setCursor(192, 157);
  tft.print("Menu 4 B6");
}
void menu5() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Menu 5 B1");
  tft.setCursor(192, 37);
  tft.print("Menu 5 B2");
  tft.setCursor(22, 97);
  tft.print("Menu 5 B3");
  tft.setCursor(192, 97);
  tft.print("Menu 5 B4");
  tft.setCursor(22, 157);
  tft.print("Menu 5 B5");
  tft.setCursor(192, 157);
  tft.print("Menu 5 B6");
}
void settingsscr() {
  // backlight level
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.fillRect(0, 20, 60, 50, RED);
  tft.drawRect(0, 20, 60, 50, WHITE);
  tft.drawRect(80, 20, 160, 50, WHITE);
  tft.fillRect(260, 20, 60, 50, GREEN);
  tft.drawRect(260, 20, 60, 50, WHITE);
  tft.setCursor(22, 33);
  tft.print("-");
  tft.setCursor(282, 33);
  tft.print("+");
  tft.setTextSize(1);
  tft.setCursor(120, 31);
  tft.print("Backlight Level");
  tft.drawRect(110, 48, 100, 10, WHITE);
  // sleep time
  tft.setTextSize(3);
  tft.fillRect(0, 80, 60, 50, RED);
  tft.drawRect(0, 80, 60, 50, WHITE);
  tft.drawRect(80, 80, 160, 50, WHITE);
  tft.fillRect(260, 80, 60, 50, GREEN);
  tft.drawRect(260, 80, 60, 50, WHITE);
  tft.setCursor(22, 93);
  tft.print("-");
  tft.setCursor(282, 93);
  tft.print("+");
  tft.setTextSize(1);
  tft.setCursor(130, 91);
  tft.print("Sleep Time");
  showsleep();
  //?? uncomment this if you want a third adjustable option
  /*
  tft.fillRect(0, 140, 60, 50, RED);
   tft.drawRect(0, 140, 60, 50, WHITE);
   tft.drawRect(80, 140, 160, 50, WHITE);
   tft.fillRect(260, 140, 60, 50, GREEN);
   tft.drawRect(260, 140, 60, 50, WHITE);
   tft.print(22, 153, "-", WHITE, 3);
   tft.print(130, 151, "Thing #3", WHITE);
   tft.print(282, 153, "+", WHITE, 3);
   tft.drawRect(110, 168, 100, 10, WHITE);
   */
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.setCursor(12, 213);
  tft.print(1234);
  tft.setCursor(60, 213);
  tft.print("mV");
  /*
  battpercent = (battv / 5000) * 100, 2;
  itoa (battpercent, battpercenttxt, 10);
  tft.print(102, 213, battpercenttxt, YELLOW, 2);
  */
}
void sleepinc() { // sleep increese adjustment
  if (sleeptime == 14400000) {
    sleepnever = 1;
    esleep = 12;
    sleeptime = 11111111;
    showsleep();
  }
  if (sleeptime == 3600000) {
    sleeptime = 14400000;
    esleep = 11;
    showsleep();
  }
  if (sleeptime == 1800000) {
    sleeptime = 3600000;
    esleep = 10;
    showsleep();
  }
  if (sleeptime == 1200000) {
    sleeptime = 1800000;
    esleep = 9;
    showsleep();
  }
  if (sleeptime == 600000) {
    sleeptime = 1200000;
    esleep = 8;
    showsleep();
  }
  if (sleeptime == 300000) {
    sleeptime = 600000;
    esleep = 7;
    showsleep();
  }
  if (sleeptime == 120000) {
    sleeptime = 300000;
    esleep = 6;
    showsleep();
  }
  if (sleeptime == 60000) {
    sleeptime = 120000;
    esleep = 5;
    showsleep();
  }
  if (sleeptime == 30000) {
    sleeptime = 60000;
    esleep = 4;
    showsleep();
  }
  if (sleeptime == 20000) {
    sleeptime = 30000;
    esleep = 3;
    showsleep();
  }
  if (sleeptime == 10000) {
    sleeptime = 20000;
    esleep = 2;
    showsleep();
  }
  delay(350);
}
void sleepdec() { // sleep decreese adjustment
  if (sleeptime == 20000) {
    sleeptime = 10000;
    esleep = 1;
    showsleep();
  }
  if (sleeptime == 30000) {
    sleeptime = 20000;
    esleep = 2;
    showsleep();
  }
  if (sleeptime == 60000) {
    sleeptime = 30000;
    esleep = 3;
    showsleep();
  }
  if (sleeptime == 120000) {
    sleeptime = 60000;
    esleep = 4;
    showsleep();
  }
  if (sleeptime == 300000) {
    sleeptime = 120000;
    esleep = 5;
    showsleep();
  }
  if (sleeptime == 600000) {
    sleeptime = 300000;
    esleep = 6;
    showsleep();
  }
  if (sleeptime == 1200000) {
    sleeptime = 600000;
    esleep = 7;
    showsleep();
  }
  if (sleeptime == 1800000) {
    sleeptime = 1200000;
    esleep = 8;
    showsleep();
  }
  if (sleeptime == 3600000) {
    sleeptime = 1800000;
    esleep = 9;
    showsleep();
  }
  if (sleeptime == 14400000) {
    sleeptime = 3600000;
    esleep = 10;
    showsleep();
  }
  if (sleepnever == 1) {
    sleeptime = 14400000;
    sleepnever = 0;
    esleep = 11;
    showsleep();
  }
  delay(350);
}
void showsleep() { // shows the sleep time on the settings page
  tft.fillRect(110, 108, 80, 10, BLACK);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  if (sleeptime == 10000) {
    tft.setCursor(130, 108);
    tft.print("10 Seconds");
  }
  if (sleeptime == 20000) {
    tft.setCursor(130, 108);
    tft.print("20 Seconds");
  }
  if (sleeptime == 30000) {
    tft.setCursor(130, 108);
    tft.print("30 Seconds");
  }
  if (sleeptime == 60000) {
    tft.setCursor(136, 108);
    tft.print("1 Minute");
  }
  if (sleeptime == 120000) {
    tft.setCursor(133, 108);
    tft.print("2 Minutes");
  }
  if (sleeptime == 300000) {
    tft.setCursor(133, 108);
    tft.print("5 Minutes");
  }
  if (sleeptime == 600000) {
    tft.setCursor(130, 108);
    tft.print("10 Minutes");
  }
  if (sleeptime == 1200000) {
    tft.setCursor(130, 108);
    tft.print("20 Minutes");
  }
  if (sleeptime == 1800000) {
    tft.setCursor(130, 108);
    tft.print("30 Minutes");
  }
  if (sleeptime == 3600000) {
    tft.setCursor(142, 108);
    tft.print("1 Hour");
  }
  if (sleeptime == 14400000) {
    tft.setCursor(139, 108);
    tft.print("4 Hours");
  }
  if (sleepnever == 1) {
    tft.setCursor(133, 108);
    tft.print("Always On");
  }
}
void option3down() { // adjust option 3 down in the settings screen
}
void option3up() { // adjust option 3 up in the settings screen
}
//custom defined actions - this is where you put your button functions
void m1b1action() {
  signal();
}
void m1b2action() {
  signalact();
}
void m1b3action() {
}
void m1b4action() {
}
void m1b5action() {
}
void m1b6action() {
}
void m2b1action() {
}
void m2b2action() {
}
void m2b3action() {
}
void m2b4action() {
}
void m2b5action() {
}
void m2b6action() {
}
void m3b1action() {
}
void m3b2action() {
}
void m3b3action() {
}
void m3b4action() {
}
void m3b5action() {
}
void m3b6action() {
}
void m4b1action() {
}
void m4b2action() {
}
void m4b3action() {
}
void m4b4action() {
}
void m4b5action() {
}
void m4b6action() {
}
void m5b1action() {
}
void m5b2action() {
}
void m5b3action() {
}
void m5b4action() {
}
void m5b5action() {
}
void m5b6action() {
}

void ant() {
  tft.fillRect((antpos + 5), 4, 1, 6, WHITE); // draws the "antenna" for the signal indicator
}
void boxes() { // redraw the button outline boxes
  tft.drawRect(0, 20, 150, 50, CYAN);
  tft.drawRect(170, 20, 150, 50, WHITE);
  tft.drawRect(0, 80, 150, 50, WHITE);
  tft.drawRect(170, 80, 150, 50, WHITE);
  tft.drawRect(0, 140, 150, 50, WHITE);
  tft.drawRect(170, 140, 150, 50, WHITE);
}
void signal() { // draws a whit 'signal indicator'
  tft.drawLine((antpos + 4), 4, (antpos + 4), 5, WHITE);
  tft.drawPixel((antpos + 3), 2, WHITE);
  tft.drawPixel((antpos + 3), 7, WHITE);
  tft.drawPixel((antpos + 2), 0, WHITE);
  tft.drawLine((antpos + 2), 3, (antpos + 2), 6, WHITE);
  tft.drawPixel((antpos + 2), 9, WHITE);
  tft.drawPixel((antpos + 1), 1, WHITE);
  tft.drawPixel((antpos + 1), 8, WHITE);
  tft.drawLine(antpos, 2, antpos, 7, WHITE);
  tft.drawLine((antpos + 6), 4, (antpos + 6), 5, WHITE);
  tft.drawPixel((antpos + 7), 2, WHITE);
  tft.drawPixel((antpos + 7), 7, WHITE);
  tft.drawPixel((antpos + 8), 0, WHITE);
  tft.drawLine((antpos + 8), 3, (antpos + 8), 6, WHITE);
  tft.drawPixel((antpos + 8), 9, WHITE);
  tft.drawPixel((antpos + 9), 1, WHITE);
  tft.drawPixel((antpos + 9), 8, WHITE);
  tft.drawLine((antpos + 10), 2, (antpos + 10), 7, WHITE);
}
void signalact() { // draws a red'signal indicator'
  tft.drawLine((antpos + 4), 4, (antpos + 4), 5, RED);
  tft.drawPixel((antpos + 3), 2, RED);
  tft.drawPixel((antpos + 3), 7, RED);
  tft.drawPixel((antpos + 2), 0, RED);
  tft.drawLine((antpos + 2), 3, (antpos + 2), 6, RED);
  tft.drawPixel((antpos + 2), 9, RED);
  tft.drawPixel((antpos + 1), 1, RED);
  tft.drawPixel((antpos + 1), 8, RED);
  tft.drawLine(antpos, 2, antpos, 7, RED);
  tft.drawLine((antpos + 6), 4, (antpos + 6), 5, RED);
  tft.drawPixel((antpos + 7), 2, RED);
  tft.drawPixel((antpos + 7), 7, RED);
  tft.drawPixel((antpos + 8), 0, RED);
  tft.drawLine((antpos + 8), 3, (antpos + 8), 6, RED);
  tft.drawPixel((antpos + 8), 9, RED);
  tft.drawPixel((antpos + 9), 1, RED);
  tft.drawPixel((antpos + 9), 8, RED);
  tft.drawLine((antpos + 10), 2, (antpos + 10), 7, RED);
}
void drawhomeicon() { // draws a white home icon
  tft.drawLine(280, 219, 299, 200, WHITE);
  tft.drawLine(300, 200, 304, 204, WHITE);
  tft.drawLine(304, 203, 304, 200, WHITE);
  tft.drawLine(305, 200, 307, 200, WHITE);
  tft.drawLine(308, 200, 308, 208, WHITE);
  tft.drawLine(309, 209, 319, 219, WHITE);
  tft.drawLine(281, 219, 283, 219, WHITE);
  tft.drawLine(316, 219, 318, 219, WHITE);
  tft.drawRect(284, 219, 32, 21, WHITE);
  tft.drawRect(295, 225, 10, 15, WHITE);
}
void drawhomeiconred() { // draws a red home icon
  tft.drawLine(280, 219, 299, 200, RED);
  tft.drawLine(300, 200, 304, 204, RED);
  tft.drawLine(304, 203, 304, 200, RED);
  tft.drawLine(305, 200, 307, 200, RED);
  tft.drawLine(308, 200, 308, 208, RED);
  tft.drawLine(309, 209, 319, 219, RED);
  tft.drawLine(281, 219, 283, 219, RED);
  tft.drawLine(316, 219, 318, 219, RED);
  tft.drawRect(284, 219, 32, 21, RED);
  tft.drawRect(295, 225, 10, 15, RED);
}
void clearmessage() {
  tft.fillRect(12, 213, 226, 16, BLACK); // black out the inside of the message box
}
