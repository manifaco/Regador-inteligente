/* ******************** Regador inteligente ********************
   Guia de conexão:
   LCD RS: pino 9
   LCD Enable: pino 8
   LCD D4: pino 5
   LCD D5: pino 4
   LCD D6: pino 3
   LCD D7: pino 2
   LCD R/W: GND
   LCD VSS: GND
   LCD VCC: VCC (5V)
   Potenciômetro de 10K terminal 1: GND
   Potenciômetro de 10K terminal 2: V0 do LCD (Contraste)
   Potenciômetro de 10K terminal 3: VCC (5V)
   Sensor de umidade do solo A0: Pino A0
   Módulo Relé (Válvula): Pino 10  

   conectores do SD: (10,11,12,13,7cs)
   sensor de temperatura: pino A1 deficido como 15 para ser usado como digital
 ***************************************************************************** */

// bibliotecas *******************************************************************************************************************************************
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "DHT.h"

//váriáveis globais *****************************************************************************************************************************************
// define os pinos de conexão entre o Arduino e o Display LCD
const int rs = 9, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// variáveis do programa
const int pinoSensor = A0; //sensor de humidade do solo
const int temp = 15; //sensor de temperatura
float t = 0; //valor da temperatura
int h = 0; //valor da humidade do ar
const int pinoValvula = 6; //pino que ativa a solenoide
const int limiarSeco = 60; //a partir dessa porcentagem deve molhar a terra
int tempoRega= 0 ;
const int tempoMinRega = 9; // Tempo minimo de rega em segundos (situação que está frio)
int umidadeSolo = 0; //valor da umidade do solo
int regou = 0; //diz se regou ou não

#define DHTPIN temp     // Pino digital sensor DHT
#define DHTTYPE DHT11 //tipo do sensor de temperatura usado
DHT dht(DHTPIN, DHTTYPE);

// setup *******************************************************************************************************
void setup() {
  pinMode(pinoValvula, OUTPUT);
  digitalWrite(pinoValvula, HIGH);// Desliga a válvula
  lcd.begin(16, 2);// define o tamanho do Display LCD
  lcd.print("Rega Inteligente");// Exibe a mensagem no Display LCD.

  Serial.begin(9600);
  SD.begin(7); //pino cs
  dht.begin();
}

//loop**********************************************************************************************************************
void loop() {
  File dados = SD.open ("dados.txt", FILE_WRITE); //nome do arquivo que vai armazenar os dados (sem espaço, sem caracteres especiais, no máximo 8 caracteres)
  dados.println();

//tempo que ficará só medindo------------------------------------------------------------- 
  for(int i=0; i < 4 ; i++) {// Mede a umidade a cada segundo.
    lcd.setCursor(0, 1);// Posiciona o cursor do LCD na coluna 0 linha 1
    lcd.print("Umidadesolo ");// Exibe a mensagem no Display LCD:
    umidadeSolo = analogRead(pinoSensor);    // Faz a leitura do sensor de umidade do solo
    umidadeSolo = map(umidadeSolo, 1023, 0, 0, 100);// Converte a variação do sensor de 0 a 1023 para 0 a 100
    Serial.println (umidadeSolo);
    delay(1); //estabilizar o sensor
    lcd.print(umidadeSolo);    // Exibe a mensagem no Display LCD:
    lcd.print("%    ");
    delay(1500);    // Espera 2 segundo
   
    lcd.setCursor(0, 1);
    lcd.print("Temper ");
    t = dht.readTemperature();
    lcd.print(t);
    lcd.write(223); // Caracter °
    lcd.print("C ");
    Serial.println(t);
    delay(1500); 
   
    lcd.setCursor(0, 1);
    lcd.print("UmidadeAr ");
    h = dht.readHumidity();
    lcd.print(h);
    lcd.print("%    ");
    Serial.println(h);

    if (isnan(h) || isnan(t)) { // Verifique se alguma leitura falhou e tenta novamente.
      Serial.println(F("Falha de leitura do sensor DHT!"));
      return;
    }
    float hic = dht.computeHeatIndex(t, h, false);// Compute heat index in Celsius (isFahreheit = false)

    dados.print(umidadeSolo);
    dados.print(",");
    dados.print(t);
    dados.print(",");
    dados.print(h);
    dados.print(",");
    dados.print("2");
    dados.println();
    delay(1500);
  }

//hora de decidir se irriga ou não---------------------------------------------------------------
  if(umidadeSolo < limiarSeco) { //se entrar aqui vai molhar a terra
    if (t>30) tempoRega = tempoMinRega*2;
    else tempoRega = tempoMinRega;
    lcd.setCursor(0, 1);
    lcd.print("    Regando     ");
    digitalWrite(pinoValvula, LOW);// Liga a válvula
    
    delay(tempoRega*1000);// Espera o tempo estipulado
    digitalWrite(pinoValvula, HIGH);// desliga a válvula
   
    regou=1;
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("Solo Encharcado ");
    delay(3000);// Espera o tempo estipulado
    regou=0;
  }

//gravação do banco de dados------------------------------------------------------------------------------
  if(dados){
    dados.print(umidadeSolo);
    dados.print(",");
    dados.print(t);
    dados.print(",");
    dados.print(h);
    dados.print(",");
    dados.println(regou);
    dados.print("Fim");
    dados.close();
  }
  else Serial.println("erro ao criar o arquivo");
}
