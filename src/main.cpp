#include <Arduino.h>
#include <SoftwareSerial.h>
#include "AccelStepper.h"

//Pinos de comunicacao serial do modulo RS485
#define Pino_RS485_RX    10
#define Pino_RS485_TX    11

//Pino de controle transmissao/recepcao
#define SSerialTxControl 3

#define RS485Transmit    HIGH
#define RS485Receive     LOW

// Definindo todas as funções, sendo servo até dado recebido via Serial pelo Esp32
String ReciveDataSerial = "", Gravidade ="";

// Pinos diginais do Motor de Passo e as configurações
// Pinos
#define MotorDirPin 2
#define MotorStepPin 4
#define MotorEnable 5
// Configurações
#define MotorInterfaceType 1
#define MotorLigar LOW
#define MotorDesligar HIGH
int convert = 0;

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(MotorInterfaceType, MotorStepPin, MotorDirPin);

// Função que transforma de RPM para Passos por Segundo
int RPM_to_PPS(int RPM){
  int result = RPM*20/3;
  return result;
}

//Cria a serial por sofware para conexao com modulo RS485
SoftwareSerial RS485Serial(Pino_RS485_RX, Pino_RS485_TX);

void setup() {
  Serial.begin(9600);

  // Configarções Init motor de passo
  pinMode(MotorEnable, OUTPUT);
  digitalWrite(MotorEnable, MotorDesligar); 
  stepper.setMaxSpeed(RPM_to_PPS(150)); // Velocidade Máxima imposta será de 150 RPM
  pinMode(SSerialTxControl, OUTPUT);

  //Coloca o modulo RS485 em modo de recepcao
  digitalWrite(SSerialTxControl, RS485Receive);

  RS485Serial.begin(9600);
}

void loop() {
  // Leitura do Esp32
  if (RS485Serial.available())
  {
    while (RS485Serial.available())
    {
      char inChar = (char)RS485Serial.read();
      if (inChar != '\0') 
      {
        ReciveDataSerial += inChar;
      }
      
      if (inChar == '\n')
      {
        
        String nova_str = ReciveDataSerial.substring(0, ReciveDataSerial.length() - 2);
        String estadoLigar, whereIsSend;

        int firstComma = nova_str.indexOf(',');
        int secondComma = nova_str.indexOf(',', firstComma + 1);

        // Se tiver virgula separa os dados pela posição
        whereIsSend = nova_str.substring(0, firstComma); 
        
        // Se não tiver virgula é porque é um dado sozinho, como o "off" para desligar o led
        if (secondComma != -1) {
          Gravidade = nova_str.substring(secondComma+1); 
          estadoLigar = nova_str.substring(firstComma+1,secondComma); 
        }
        else estadoLigar = nova_str.substring(firstComma+1); 

        // Se não tiver virgula é porque é um dado sozinho, como o "off" para desligar o led
        if (whereIsSend == "ESP-Master"){
          if (estadoLigar == "off"){
            Gravidade = "";
            digitalWrite(MotorEnable, MotorDesligar);  // Desligar o motor
          }
          if (estadoLigar == "on"){
            digitalWrite(MotorEnable, MotorLigar);  // Liga o motor
          } 
          Serial.println(whereIsSend);
          Serial.println(estadoLigar);
          
        }
        
        ReciveDataSerial = "";
      }
    }
  }
  if (Gravidade != ""){
    stepper.setSpeed(atoi(Gravidade.c_str())); // Coloca a velocidade imposta pelo usuário
    stepper.runSpeed(); // Inicia o funcionamento
  } 
}