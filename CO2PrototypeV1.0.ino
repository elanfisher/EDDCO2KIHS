//Elan Fisher, 4/15/19

#include <LiquidCrystal.h>
#include <BigNumbers.h>
#include <Adafruit_NeoPixel.h>   //Led (Neopixel)

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
BigNumbers bigNum(&lcd);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, 13, NEO_GRB + NEO_KHZ800);

int ledPin = 5;
int displayButton = A0;
int muteButton = A1;
byte leds = 0;
int onOffCount = 0;
int transPin = 4;
boolean soundOff = false;
String currentColor = "none";

//----------co2--------
int sensorIn = A2;

int maxCurrent = 0;
int sensorValue = 0;
float voltage = 0.0;
int voltage_difference = 0;
float ppmVal = 0.0;

int temp = 0;
int spikeCount = 0;
int cO2 = 0;
bool next = false;
bool change = false;
int preVal = 0;

int average = 1;
int counter = 1;
//-----------co2-------

void setup() 
{
  pinMode(ledPin, OUTPUT);
  pinMode(displayButton, INPUT_PULLUP);  
  pinMode(muteButton, INPUT_PULLUP);
  pinMode(transPin, OUTPUT);
  pinMode(2,OUTPUT);
  digitalWrite(transPin, HIGH);
  pinMode(13,OUTPUT);//neopixel ring
  
  lcd.begin(16,2); // setup LCD rows and columns
  lcd.setCursor(0, 0);
  lcd.display();
  
  bigNum.begin(); // set up BigNumbers
  lcd.clear(); // clear display
  
  Serial.begin(115200); 
  Serial.println("Starting: Wait ~ 20s for a reading");
  //lcd.print("Loading... 20s");
  //lcd.setCursor(0, 1);
  //lcd.print("Elan CharlieLiza");

  soundOff = false;
  soundNoise();
}

void loop() 
{
  
  if(digitalRead(displayButton) == HIGH)
  {
    co2Read();
  }
  if (digitalRead(displayButton) == LOW)
  {
    delay(10);
    if (digitalRead(displayButton) == HIGH && onOffCount % 2 == 0)//on
    {
      onOffCount++;
      lcd.noDisplay();
      digitalWrite(transPin, LOW);
      
      Serial.println("d off");
    }
    else if(digitalRead(displayButton) == HIGH && onOffCount % 2 == 1)//off
    {
      onOffCount = 0;
      lcd.display();
      digitalWrite(transPin, HIGH);
      
      Serial.println("d on");
    }
  }
  if (digitalRead(muteButton) == LOW)
  {
    delay(10);
    if (digitalRead(muteButton) == HIGH && onOffCount % 2 == 0)
    {
      onOffCount++;
      
      soundOff = true;
      Serial.println("m off");
    }
    else if(digitalRead(muteButton) == HIGH && onOffCount % 2 == 1)
    {
      onOffCount = 0;
      
      soundOff = false;
      Serial.println("m on");
    }
  }
}


void changeColor(int ppm)
{
  lcd.clear();
  //ColorSetter(0 , 0 , 0 , 0);
  if(ppm < 800)//green
  {
    bigNum.displayLargeInt(ppm, 0, 4, false);
    
    lcd.setCursor(13, 1);
    lcd.print("ppm");
    
    if(preVal>= 800)
    {
      soundNoise();
      ColorSetter(0 , 255 , 0 , 0);
      preVal = ppm;
    }
    
    delay(10);
  }
  else if(ppm < 1300)//yellow
  {
    bigNum.displayLargeInt(ppm, 0, 4, false);
    
    lcd.setCursor(13, 1);
    lcd.print("ppm");
    
    if(preVal < 800||preVal>=1300)
    {
      soundNoise();
      ColorSetter(100 , 100 , 0 , 0);
      preVal = ppm;
    }
    
    delay(10);
  }
  else//red
  {
    bigNum.displayLargeInt(ppm, 0, 4, false);
    
    lcd.setCursor(13, 1);
    lcd.print("ppm");
    
    if(preVal<1300)
    {
      soundNoise();
      ColorSetter(255 , 0 , 0 , 0);
      preVal = ppm;
    }
    
    delay(10);
  }
  
}

void ColorSetter(uint16_t Red, uint16_t Green, uint16_t Blue, uint8_t white)
{
  uint16_t i;
  for (i = 0; i < strip.numPixels(); i++) //For each pixel
  {
    strip.setPixelColor(i, Red , Green , Blue, white);
    strip.show();
  }
}

void soundNoise()
{
  if(soundOff == false)
  {
    for(int i=0;i<80;i++)
    {
      digitalWrite(2,HIGH);
      delay(1);//wait for 1ms
      digitalWrite(2,LOW);
      delay(1);//wait for 1ms
    }
   
    //tone(2, NOTE_C5, 200);
    delay(50);
  }
}

void co2Read()
{
  delay(20);
  sensorValue = analogRead(sensorIn);
  //delay(200);

  voltage = sensorValue*(5000/1024.0);
  if(voltage == 0)
  {
    Serial.println("Fault");
  }
  else if(voltage < 400)
  {
    Serial.println("Preheating");
  }
  else
  {
    voltage_difference = voltage-450;
    ppmVal = voltage_difference*50.0/16.0;
    
    Serial.println("Concentration: " + String(ppmVal) + "ppm " + " count: " + counter);
    counter++;
    delay(100);
    //lcd.setCursor(1, 0);
    //lcd.print(int(ppmVal));
    //delay(100);
    average = average + ppmVal;
    if (next == true || counter == 70)
    {
      next = false;
      Serial.println("Final: " + String(ppmVal) + "ppm "+ String(counter));
      cO2 = ppmVal;
      
      average = average + ppmVal;
      //delay(100);
      changeColor(abs(ppmVal));
      delay(100);
      counter = 0;
      average = 0;
     }
     else if(ppmVal > temp + 120)
     {
       Serial.println("Spike: " + String(ppmVal) + "ppm " + "spike");
       delay(1000);
       sensorValue = analogRead(sensorIn);

       voltage = sensorValue*(5000/1024.0);
       voltage_difference = voltage-440;
       ppmVal = voltage_difference*50.0/16.0;
       //Serial.println("Spike Post: " + String(ppmVal) + "ppm ");
       
       temp = ppmVal;
       next = true;
     }
     else
     {
       temp = ppmVal;
     }
     counter++;
   }
}

