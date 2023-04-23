/* Bibliothèques requises */
#include <SoftwareSerial.h>      /* Connexion série */
#include "DFRobotDFPlayerMini.h" /* Lecteur MP3 */
#include "RotaryDialer.h"        /* Gestion du cadran rotatif */

/* Définition des constantes */
#define PIN_READY A2
#define PIN_PULSE 6
#define PIN_HANG A3

#define TIMEOUT 6000 /* Délai pour attendre le deuxième chiffre (en millisecondes) */

/* Déclaration des variables */
SoftwareSerial mySoftwareSerial(10, 11);  // RX, TX
DFRobotDFPlayerMini myDFPlayer;
int firstDigit = -1;
int numberSpecified = -1;
int pendingNumber = -1;
RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);
unsigned long digitTimeout = 0;

bool isFirstPlaySinceHangUp = true;

void setup() {
 /* Connexion série pour la remontée d'informations au PC */
 // Serial.begin(9600); 
  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);

  /* Initiation de la gestion du cadran rotatif */
  dialer.setup();

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    while (true)
      ;
  }

  /* Etat initial du DFPlayer */
  myDFPlayer.pause();
  myDFPlayer.volume(1);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);
}
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
}
void loop() {
  /* Récupération de l'état de décroché/raccroché */

  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 et on réinitialise les variables */
  if (isHangedUp()) {
  //  Serial.print("stop");
    myDFPlayer.pause();
    firstDigit = -1;
    pendingNumber = numberSpecified;
    digitTimeout = 0;
    isFirstPlaySinceHangUp = true;
    return;
  }
  
if (!isHangedUp() && pendingNumber != -1) {
  numberSpecified = pendingNumber;
  pendingNumber = -1;
}
  // Si une nouvelle impulsion a été détectée sur le cadran
  if (dialer.update()) {
    // Récupérer le numéro composé
    int dialedNumber = dialer.getNextNumber();

    // Si c'est le premier chiffre composé, le stocker dans 'firstDigit' et définir le délai pour attendre un deuxième chiffre
    if (firstDigit == -1) {
      firstDigit = dialedNumber;
      digitTimeout = millis() + TIMEOUT;
    } else {
      // Si un deuxième chiffre est composé, calculer le numéro final en combinant les deux chiffres
      numberSpecified = firstDigit * 10 + dialedNumber;
      firstDigit = -1; // Réinitialiser 'firstDigit' pour détecter un nouveau numéro
    }
  }

  // Si le délai pour attendre un deuxième chiffre est dépassé et que le premier chiffre a été composé
  if (digitTimeout != 0 && millis() > digitTimeout) {
    // Utiliser le premier chiffre comme numéro spécifié (puisque le deuxième chiffre n'a pas été composé)
    numberSpecified = firstDigit;
    firstDigit = -1; // Réinitialiser 'firstDigit' pour détecter un nouveau numéro
    digitTimeout = 0; // Réinitialiser le délai
  }

  // Si un 0 est composé seul, on l'interprète comme le nombre 100
  if (numberSpecified == 0) {
    numberSpecified = 100;
  }

  /* Si un numéro a été composé, alors on joue le MP3 correspondant */
  if (numberSpecified != -1) {
   // Serial.print(numberSpecified);
    myDFPlayer.pause();
    if (isFirstPlaySinceHangUp) {
      delay(1000);
      isFirstPlaySinceHangUp = false;
    }
    myDFPlayer.playMp3Folder(numberSpecified);
    numberSpecified = -1;
  }
}

