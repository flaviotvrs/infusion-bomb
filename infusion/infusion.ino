#include <LiquidCrystal.h>
#include <AccelStepper.h>

const int RS = 42, EN = 43, D4 = 44, D5 = 45, D6 = 46, D7 = 47;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

AccelStepper stepper(1, 24, 22); /* EN=1, CLK=24, CW=22 */

#define MIN_POSITION 0
#define MAX_POSITION 36400
#define MAX_POTENTIOMETER 868
#define MAX_ML 20
#define MAX_FLOW_RATE 10

#define READY_LED_PIN 32
#define LOAD_LED_PIN 34
#define INFUSION_LED_PIN 36

#define SET_BUTTON_PIN 33
#define RESTART_BUTTON_PIN 35

#define SET_PIN A5

boolean loadFinish = false;

void setup()
{
   Serial.begin(9600);

   pinMode(SET_BUTTON_PIN, INPUT);
   pinMode(RESTART_BUTTON_PIN, INPUT);

   pinMode(READY_LED_PIN, OUTPUT);
   pinMode(LOAD_LED_PIN, OUTPUT);
   pinMode(INFUSION_LED_PIN, OUTPUT);

   lcd.begin(16, 2);
   lcdPrint("BOMBA DE INFUSAO", "Iniciando ...");

   delay(2000);
}

void loop()
{
   readySign();
   float volumeMl = readQuantityMl();
   lcdPrint("Carregando a", "seringa ...");
   loading(volumeMl);
   readySign();
   long mlS = readVazaoMlS();
   lcdPrint("Realizando a", "infusao ...");
   infusion(mlS);
   lcdPrint("    INFUSAO     ", "  FINALIZADA :D ");

   boolean recomecar = false;
   while (!recomecar)
   {
      int buttonState = digitalRead(RESTART_BUTTON_PIN);
      if (buttonState == HIGH)
      {
         recomecar = true;
      }
   }
}

float readQuantityMl()
{

   boolean volumeRead = false;
   float volume;
   while (!volumeRead)
   {
      long input = analogRead(SET_PIN);

      long inputMl = MAX_ML * input / MAX_POTENTIOMETER;

      if (inputMl > MAX_ML)
      {
         inputMl = MAX_ML;
      }

      char mlText[16];
      sprintf(mlText, "%d mL           ", inputMl);
      lcdPrint("Informe o volume", mlText);

      int buttonState = digitalRead(SET_BUTTON_PIN);
      if (buttonState == HIGH)
      {
         volumeRead = true;
         volume = inputMl;
      }

      delay(500);
   }

   return volume;
}

long readVazaoMlS()
{

   long vazao;
   boolean inputRead = false;
   while (!inputRead)
   {
      long input = analogRead(SET_PIN);

      long inputMlS = MAX_FLOW_RATE * input / MAX_POTENTIOMETER;

      if (inputMlS > MAX_FLOW_RATE)
      {
         inputMlS = MAX_FLOW_RATE;
      }

      char mlText[16];
      sprintf(mlText, "%d mL/s         ", inputMlS);
      lcdPrint("Informe a vazao", mlText);

      int buttonState = digitalRead(SET_BUTTON_PIN);
      if (buttonState == HIGH)
      {
         inputRead = true;
         vazao = inputMlS;
      }

      delay(500);
   }

   return vazao;
}

void loading(float quantityMl)
{
   long calculatedQuantity = quantityMl * MAX_POSITION / MAX_ML;
   loadingSign(true);
   stepper.setAcceleration(1000);
   stepper.setMaxSpeed(5000);
   stepper.runToNewPosition(calculatedQuantity);
   loadFinish = true;
   loadingSign(false);
   Serial.print("Loaded! Current position: ");
   Serial.println(stepper.currentPosition());
}

void infusion(float mlS)
{
   Serial.print("Infusion Started! Current position: ");
   Serial.println(stepper.currentPosition());
   infusionSign(true);

   long maxSpeed = mlS / MAX_ML * MAX_POSITION;
   stepper.setAcceleration(maxSpeed);
   stepper.setMaxSpeed(maxSpeed);
   stepper.runToNewPosition(MIN_POSITION);
   loadFinish = false;
   infusionSign(false);
   Serial.print("Infusion Finished! Current position: ");
   Serial.println(stepper.currentPosition());
}

void readySign()
{
   digitalWrite(READY_LED_PIN, HIGH);
   digitalWrite(LOAD_LED_PIN, LOW);
   digitalWrite(INFUSION_LED_PIN, LOW);
}

void loadingSign(boolean start)
{
   digitalWrite(READY_LED_PIN, LOW);
   digitalWrite(INFUSION_LED_PIN, LOW);
   if (start)
   {
      digitalWrite(LOAD_LED_PIN, HIGH);
   }
   else
   {
      digitalWrite(LOAD_LED_PIN, LOW);
   }
}

void infusionSign(boolean start)
{
   digitalWrite(READY_LED_PIN, LOW);
   digitalWrite(LOAD_LED_PIN, LOW);
   if (start)
   {
      digitalWrite(INFUSION_LED_PIN, HIGH);
   }
   else
   {
      digitalWrite(INFUSION_LED_PIN, LOW);
   }
}

void lcdPrint(String line1, String line2)
{
   lcd.clear();

   lcd.setCursor(0, 0);
   lcd.print(line1);

   lcd.setCursor(0, 1);
   lcd.print(line2);
}
