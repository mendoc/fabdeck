#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "WiFiClient.h"

// Définition des PIN
#define PIN_LED_BLEU 5
#define PIN_BOUTON_1 4
#define PIN_BOUTON_2 14
#define PIN_BOUTON_3 12
#define PIN_BOUTON_4 13

// Différents états du Fabdeck
#define RECHERCHE 1 // Tentative de connexion au WiFi
#define CONNECTE 2  // Connecté au WiFi mais pas à Internet
#define ENLIGNE 3   // Connecté à Internet

// Informations de connexion
const char * SSID = "NOM_DU_WIFI";
const char * PASSW = "MOT_DE_PASSE_DU_WIFI";

// Varibles de travail
int etat;  // Pour gérer l'état du Fabdeck
unsigned long momentDernierPing;     // Pour le moment du dernier ping au serveur
unsigned long momentDernierCligno;   // Pour le moment du dernier allumage de la LED
unsigned long momentDernierAppui;    // Pour le moment du dernier appui
int dureeEntreDeuxPings = 30 * 1000; // 30 secondes
int dureeEntreDeuxAppuis = 3 * 1000; // Pour la durée de 3 secondes entre deux appuis de bouton;

HTTPClient httpClient;
WiFiClient client;

// Enènements pour le WiFi
void onConnected(const WiFiEventStationModeConnected& event);
void onGotIP(const WiFiEventStationModeGotIP& event);

void setup() {
  // Configuration de la liaison série
  Serial.begin(115200);

  // Définition des pins
  pinMode(PIN_LED_BLEU, OUTPUT);
  pinMode(PIN_BOUTON_1, INPUT_PULLUP);
  pinMode(PIN_BOUTON_2, INPUT_PULLUP);
  pinMode(PIN_BOUTON_3, INPUT_PULLUP);
  pinMode(PIN_BOUTON_4, INPUT_PULLUP);

  // Initialisation des variables de travail
  momentDernierPing = millis();
  momentDernierCligno = millis();
  momentDernierAppui = millis();
  etat = RECHERCHE;

  // Connexion au réseau WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSW);

  // Liaison des évènements du WiFi
  static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
  static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);
}

void loop() {
  // Gestion du clignotement de la LED
  if (etat == RECHERCHE) {
    // La LED clignote rapidement
    clignoter(PIN_LED_BLEU, 300);
  } else if (etat == CONNECTE) {
    // La LED clignote lentement
    clignoter(PIN_LED_BLEU, 1000);
  } else if (etat == ENLIGNE) {
    // La LED est allumée
    digitalWrite(PIN_LED_BLEU, HIGH);
  }

  // A intervalle de temps régulier on vérifie si on est connecté à Internet
  if (WiFi.isConnected()) {
    // Gestion du ping au serveur
    if (millis() - momentDernierPing >= dureeEntreDeuxPings || momentDernierPing < 3000) {
      // Requete vers le serveur
      ping();
      momentDernierPing = millis();
    }

    if (millis() - momentDernierAppui >= dureeEntreDeuxAppuis) {
      // Gestion de l'appui sur le bouton 1
      if (digitalRead(PIN_BOUTON_1) == LOW) {
        momentDernierAppui = millis();
        gererAppui(1);
      }

      // Gestion de l'appui sur le bouton 2
      if (digitalRead(PIN_BOUTON_2) == LOW) {
        momentDernierAppui = millis();
        gererAppui(2);
      }
      
      // Gestion de l'appui sur le bouton 3
      if (digitalRead(PIN_BOUTON_3) == LOW) {
        momentDernierAppui = millis();
        gererAppui(3);
      }
      
      // Gestion de l'appui sur le bouton 4
      if (digitalRead(PIN_BOUTON_4) == LOW) {
        momentDernierAppui = millis();
        gererAppui(4);
      }
    }
  } else {
    etat = RECHERCHE;
  }
}

void gererAppui(int btnNumero) {
  // Gestion de l'appui sur un bouton
  Serial.print("Bouton ");
  Serial.print(btnNumero);
  Serial.println(" pressé !");

  String url = String("http://fabdeck.netlify.app/.netlify/functions/routeur?e=");
  url += btnNumero;
  envoyerEvent(url);
}

void ping() {
  Serial.println("");
  Serial.println("Ping au serveur");
  envoyerEvent("http://fabdeck.netlify.app/.netlify/functions/routeur?e=5");
}

void envoyerEvent(String url) {
  if (httpClient.begin(client, url)) {
    int httpCode = httpClient.GET();
    if (httpCode == HTTP_CODE_OK) {
      etat = ENLIGNE;
      Serial.println("Fabdeck connecté à Internet");
    } else {
      Serial.print("Code erreur : ");
      Serial.print(httpCode);
      Serial.print(" - ");
      Serial.println(httpClient.errorToString(httpCode).c_str());
      etat = CONNECTE;
    }
  } else {
    Serial.println("Connexion impossible");
    etat = CONNECTE;
  }
  httpClient.end();
}

void clignoter(int pin, int duree) {
  if (millis() - momentDernierCligno >= duree) {
    if (digitalRead(pin) == LOW) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    momentDernierCligno = millis();
  }
}

// Gestion de l'évènement de connexion au WiFi
void onConnected(const WiFiEventStationModeConnected& event) {
  Serial.println("");
  Serial.println("Connecté au WiFi");
}

// Gestion de l'évènement d'obtention d'une adresse IP
void onGotIP(const WiFiEventStationModeGotIP& event) {
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
  Serial.print("Puissance du signal : ");
  Serial.println(WiFi.RSSI());
  etat = CONNECTE;
}