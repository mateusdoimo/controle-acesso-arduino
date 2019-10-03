#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <SD.h>
#include <DS3231.h>
DS3231  rtc(SDA, SCL);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 8
#define RST_PIN 9

// Definicoes pino modulo RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x3f,2,1,0,4,5,6,7,3, POSITIVE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "www.teste.com.br";    // name address for Google (using DNS)
IPAddress ip(0, 0, 0, 0);
EthernetClient client;

const int buzzer = 5;
const int livre = 8;
//const int erro = 7;
const int pronto = 6;
const int aguarde = 3;

int valB = 0; // variável para ler o status do pino

int inPin = 7; // porta 7 em input (para o push button)

File webFile;

void setup() {
 
  pinMode(buzzer,OUTPUT);
  pinMode (livre, OUTPUT);
  pinMode(inPin, INPUT); 
  pinMode (pronto, OUTPUT);
  pinMode (aguarde, OUTPUT);
  
  digitalWrite (livre, LOW);
  digitalWrite (pronto, LOW);
  digitalWrite (aguarde, HIGH);
  
  lcd.begin (16,2);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("TESTE");
  lcd.setCursor(0,1);
  lcd.print("ACESSO");

  // Inicia  SPI bus
  SPI.begin();
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  // Inicia MFRC522
  mfrc522.PCD_Init(); 
  // Mensagens iniciais no serial monitor
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
  digitalWrite (aguarde, LOW);
  digitalWrite (pronto, HIGH);

  rtc.begin();
  
  // Imprime no monitor serial:
  Serial.print("Initializing SD card...");
 
  // Inicializa o cartão e imprime o resultado no monitor serial:
  if (!SD.begin(4)) {
    Serial.println("Falha na inicialização!");
    return;
  }
  Serial.println("Cartão inicializado!");
}

void loop() {


  valB = digitalRead(inPin); // ler a entrada de valor
  
  if (valB == HIGH) {// verificar se a entrada é alta
  lcd.setCursor(0,1);
  lcd.print("DESLIGADO");
  } else {
  
  lcd.setCursor(0,1);
  lcd.print("LIGADO");
  }

  // Aguarda a aproximacao do cartao
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Seleciona um dos cartoes
  if ( !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  tone(buzzer,1000);
  delay(500);
  noTone(buzzer);
  conect(conteudo.substring(1));
}

void conect(String tag) {
   tag.replace(" ", "%20");
  
   if (client.connect(server,80)) { // REPLACE WITH YOUR SERVER ADDRESS
     client.print("GET /acesso.php?"); 
     client.print("t=");
     client.print(tag);
     client.println( " HTTP/1.1");
     client.println( "Host: www.teste.com.br" );
     client.println("Content-Type: application/x-www-form-urlencoded"); 
     client.println( "Connection: close" );
     client.println(); 
     client.println();
   } 
    int i = 0;
   
   
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
              i++;
      }
    }
    
    if (i == 162){
       digitalWrite (pronto, LOW);
       digitalWrite (livre, HIGH);
       tone(buzzer,2000);
       delay(500);
       noTone(buzzer);
       digitalWrite (livre, LOW);
    }
    else if (i == 163){
       digitalWrite (pronto, LOW);
       digitalWrite (livre, LOW);
       tone(buzzer,500);
       delay(500);
       noTone(buzzer);
    }
    client.stop();
    digitalWrite (pronto, HIGH);
}

