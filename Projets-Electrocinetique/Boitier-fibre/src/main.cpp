#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

int SlowWrite(byte b); // fonctions  maison pour une com série ultra-lente et non bloquante
char SlowRead();

void clearString(char *str, int len = 16)
{
  for (int i = 0; i < len; i++)
    str[i] = 0;
}

const int T_REPET_BUTTON = 300;  // Delay de repetition des bouton
const int T_USER_INTERFACE = 50; // Delay d'update de l'UI

LiquidCrystal lcd(12, 13, 11, 10, 9, 8); // L'écran
SoftwareSerial SerialS(2, 3, true);      // Liaison série soft pins 2 et 3

enum mode
{
  ON,
  OFF,
  BLINK,
  MSG1,
  MSG2,
  RECEIVE,
  REPEATER
};

const char *ModeDescription[] = {
    "    Laser ON    ",
    "    Laser OFF   ",
    "    CLIGNOTE    ",
    "SORBONNE UNIV.>>",
    "-- BONJOUR -- >>",
    ">>              ",
    "              >>"};

char message1[] = "SORBONNE UNIV.";
char message2[] = "-- BONJOUR -- ";

int modeMAX = 5;
int ModeSelect = RECEIVE;

long ComSpeed[] = {10, 30, 50, 100, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
int SpeedMAX = 15;
int SpeedSelect = 2;

long Freq[] = {1, 5, 10, 50, 100, 500, 1000, 5000, 10000, 50000};
int FreqMAX = 9;
int FreqSelect = 0;
unsigned long HalfPeriod_us = 500000;

int MsBetweenChar = 200; // delay entre chaque caractère en ms

char StringRX[16]; // chaine de caractère lu sur le port série
char StringTX[16]; // chaine de caractère lu sur le port série pout transmission

void setup()
{
  // Démarrage de l'écran
  lcd.begin(16, 2);

  // Démarrage d'une liason série avec le PC
  Serial.begin(9600);
  Serial.println("Terminal Série");
  Serial.println("Projet Fibre Optique");

  // Config des pins des boutons
  pinMode(A3, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);

  pinMode(2, INPUT);
}

bool refresh = true;

void loop()
{

  /****************** Interface utilisateur ********************/
  static unsigned long tButRepet = 0;
  static unsigned long tUserInt = 0;

  // static unsigned long tbut = 0;
  char str[50];
  static int index = 0;
  static int indexToSend = 0;

  if (millis() > tUserInt + T_USER_INTERFACE)
  {
    tUserInt = millis();

    // Liaison série PC
    if (Serial.available())
    {                             // Si le PC parle à la carte
      if (ModeSelect != REPEATER) // passe en mode repeteur
      {
        ModeSelect = REPEATER;
        // indexToSend = index = 0;
        // clearString(StringTX);
        refresh = true;
      }
    }

    // Gestion des boutons

    static bool Butdisabled;

    bool SpUp = !digitalRead(A2); // Speed Up button pushed
    bool SpDw = !digitalRead(A3); // Speed Down button pushed
    bool MdUp = !digitalRead(A0); // Mode Up button pushed
    bool MdDw = !digitalRead(A1); // Mode Down button pushed

    if ((millis() > tButRepet + T_REPET_BUTTON) || !(SpUp || SpDw || MdUp || MdDw)) // Réarme les boutons
      Butdisabled = false;

    if (!Butdisabled)
    {

      if (SpUp || SpDw || MdUp || MdDw) // Un bouton a été appuyé
      {
        tButRepet = millis();
        refresh = true;
        Butdisabled = true; // Désarme les boutons

        if (ModeSelect == BLINK)
        {
          if (SpDw && FreqSelect > 0)
            FreqSelect--;
          if (SpUp && FreqSelect < FreqMAX)
            FreqSelect++;
        }
        else if (ModeSelect > BLINK)
        {
          if (SpDw && SpeedSelect > 0)
            SpeedSelect--;
          if (SpUp && SpeedSelect < SpeedMAX)
            SpeedSelect++;
        }

        if (MdDw && ModeSelect > 0)
          ModeSelect--;

        if (MdUp && ModeSelect < modeMAX)
          ModeSelect++;
      }
    }

    if (refresh)
    { // Rafraichit l'affichage et change la vitesse de com

      if (ComSpeed[SpeedSelect] >= 300)
        SerialS.begin(ComSpeed[SpeedSelect]);
      else
        SerialS.end();
      HalfPeriod_us = (unsigned long)(0.5 * 1e6 / (float)Freq[FreqSelect]);

      lcd.setCursor(0, 0);
      //Serial.println(ModeDescription[ModeSelect]);
      lcd.print(ModeDescription[ModeSelect]);
      indexToSend = index = 0;
      clearString(StringRX);
      clearString(StringTX);

      lcd.setCursor(0, 1);

      if (ModeSelect == BLINK)
        sprintf(str, "FREQ %6ld Hz  ", Freq[FreqSelect]);
      else if (ModeSelect == ON || ModeSelect == OFF)
        sprintf(str, "                ");
      else
        sprintf(str, "COM %6ld bit/s", ComSpeed[SpeedSelect]);
      //Serial.println(ComSpeed[SpeedSelect]);
      lcd.print(str);

      refresh = false;
    }

    //// Affiche un curseur si c'est pas trop rapide
    if ((ModeSelect == MSG1 || ModeSelect == MSG2 || ModeSelect == REPEATER) && ComSpeed[SpeedSelect] <= 100)
    {
      lcd.cursor();
      lcd.setCursor(index, 0);
    }
    else
      lcd.noCursor();

    //// Affiche le message si en mode réception
    if (ModeSelect == RECEIVE)
    {
      lcd.setCursor(2, 0);
      lcd.print(StringRX);
      // lcd.cursor();
      // lcd.setCursor(index + 2, 0);
    }

    if (ModeSelect == REPEATER)
    {
      lcd.setCursor(0, 0);
      lcd.print(StringTX);
      lcd.cursor();
      lcd.setCursor(indexToSend - 1, 0);
    }
  }

  /*************************************** Gestion du laser ************************/

  static unsigned long tOld_us; // Utilisé pour le clignotement
  static bool state = false;
  unsigned long t_us = micros();

  switch (ModeSelect)
  {
  case ON:
    digitalWrite(3, HIGH);
    break;
  case OFF:
    digitalWrite(3, LOW);
    break;
  case BLINK:
  {
    int NRep = 1;
    if (Freq[FreqSelect] >= 100L)
    {
      NRep = Freq[FreqSelect] / 4;

      for (int i = 0; i < NRep; i++)
      {
        digitalWrite(3, state);
        state = !state;
        delayMicroseconds(HalfPeriod_us);
      }
    }
    else
    {
      if (t_us - tOld_us > HalfPeriod_us - 6)
      { // Toggle laser
        digitalWrite(3, state);
        state = !state;
        tOld_us += HalfPeriod_us;
      }
    }
  }
  break;

  case MSG1:
  case MSG2:
  {
    char *message = ModeSelect == MSG1 ? message1 : message2;

    if (SlowWrite(message[index]))
      index = (index + 1) % 14;
  }
  break;
  case RECEIVE:
  {
    char c = SlowRead();

    if (c != 0)
    {
      Serial.print(c);
      if (index < 14)
      {
        StringRX[index] = c;
        index++;
      }
      else
      {
        for (int i; i < 13; i++)
          StringRX[i] = StringRX[i + 1];
        StringRX[13] = c;
      }
    }
  }
  break;
  case REPEATER:

    if (indexToSend < index)
      if (SlowWrite(StringTX[indexToSend]))
        indexToSend++;

    if (Serial.available())
    {
      char c = Serial.read();
      if (c < ' ') // Si le caractère n'est pas imprimable on met un espace
        c = ' ';
      // break;
      // Serial.print((int)c);Serial.print(" ");
      if (index < 14)
      {
        StringTX[index] = c;
        index++;
      }
      else if (indexToSend > 0)
      {
        for (int i = 0; i < 13; i++)
          StringTX[i] = StringTX[i + 1];
        StringTX[13] = c;
        indexToSend--;
      }
    }
    break;
  }
}

int SlowWrite(byte b)
{
  static long nextT = 0;
  static int k = -1; // numéro du bit
  static long Periode = 1e6 / ComSpeed[SpeedSelect];

  if (ComSpeed[SpeedSelect] >= 300) // Si la vitesse est grande on utilise le software serial
  {
    SerialS.write(b);
    delay(100);
    return 1;
  }

  long t = micros();

  if (t > nextT + 2 * Periode) // il y a eu un bug on reset
    k = -1;

  if (t > nextT) // Next bit
  {
    if (k < 0)
    {
      Periode = 1e6 / ComSpeed[SpeedSelect];
      digitalWrite(3, HIGH); // bit d'amorçe
      nextT = t + Periode;
      k = 0;
      return 0;
    }

    if (k < 8)
    {
      digitalWrite(3, !(b & 1 << k)); // next bit
      nextT = nextT + Periode;
      k++;
      return 0;
    }
    else
    {
      digitalWrite(3, LOW);
      k = -1;
      nextT = nextT + 10 * Periode;
      return 1;
    }
  }

  return 0;
}

char SlowRead()
{
  static long nextT = 0;
  static int k = -1; // numéro du bit
  static long Periode = 1e6 / ComSpeed[SpeedSelect];
  static char c;

  if (ComSpeed[SpeedSelect] >= 300)
  {
    if (SerialS.available())
      return SerialS.read();
    else
      return 0;
  }

  long t = micros();

  if (t > nextT + 2 * Periode) // il y a eu un bug on reset
    k = -1;

  if (k < 0)
  {
    Periode = 1e6 / ComSpeed[SpeedSelect];
    c = 0;
    bool inputB = digitalRead(2);

    if (inputB == LOW) // attend un front montant
      return 0;
    else // On a le front on démarre la lecture
    {
      k = 0;
      nextT = t + Periode + Periode / 4;
      return 0;
    }
  }

  if (t > nextT) // next bit
  {
    bool inputB = digitalRead(2);
    if (k < 8)
    {
      if (inputB == LOW)
        c += 1 << k;
      nextT = nextT + Periode;
      k++;
      return 0;
    }
    else
    {
      k = -1;
      return c;
    }
  }

  return 0;
}