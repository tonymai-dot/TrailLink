#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define LARGEUR_ECRAN 128 
#define HAUTEUR_ECRAN 64
#define ADRESSE_I2C 0x3C
Adafruit_SSD1306 ecran(LARGEUR_ECRAN, HAUTEUR_ECRAN, &Wire, -1);
void setup() {
  Serial.begin(115200);
  if(!ecran.begin(SSD1306_SWITCHCAPVCC, ADRESSE_I2C)) {
    Serial.println(F("Erreur : Impossible de trouver l'écran OLED SSD1306"));
    while(1); // Bloque le programme si l'écran n'est pas détecté
  }

  ecran.clearDisplay();
  ecran.setTextSize(2);          // Taille des caractères (1 = petit, 2 = moyen, etc.)
  ecran.setTextColor(SSD1306_WHITE); // Couleur du texte (Blanc sur fond noir)
  
  ecran.setCursor(10, 20);       

  ecran.print("bonjour");

  ecran.display();
}

void loop() {
}