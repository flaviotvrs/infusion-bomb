// Bomba de Infusão

// Bibliotecas externas
#include <LiquidCrystal.h>
#include <AccelStepper.h>

// Variáveis de ajuste do equipamento
#define MIN_POSITION 0
#define MAX_POSITION 36400
#define MAX_POTENTIOMETER 868
#define MAX_ML 20
#define MAX_FLOW_RATE 10
#define LOADING_SPEED 5000

// Variáveis de localização dos sensores no arduino
#define LCD_RS_PIN 42
#define LCD_EN_PIN 43
#define LCD_D4_PIN 44
#define LCD_D5_PIN 45
#define LCD_D6_PIN 46
#define LCD_D7_PIN 47
#define STEPPER_EN_PIN 1
#define STEPPER_CLK_PIN 24
#define STEPPER_CW_PIN 22
#define READY_LED_PIN 32
#define LOAD_LED_PIN 34
#define INFUSION_LED_PIN 36
#define SET_BUTTON_PIN 33
#define RESTART_BUTTON_PIN 35
#define SET_PIN A5

// Inicializa o objeto responsável por lidar com o LCD
LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

// Inicializa o objeto responsável por lidar com o motor
AccelStepper stepper(STEPPER_EN_PIN, STEPPER_CLK_PIN, STEPPER_CW_PIN);

// Variáveis de controle de execução
boolean loadFinish = false;

// Esta função é chamada uma única vez logo que o arduino é ligado
// Faz a configuração inicial dos pinos de entrada e saída
void setup()
{
   pinMode(SET_BUTTON_PIN, INPUT);
   pinMode(RESTART_BUTTON_PIN, INPUT);

   pinMode(READY_LED_PIN, OUTPUT);
   pinMode(LOAD_LED_PIN, OUTPUT);
   pinMode(INFUSION_LED_PIN, OUTPUT);

   lcd.begin(16, 2);
   lcdPrint("BOMBA DE INFUSAO", "Iniciando ...");

   // Faz um charme na inicialização
   delay(2000);
}

// Função principal do código, ela é chamada em loop pelo arduino após a inicialização (setup).
// Nesta função é implementada toda a lógica dos procedimentos para realizar a infusão:
// * Leitura do volume a ser carregado e injetado em mL
// * Carregamento da seringa
// * Leitura da vazão a ser injetada em mL/s
// * Realização da injeção com base na vazão desejada
// * Por fim, aguarda a indicação de reinício dos procedimentos através do botão restart (configurado no RESTART_BUTTON_PIN)
void loop()
{
   lcdPrint("BOMBA DE INFUSAO", "Pronto!");
   
   // Aguarda o usuário pressionar o botão de início
   while (!digitalRead(RESTART_BUTTON_PIN));

   readySign();
   float volumeMl = readVolumeMl();
   lcdPrint("Carregando a", "seringa ...");
   loading(volumeMl);
   readySign();
   long flowRateMlS = readFlowRateMlS();
   lcdPrint("Realizando a", "infusao ...");
   infusion(flowRateMlS);
   lcdPrint("    INFUSAO     ", "  FINALIZADA :D ");
   delay(2000);
}

// Função que lê e recebe o volume em mL
// Faz a leitura do potenciômetro ao passo que apresenta a informação lida na tela
// Após o usuário confirmar a leitura a função retorna o valor
float readVolumeMl()
{
   float volume;
   boolean volumeRead = false;
   while (!volumeRead)
   {
      long input = analogRead(SET_PIN);
      long inputMl = MAX_ML * input / MAX_POTENTIOMETER;

      // Impede que seja lido um volume maior do que o máximo permitido
      // Necessário pois pode haver mudança na leitura do potenciômetro devido a interferências
      if (inputMl > MAX_ML)
      {
         inputMl = MAX_ML;
      }

      char mlText[16];
      sprintf(mlText, "%d mL           ", inputMl);
      lcdPrint("Informe o volume", mlText);

      // se o usuário confirmar, ou seja, pressionar o botão de confirmação
      if (digitalRead(SET_BUTTON_PIN))
      {
         volumeRead = true;
         volume = inputMl;
      }

      // um pequeno delay para evitar que a tela do LCD fique piscando demais
      delay(500);
   }

   return volume;
}

// Função que lê e recebe a vazão em mL/s
// Faz a leitura do potenciômetro ao passo que apresenta a informação lida na tela
// Após o usuário confirmar a leitura a função retorna o valor
long readFlowRateMlS()
{
   long flowRate;
   boolean inputRead = false;
   while (!inputRead)
   {
      long input = analogRead(SET_PIN);
      long inputMlS = MAX_FLOW_RATE * input / MAX_POTENTIOMETER;

      // Impede que seja lido um volume maior do que o máximo permitido
      // Necessário pois pode haver mudança na leitura do potenciômetro devido a interferências
      if (inputMlS > MAX_FLOW_RATE)
      {
         inputMlS = MAX_FLOW_RATE;
      }

      char mlText[16];
      sprintf(mlText, "%d mL/s         ", inputMlS);
      lcdPrint("Informe a vazao", mlText);

      // se o usuário confirmar, ou seja, pressionar o botão de confirmação
      if (digitalRead(SET_BUTTON_PIN))
      {
         inputRead = true;
         flowRate = inputMlS;
      }

      // um pequeno delay para evitar que a tela do LCD fique piscando demais
      delay(500);
   }

   return flowRate;
}

// Função responsável por encher a seringa
// Parâmetro: volumeMl :: Volume em mL
void loading(float volumeMl)
{
   // Faz a tradução do volume em mL para quantidade de passos (steps) que o motor precisa andar
   long stepsToGo = volumeMl * MAX_POSITION / MAX_ML;

   loadingSign(true);
   
   // A aceleração e velocidade de carga da seringa é fixa
   stepper.setAcceleration(LOADING_SPEED);
   stepper.setMaxSpeed(LOADING_SPEED);
   // Inicia a carga da seringa fazendo o motor andar alguns passos (stepsToGo)
   stepper.runToNewPosition(stepsToGo);

   loadFinish = true;
   loadingSign(false);
}

// Função responsável por comprimir a seringa
// Parâmetro: flowRateMlS :: Vazão em mL/S
void infusion(float flowRateMlS)
{
   // Faz a tradução da vazão em mL/s para quantidade de passos por segundo (steps/s) que o motor precisa andar
   long maxSpeed = flowRateMlS / MAX_ML * MAX_POSITION;

   infusionSign(true);

   // A aceleração aqui é mantida com o mesmo valor da velocidade
   stepper.setAcceleration(maxSpeed);
   stepper.setMaxSpeed(maxSpeed);
   // Inicia a descarga da seringa retornando o motor para a posição inicial MIN_POSITION
   stepper.runToNewPosition(MIN_POSITION);

   loadFinish = false;
   infusionSign(false);
}

// Função para dar feedback para o usuário
// Indica, através do LED verde, que o sistema está pronto para novos comandos
void readySign()
{
   digitalWrite(READY_LED_PIN, HIGH);
   digitalWrite(LOAD_LED_PIN, LOW);
   digitalWrite(INFUSION_LED_PIN, LOW);
}

// Função para dar feedback para o usuário
// Indica, através do LED amarelo, que o sistema está realizando a carga da seringa
// Parâmetro: start :: true se é o início do procedimento ou false se é o fim do procedimento
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

// Função para dar feedback para o usuário
// Indica, através do LED azul, que o sistema está realizando a descarga da seringa
// Parâmetro: start :: true se é o início do procedimento ou false se é o fim do procedimento
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

// Função para dar feedback para o usuário
// Imprime mensagens no LCD
// Parâmetro: line1 :: Texto a ser apresentado na primeira linha do LCD
// Parâmetro: line2 :: Texto a ser apresentado na segunda linha do LCD
void lcdPrint(String line1, String line2)
{
   lcd.clear();

   lcd.setCursor(0, 0);
   lcd.print(line1);

   lcd.setCursor(0, 1);
   lcd.print(line2);
}
