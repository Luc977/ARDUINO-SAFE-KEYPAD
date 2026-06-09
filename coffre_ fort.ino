#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

LiquidCrystal lcd(7,8,9,10,11,12);
Servo monServo;
const byte ROWS=4,COLS=4;
char keys[4][4]={{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
byte rowPins[4]={2,3,4,5}; byte colPins[4]={A0,A1,A2,A3};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);
const int LED_VERTE=A4,LED_ROUGE=A5,BUZZER=13;

String codeActuel="", saisie="";
int essais=0;
int indexCode = 0;
unsigned long lockout=0;
bool changeMode=false;

void bip(){tone(BUZZER,2000,100);}

void afficherEtoiles(){
  lcd.setCursor(5,0);
  for(int i=0; i<4; i++){
    if(i < saisie.length()) lcd.print("*");
    else lcd.print(" ");
  }
}

void retourAccueil(){
  delay(1800);
  lcd.clear();
  lcd.print("Code: ");
  saisie="";
}

void sauverCode(String nouveauCode){
  for(int i=0; i<4; i++){
    EEPROM.write(i, nouveauCode[i]);
    delay(10); // Laisse le temps d’écrire
  }
  EEPROM.write(10, 42); // Flag
  delay(10);

  // VERIFICATION : On relit pour être sûr
  String testCode="";
  for(int i=0; i<4; i++) testCode += (char)EEPROM.read(i);

  lcd.clear();
  if(testCode == nouveauCode){
    lcd.print("Code Changer");
  } else {
    lcd.print("Erreur SAUVE!");
  }
}

void setup(){
  lcd.begin(16,2);
  pinMode(LED_VERTE,OUTPUT); 
  pinMode(LED_ROUGE,OUTPUT); 
  pinMode(BUZZER,OUTPUT);
  monServo.attach(6); 
  monServo.write(0);

  if(EEPROM.read(10) == 42){
    codeActuel = "";
    for(int i=0; i<4; i++) codeActuel += (char)EEPROM.read(i);
  } else {
    codeActuel = "1234";
    sauverCode(codeActuel);
  }
  lcd.print("Code: ");
}

void loop(){
  if(lockout>millis()){
    lcd.clear(); 
    lcd.print("BLOQUE"); 
    lcd.setCursor(0,1);
    lcd.print((lockout-millis())/1000); 
    lcd.print("s");
    digitalWrite(LED_ROUGE,HIGH); 
    delay(300); 
    digitalWrite(LED_ROUGE,LOW); 
    delay(300);
    lcd.clear();
    lcd.print("Code: ");
    return;
  }

  char key=keypad.getKey();
  if(key){
    bip();

    if(key=='*' && saisie==codeActuel &&!changeMode){
      changeMode=true; saisie=""; 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("N_cde");
      lcd.setCursor(0,1);
      for(int i=0; i<indexCode; i++) lcd.print("*");
    }

    else if(key=='#'){
      if(changeMode){
        if(saisie.length()==4){
          codeActuel=saisie;
          sauverCode(codeActuel);
          delay(1500);
          retourAccueil();
          changeMode=false;
        } else {
          lcd.clear(); lcd.print("4 chiffres!");
          retourAccueil();
        }
      }

      else{
        if(saisie==codeActuel){
          lcd.clear(); lcd.print("OK Ouvert");
          digitalWrite(LED_VERTE,HIGH); monServo.write(90);
          bip(); delay(3000); monServo.write(0);
          digitalWrite(LED_VERTE,LOW); essais=0;
        } else {
          essais++; lcd.clear(); lcd.print("ERREUR");
          digitalWrite(LED_ROUGE,HIGH); bip(); delay(800);
          digitalWrite(LED_ROUGE,LOW);
          if(essais>=3) lockout=millis()+60000;
        }
        retourAccueil();
      }
    }

    else if(key!='*' && key!='#'){
      if(saisie.length()<4){
        saisie+=key;
        afficherEtoiles();
      }
    }
  }
}
