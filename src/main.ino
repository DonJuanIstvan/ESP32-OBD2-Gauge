/*

 Example sketch for TFT_eSPI library.

 No fonts are needed.
 
 Draws a 3d rotating cube on the TFT screen.
 
 Original code was found at http://forum.freetronics.com/viewtopic.php?f=37&t=5495
 
 */

bool debug;
uint16_t randomChangeInterval = 0;
// int randomInt = 1;
int previousNumber = 1;
int currentNumber = 1;

#include <Wire.h>
#include "FT62XXTouchScreen.h"
#include "BluetoothSerial.h"

//
// Define these in User_Setup.h in the TFT_eSPI
//
#define ST7796_DRIVER 1
#define TFT_WIDTH  480
#define TFT_HEIGHT 320

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320

#define USE_HSPI_PORT 1
#define PIN_SDA 18
#define PIN_SCL 19
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   21
#define TFT_RST  22
#define TFT_BL   23

//#define TOUCH_CS PIN_D2     // Chip select pin (T_CS) of touch screen

#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);
BluetoothSerial SerialBT;
BluetoothSerial SerialBT2;

int16_t h;
int16_t w;

#define ELM_PORT   SerialBT
#define ESP_PORT   SerialBT2
#define DEBUG_PORT Serial


//OBD variables
byte inData;
char inChar;
String BuildINString = "";
String DisplayString = "";
String DisplayValue = "";
String SentMessage = "";
int ByteCount = 0;
float A;
float ASecond;
float B;
float BSecond;
int WorkingVal;
String WorkingString = "";

uint16_t  ENGINE_RPM;                       //12   C    2
uint8_t   ENGINE_LOAD;                      //4    4    1
uint8_t   RELATIVE_ACCELERATOR_PEDAL_POS;   //90   5A   1
int16_t   ENGINE_COOLANT_TEMP;              //5    5    1
uint16_t  ENGINE_OIL_TEMP;                  //92   5C   1
uint8_t   THROTTLE_POSITION;                //69   11   1
float     CONTROL_MODULE_VOLTAGE;           //66   42   2
int16_t   INTAKE_AIR_TEMP;                  //15   F    1


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  RgbColor(255, 60, 30)
#define DARKORANGE  RgbColor(117, 24, 13)
#define DARKGREY RgbColor(200, 200, 200)
#define LIGHTGREY RgbColor(100, 100, 100)
#define DARKRED RgbColor(100, 0, 0)

//MINI Gauge Cluster Colors
#define WaringYellow RgbColor(255, 150, 0) //255, 202, 1
#define NumberColor RgbColor(238, 64, 18) 
#define NumberColorContrast RgbColor(177, 52, 27)



#define Border        LIGHTGREY//DARKORANGE
#define Text          NumberColor
#define ContrastText  DARKGREY
#define WarningText   WaringYellow


int RgbColor(int R, int G, int B)
{
  return int(R/8*2048) + int(G/4*32)+ int(B/8);
}



void setup() {
  DEBUG_PORT.begin(115200);
  ELM_PORT.setPin("1234");
  ELM_PORT.begin("ArduHUD", true);
  tft.init();
  delay(2000);
  DEBUG_PORT.println("tft init");

  // Backlight hack...
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, 128);

  touchScreen.begin();

  h = tft.height();
  w = tft.width();
  tft.setRotation(1);

  centeredText("Start Demo Mode?",4,Text);
  waitForTouch();

  if(debug == false)
  {
    centeredText("Searching for OBDII",4,Text);
    DEBUG_PORT.println("searching for OBDII");

    if (!ELM_PORT.connect("OBDII"))
    {
      DEBUG_PORT.println("Couldn't connect to OBD scanner");
      centeredText("OBDII not found",4,RED);
      while(1);
    }
    centeredText("OBDII found", 4, GREEN);
    DEBUG_PORT.println("OBDII found");
    DEBUG_PORT.println("ATZ");
    ELM_PORT.println("ATZ");
    delay(1000);
    
    while (ELM_PORT.available() > 0)
    {
      inData = 0;
      inChar = 0;
      inData = ELM_PORT.read();
      //DEBUG_PORT.println(inData);
      inChar = char(inData);
      BuildINString = BuildINString + inChar;
      //DEBUG_PORT.println(BuildINString);
    }
    DEBUG_PORT.println(BuildINString.substring(0, 3));

    if (BuildINString.substring(0, 3) != "ATZ")
    {
      DEBUG_PORT.println("Couldn't connect to OBD scanner");
      centeredText("ELM not Responding", 4, RED);
      while(1);
    }
    else
    {
      DEBUG_PORT.println("Connected to ELM327");
      centeredText("Connection OK", 6, GREEN);
      delay(3000);
    }
  }  
  tft.fillScreen(BLACK);

  DrawGrid();
  SetDataStrings();

  if (debug == true)
  {
    ENGINE_RPM = 0;                       //12   C
    ENGINE_LOAD = 63;                      //4    4
    RELATIVE_ACCELERATOR_PEDAL_POS = 72;   //90   5A
    ENGINE_COOLANT_TEMP = 85;              //5    5
    ENGINE_OIL_TEMP = 98;                  //92   5C
    THROTTLE_POSITION = 31;                //69   11
    CONTROL_MODULE_VOLTAGE = 9.8;           //66   42
    INTAKE_AIR_TEMP = 30;
  }
}

/***********************************************************************************************************************************/
void loop() {

  TouchPoint touchPos = touchScreen.read();

  if (touchPos.touched) {
    DEBUG_PORT.printf("X: %d, Y: %d\n", touchPos.xPos, touchPos.yPos);
    tft.drawCircle(touchPos.xPos, touchPos.yPos, 10, TFT_SILVER);
  }
  if(debug == false)
  {
    ENGINE_RPM = 0;                       //12   C
    ENGINE_LOAD = 0;                      //4    4
    RELATIVE_ACCELERATOR_PEDAL_POS = 0;   //90   5A
    ENGINE_COOLANT_TEMP = 0;              //5    5
    ENGINE_OIL_TEMP = 0;                  //92   5C
    THROTTLE_POSITION = 0;                //69   11
    CONTROL_MODULE_VOLTAGE = 0;           //66   42
    INTAKE_AIR_TEMP = 0;

    int verzoegerung = 200;

    SentMessage = "01 04 05";
    DEBUG_PORT.println(SentMessage);
    ELM_PORT.println(SentMessage);
    delay(verzoegerung);
    ReadDataNew();
    SentMessage = "01 0F 11";
    DEBUG_PORT.println(SentMessage);
    ELM_PORT.println(SentMessage);
    delay(verzoegerung);
    ReadDataNew();
    SentMessage = "01 0C 42";
    DEBUG_PORT.println(SentMessage);
    ELM_PORT.println(SentMessage);
    delay(verzoegerung);
    ReadDataNew();
  }
  else
  {    
    static unsigned long lastTrigger = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastTrigger >= 5000) {
      previousNumber = currentNumber;
      while (currentNumber == previousNumber) {
        currentNumber = random(1, 6); // Generate a random number between 1 and 6
      }        
      DEBUG_PORT.println(currentNumber);      
      lastTrigger = currentMillis; // Reset the timer
    }

    if (currentNumber*1000 > ENGINE_RPM)
    {
      ENGINE_RPM = ENGINE_RPM + 7;
    }
    else
    {
      ENGINE_RPM = ENGINE_RPM - 9;  
    }   
                         
    // ENGINE_LOAD = 63;                      //4    4
    // RELATIVE_ACCELERATOR_PEDAL_POS = 72;   //90   5A
    // ENGINE_COOLANT_TEMP = 85;              //5    5
    // ENGINE_OIL_TEMP = 98;                  //92   5C
    // THROTTLE_POSITION = 31;                //69   11
    // CONTROL_MODULE_VOLTAGE = 9.8;           //66   42
    // INTAKE_AIR_TEMP = 30;
  }

  tft.textcolor = Text;
  tft.textsize = 7;

  //Engine Oil Temp
  //if(ENGINE_OIL_TEMP > 89)
  //{
  //  mylcd.Set_Text_colour(WarningText);
  //}

  tft.drawString(fixedLengthInt(ENGINE_OIL_TEMP,5,' '),70, 183);
  tft.textcolor = Text;

  //Engine Coolant Temp
  //if(ENGINE_COOLANT_TEMP > 89)
  //{
  //  mylcd.Set_Text_colour(WarningText);
  //}

  tft.drawString(fixedLengthInt(ENGINE_COOLANT_TEMP,5,' '),310, 183);
  tft.textcolor = Text;

  //Accelerator Pedal Position
  if (RELATIVE_ACCELERATOR_PEDAL_POS > 89)
  {
    tft.textcolor = WarningText;
  }

  tft.drawString(fixedLengthInt(RELATIVE_ACCELERATOR_PEDAL_POS,5,' '),70, 103);
  tft.textcolor = Text;

  //Throttle Position
  if (THROTTLE_POSITION > 89)
  {
    tft.textcolor = WarningText;
  }

  tft.drawString(fixedLengthInt(THROTTLE_POSITION,5,' '),310, 103);
  tft.textcolor = Text;

  //Engine Load
  if (ENGINE_LOAD > 89)
  {
    tft.textcolor = WarningText;
  }

  tft.drawString(fixedLengthInt(ENGINE_LOAD,5,' '),310, 23);
  tft.textcolor = Text;

  //Volatege
  if (CONTROL_MODULE_VOLTAGE < 11.5)
  {
      tft.textcolor = WarningText;
  }


  tft.drawString(fixedLengthFloat(CONTROL_MODULE_VOLTAGE, 1,'.', 4, ' '), 70, 263);
  tft.textcolor = Text;

  //Intake Air Temp
  //if(INTAKE_AIR_TEMP > 89)
  //{
  //  mylcd.Set_Text_colour(WarningText);
  //}

  tft.drawString(fixedLengthInt(INTAKE_AIR_TEMP,5,' '),310, 263);
  tft.textcolor = Text;

  //RPM

  if (ENGINE_RPM > 5999)
  {
    tft.textcolor = WarningText;
  }

  tft.drawString(fixedLengthInt(ENGINE_RPM,5,' '),70, 23);
  tft.textcolor = Text;
  //for (ENGINE_RPM = 0; ENGINE_RPM <= 6500; ENGINE_RPM = ENGINE_RPM + 100) {


  //  delay(50);
  //}
  //Serial.println(CONTROL_MODULE_VOLTAGE);
  
}

void ReadDataNew()
{
  BuildINString = "";
  //Serial.println(bluetoothSerial.available());
  while (ELM_PORT.available() > 0)
  {
    inData = 0;
    inChar = 0;
    inData = ELM_PORT.read();
    inChar = char(inData);
    //Serial.println(inData);
    //Serial.println(inChar);
    BuildINString = BuildINString + inChar;
  }
  Serial.println(BuildINString);
  //Serial.println(BuildINString.length());
  //Serial.println("Checking Length");
  if (BuildINString.length() == 27)
  {
    BuildINString = BuildINString.substring(12, 23);
    //Serial.println(BuildINString);
    if (BuildINString.startsWith("04"))
    {
      WorkingString = BuildINString.substring(3, 5);
      A = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("A: " + WorkingString);

      WorkingString = BuildINString.substring(9, 11);
      ASecond = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("A2: " + WorkingString);
    }
    if (BuildINString.startsWith("0F"))
    {
      WorkingString = BuildINString.substring(3, 5);
      A = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("A: " + WorkingString);

      WorkingString = BuildINString.substring(9, 11);
      ASecond = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("A2: " + WorkingString);
    }
  }
  if (BuildINString.length() == 33)
  {
    BuildINString = BuildINString.substring(12, 29);
    //Serial.println(BuildINString);
    if (BuildINString.startsWith("0C"))
    {
      WorkingString = BuildINString.substring(3, 5);
      A = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("A: " + WorkingString);

      WorkingString = BuildINString.substring(6, 8);
      B = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("B: " + WorkingString);

      WorkingString = BuildINString.substring(12, 14);
      ASecond = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("A2: " + WorkingString);

      WorkingString = BuildINString.substring(15, 17);
      BSecond = strtol(WorkingString.c_str(), NULL, 16);
      //Serial.println("B2: " + WorkingString);
    }
  }
  //RPM
  if (SentMessage.indexOf("0C") > 0)
  {
    //DisplayValue = ((A * 256)+B)/4;
    ENGINE_RPM = ((A * 256) + B) / 4;
  }

  //Coolant Temp
  if (SentMessage.indexOf("05") > 0)
  {
    //DisplayValue = A-40;
    ENGINE_COOLANT_TEMP = ASecond - 40;
  }

  //Intake Air Temp
  if (SentMessage.indexOf("0F") > 0)
  {
    //DisplayValue = A-40;
    INTAKE_AIR_TEMP = A - 40;
  }

  //Throttle position
  if (SentMessage.indexOf("11") > 0)
  {
    //DisplayValue = A*100/255;
    THROTTLE_POSITION = ASecond * 100 / 255;
  }

  //Engine Load
  if (SentMessage.indexOf("04") > 0)
  {
    //DisplayValue = A*100/255;
    ENGINE_LOAD = A * 100 / 255;
  }

  //Relative Acc Pedal Position
  if (SentMessage.indexOf("5A") > 0)
  {
    //DisplayValue = A*100/255;
    RELATIVE_ACCELERATOR_PEDAL_POS = A * 100 / 255;
  }

  //Engine Oil Temp
  if (SentMessage.indexOf("5C") > 0)
  {
    //DisplayValue = A-40;
    ENGINE_OIL_TEMP = A - 40;
  }

  //Control Modul Volatge
  if (SentMessage.indexOf("42") > 0)
  {
    //DisplayValue = ((A * 256)+B)/1000;
    CONTROL_MODULE_VOLTAGE = ((ASecond * 256) + BSecond) / 1000;
  }  
}

void centeredText(String text, int textSize, uint32_t textColor)
{
  tft.fillScreen(BLACK);
  tft.textcolor = textColor;
  tft.textsize = textSize;
  int textLength = text.length();
  int textWidthPx = textLength*(5*textSize)+(textLength-1)*textSize;
  int textHeightPx = textSize*7;
  int xpos;
  int ypos;
  if(tft.getRotation()%2 == 1)
  {
    xpos = (h-textWidthPx)/2;
    ypos = (w-textHeightPx)/2;
  }
  else
  {
    xpos = (w-textWidthPx)/2;
    ypos = (h-textHeightPx)/2;
  } 

  DEBUG_PORT.println(textWidthPx);
  DEBUG_PORT.println(textHeightPx);
  DEBUG_PORT.println(xpos);
  DEBUG_PORT.println(ypos);
  tft.drawString(text,xpos,ypos);
}

void DrawGrid()
{

  tft.drawFastVLine(239, 0, 340, Border);
  tft.drawFastVLine(240, 0, 320, Border);

  tft.drawFastHLine(0, 79, 480, Border);
  tft.drawFastHLine(0, 80, 480, Border);

  tft.drawFastHLine(0, 159, 480, Border);
  tft.drawFastHLine(0, 160, 480, Border);

  tft.drawFastHLine(0, 239, 480, Border);
  tft.drawFastHLine(0, 240, 480, Border);
}


void SetDataStrings()
{
  int x = 3;
  int y = 3;
  tft.textcolor = ContrastText;
  //mylcd.Set_Text_Back_colour(BLACK);
  tft.textsize = 2;

  //mylcd.Set_Draw_color (ContrastText);

  tft.drawString("RPM", 2+x, 2+y);

  tft.drawString("Engine Load (%)", 242+x, 2+y);

  tft.drawString("Acc. Pedal Pos (%)", 2+x, 82+y);

  tft.drawString("Throttle Pos. (%)", 242+x, 82+y);

  tft.drawString("Oil Temp ( C)", 2+x, 162+y);
  tft.drawRect(124+x, 163+y, 3, 3, ContrastText);
  tft.drawRect(123+x, 162+y, 5, 5, ContrastText);

  tft.drawString("Coolant Temp ( C)", 242+x, 162+y);
  tft.drawRect(412+x, 163+y, 3, 3, ContrastText);
  tft.drawRect(411+x, 162+y, 5, 5, ContrastText);

  tft.drawString("Battery Voltage (V)", 2+x, 242+y); //https://stackoverflow.com/questions/54568402/how-to-get-the-battery-voltage-of-a-car-using-obd-2

  tft.drawString("Intake Air Temp ( C)", 241+x, 242+y);
  tft.drawRect(447+x, 243+y, 3, 3, ContrastText);
  tft.drawRect(446+x, 242+y, 5, 5, ContrastText);


  //To set boarder over text
  //mylcd.Set_Draw_color (Border);
  //mylcd.Draw_Rectangle(0, 0, 479, 319);
}

String fixedLengthInt(uint16_t num, int length, char filler)
{
  int16_t system = 10; 
	uint8_t st[27] = {0};
	uint8_t *p = st+26;
	boolean flag = false;
	int16_t len = 0,nlen = 0,left_len = 0,i = 0;
	*p = '\0';
	if(0 == num)
	{
		*(--p) = '0';
		len = 1;
	}
	else
	{
		if(num < 0)
		{
			num = -num;
			flag = true;
		}		
	}
	while((num > 0) && (len < 10))
	{
		if(num%system > 9)
		{
			*(--p) = 'A' + (num%system-10);
		}
		else
		{
			*(--p) = '0' + num%system;
		}
		num = num/system;
		len++;
	}
	if(flag)
	{
		*(--p) = '-';
	}
	if(length > (len + flag + 1))
	{
		if(length > sizeof(st))
		{
			nlen = sizeof(st) - len - flag - 1;
		}
		else
		{
			nlen = length - len - flag - 1;
		}
		for(i = 0;i< nlen;i++)
		{
			*(--p) = filler;
		}
		left_len = sizeof(st) - nlen - len - flag - 1;
	}	
	else
	{
		left_len = sizeof(st) - len - flag - 1;
	}
	for(i = 0; i < (sizeof(st) - left_len);i++)
	{
		st[i] = st[left_len + i];
	}
	st[i] = '\0';
	return String(st,27);
}


String fixedLengthFloat(double num, uint8_t dec, uint8_t divider, int16_t length, uint8_t filler)
{
	uint8_t st[27] = {0};
	uint8_t * p = st;
	boolean flag = false;
	int16_t i = 0;
	if(dec<1)
	{
		dec=1;
	}
	else if(dec>5)
	{
		dec=5;
	}
	if(num<0)
	{
		flag = true;
	}
	dtostrf(num, length, dec, (char *)st);
	if(divider != '.')
	{
		while(i < sizeof(st))
		{
			if('.' == *(p+i))
			{
				*(p+i) = divider;
			}
			i++;
		}	
	}
	if(filler != ' ')
	{
		if(flag)
		{
			*p = '-';
			i = 1;
			while(i < sizeof(st))
			{
				if((*(p+i) == ' ') || (*(p+i) == '-'))
				{
					*(p+i) = filler;
				}
				i++;
			}
		}
		else
		{
			i = 0;
			while(i < sizeof(st))
			{
				if(' ' == *(p+i))
				{
					*(p+i) = filler;
				}
			}
		}
	}
	return String(st,27);
}

void waitForTouch() {
  unsigned long start = millis();
  while (millis() - start < 500) {
    TouchPoint touchPos = touchScreen.read();
    if (touchPos.touched) {
      DEBUG_PORT.println("touch detected");
      // Touch erkannt, beende die Funktion und gib true zurück
      debug = true;
      return;
    }
  }
  // Kein Touch erkannt, gib false zurück
  DEBUG_PORT.println("no touch detected");
  debug = false;
  return;
}