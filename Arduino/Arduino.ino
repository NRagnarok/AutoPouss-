/* INCLUSIONS */
#include <OneWire.h> // librairie OneWire

/* CONSTANTES */
#define LED_PH_R 2
#define LED_PH_V 3
#define LED_PH_B 4
#define LED_TEMPERATURE_LED 5
#define LED_FONCT_R 7
#define LED_FONCT_V 10
#define LED_FONCT_B 11
#define LED_ALERTE_LED 13
#define PWM_ECLAIRAGE 6
#define PWM_VENTILATION 9
#define TOR_CHAUFFAGE 8
#define TOR_POMPE 12
#define ACQUISITION_THERMOMETRE 16
#define ACQUISITION_LUXMETRE A3
#define ACQUISITION_PHMETRE A1
#define RASPI_READY A0

/* OBJETS */
OneWire ds(ACQUISITION_THERMOMETRE); // Objet OneWire ds

/* VARIABLES */
long tempsNutrition = 30000;    //Temps de nutrition des plantes
long tempsPause = 120000;       //Temps d'arrêt de nutrition des plantes
int tempsSequentiel = 250;      //Temps d'intervalle de l'exécution du programme
int positionSequentielle = 1;   //Variable position du programme dans le temps (incrémentation toutes les tempSequentiel, raz à tempsNutrition+tempsPause)

float ph = 7;                   //Valeur acquise du pH
float paliersPh[2] = {5, 8};

int luminosite = 0;             //Valeur analogique de la luminosité
int luminositeArtificielle = 0; //Variable de la valeur de la luminosité artificielle
int luminositeRequise = 624;   //Configuration luminosité requise (sur 1024bits)

float temperature = 20;            //Valeur acquise de la température
int tempsCaptureTemperature = 750; // Temps minimum d'attente de la mesure supportée par le capteur : 750ms
long intervalleCaptureTemperature = 30000;
int etatVentilation = 0;          //Variable de la vitesse de la ventilation
int etatSystTemp = 0;         //Variable position température : 0 Tout va bien, 1 Sous chauffe, 2 Surchauffe
bool etatChauffage = 0;           //Booléen fonctionnement chauffage (0 Off, 1 On)
float paliersTemperature[4] = {18, 20, 22, 24};//Configuration température : [0] Température minimale, [1] Hystérésis min, [2] Hystérésis max, [3] Température maximale

bool pompe = 0;

int intervalleCommunicationRPI = 1000;

void setup() {
  /* DEMARRAGE DE LA LIAISON SERIE */
  Serial.begin(9600);
  /* PARAMETRAGE BROCHES VOYANTS */
  pinMode(LED_PH_R, OUTPUT); // LED RVB Ph R
  pinMode(LED_PH_V, OUTPUT); // LED RVB Ph V
  pinMode(LED_PH_B, OUTPUT); // LED RVB Ph B
  pinMode(LED_TEMPERATURE_LED, OUTPUT); // LED Température
  pinMode(LED_FONCT_R, OUTPUT); // LED RVB Fonct. R
  pinMode(LED_FONCT_V,OUTPUT); // LED RVB Fonct. V
  pinMode(LED_FONCT_B,OUTPUT); // LED RVB Fonct. B
  pinMode(LED_ALERTE_LED,OUTPUT); // LED Rouge Alerte
  /* PARAMETRAGE BROCHES COMMANDES */
  pinMode(PWM_ECLAIRAGE, OUTPUT); // PWM Eclairage
  pinMode(PWM_VENTILATION, OUTPUT); // PWM Ventilation
  pinMode(TOR_CHAUFFAGE, OUTPUT); // TOR Chauffage
  pinMode(TOR_POMPE, OUTPUT); // TOR Pompe
  /* PARAMETRAGE BROCHES CAPTEURS */
  pinMode(ACQUISITION_THERMOMETRE, INPUT); // Digital A1 Thermomètre

  digitalWrite(LED_ALERTE_LED, HIGH);
}

/*
 * FONCTIONS D'INFORMATIONS
 */
/* FONCTION POUR COMMANDER LE VOYANT DU PH */
void ledPh(bool r, bool v, bool b){
  digitalWrite(LED_PH_R, !r);
  digitalWrite(LED_PH_V, !v);
  digitalWrite(LED_PH_B, !b);
}
/* FONCTION POUR COMMANDER LE VOYANT INDICATIF DE L'ETAT DE FONCTIONNEMENT */
void ledFonctionnement(bool r, bool v, bool b){
  digitalWrite(LED_FONCT_R, !r);
  digitalWrite(LED_FONCT_V, !v);
  digitalWrite(LED_FONCT_B, !b);
}
/* FONCTION POUR COMMANDER LE VOYANT DE LA TEMPERATURE */
void ledTemperature(int etat){
  digitalWrite(LED_TEMPERATURE_LED, !etat);
}
/* FONCTION POUR COMMANDER LE VOYANT D'ALERTE */
void ledAlerte(int etat){
  digitalWrite(LED_ALERTE_LED, !etat);
}
/* FONCTION POUR COMMUNIQUER AVEC LE RASPBERRY */
void communication_raspberry(){
  String chaineLue;
  
  Serial.print(temperature);
  Serial.print("|");
  Serial.print(etatChauffage);
  Serial.print("|");
  Serial.print(etatVentilation);
  Serial.print("|");
  Serial.print(luminosite);
  Serial.print("|");
  Serial.print(luminositeArtificielle);
  Serial.print("|");
  Serial.print(ph);
  Serial.print("|");
  Serial.print(pompe);
  
  Serial.println();

  
  while (!Serial.available()){
    ledAlerte(1);  
  }
    ledAlerte(0);
  
  while (Serial.available()) {
    delay(2);  //delay to allow buffer to fill
    if (Serial.available() >0) {
      char c = Serial.read();  //gets one byte from serial buffer
      chaineLue += c; //makes the string readString
    }
  }

  if (chaineLue.length() >0) {
    char str[50]; 
    chaineLue.toCharArray(str, 50);
    int cnt = 0;
    
    char* tab[10] = { 0 };
 
    char *pch = strtok(str, "|");
 
    while ( pch != NULL ) {
        if (cnt < 10) {
          tab[cnt++] = pch;
        } else {
          break;
        }
    pch = strtok (NULL, "|");
    }
    
  tempsNutrition = atol(tab[0]);
  tempsPause = atol(tab[1]);
  paliersPh[0] = (float)atoi(tab[2])/100;
  paliersPh[1] = (float)atoi(tab[3])/100;
  luminositeRequise = atoi(tab[4]);
  paliersTemperature[0] = (float)atoi(tab[5])/100;
  paliersTemperature[1] = (float)atoi(tab[6])/100;
  paliersTemperature[2] = (float)atoi(tab[8])/100;
  paliersTemperature[3] = (float)atoi(tab[7])/100;
  }
}
/*
 * FONCTIONS D'ACTION
 */

/* ACTION PH */
void actionPh(){

  if(ph == 0){
    ledPh(0, 0, 0);
  }else{
    if(ph >= paliersPh[0]){
      if(ph <= paliersPh[1]){
        ledPh(0, 1, 0);
      }else{
        ledPh(0, 0, 1);
      }
    }else{
    ledPh(1, 0, 0);
    }
  }
}
/* ACTION LUMINOSITE */
void actionLuminosite(){
  
  if((luminosite < luminositeRequise) && (luminositeArtificielle < 255)){
    luminositeArtificielle++;
  }else if((luminosite > luminositeRequise) && (luminositeArtificielle > 0)){
    luminositeArtificielle--;
  }
  
  analogWrite(PWM_ECLAIRAGE, luminositeArtificielle);
}
/* ACTION TEMPERATURE */
void actionTemperature(){
  bool etatLedAlerteTemperature = 0;
  
  if(temperature < paliersTemperature[0]){
    etatChauffage = 1;
    etatVentilation = 128;
    etatSystTemp = 1;
  }else if((temperature > paliersTemperature[1]) && (etatSystTemp == 1)){
    etatChauffage = 0;
    etatSystTemp = 0;
  }

  if(temperature > paliersTemperature[3]){
    etatChauffage = 0;
    etatVentilation = 255;
    etatLedAlerteTemperature = 1;
    etatSystTemp = 2;
  }else if((temperature < paliersTemperature[2]) && (etatSystTemp == 2)){
    etatSystTemp = 0;
  }

  if(etatSystTemp == 0){etatVentilation = 0;etatLedAlerteTemperature=0;}


  ledTemperature(etatLedAlerteTemperature);
  digitalWrite(TOR_CHAUFFAGE, etatChauffage);
  analogWrite(PWM_VENTILATION, etatVentilation);
}
/* ACTION CYCLE NUTRITIONNEL */
void actionCycle(){
  if((positionSequentielle) <= (tempsPause/2)/tempsSequentiel){
    pompe = 0;
  }else if((positionSequentielle) <= (tempsPause/2+tempsNutrition)/tempsSequentiel){
    pompe = 1;
  }else if((positionSequentielle) > (tempsPause/2+tempsNutrition)/tempsSequentiel){
    pompe = 0;
  }
  digitalWrite(TOR_POMPE, pompe);
}

/*
 * FONCTIONS DE CAPTURE
 */
/* CAPTEUR LUMINOSITE */
void capteurLuminosite(){
  luminosite = 1024-analogRead(ACQUISITION_LUXMETRE);
}

/* CAPTEUR TEMPERATURE */
void capteurTemperature(){
  byte data[9], addr[8];
  // data : Données lues depuis le scratchpad
  // addr : adresse du module 1-Wire détecté

  while(!ds.search(addr)){ // Recherche un module 1-Wire
    ds.reset_search(); // Réinitialise la recherche de module
  }

  if(OneWire::crc8(addr, 7) == addr[7]){ // Vérifie que l'adresse a été correctement reçue
    if (addr[0] == 0x28){ // Vérifie qu'il s'agit bien d'un DS18B20
      ds.reset();             // On reset le bus 1-Wire
      ds.select(addr);        // On sélectionne le DS18B20

      ds.write(0x44, 1);      // On lance une prise de mesure de temperatureérature
      
      delay(tempsCaptureTemperature);    // Et on attend la fin de la mesure
      positionSequentielle=positionSequentielle+((int)tempsCaptureTemperature/tempsSequentiel); //Déplacement du curseur temporel de 3 blocs

      ds.reset();             // On reset le bus 1-Wire
      ds.select(addr);        // On sélectionne le DS18B20
      ds.write(0xBE);         // On envoie une demande de lecture du scratchpad

      for(byte i = 0; i < 9; i++) // On lit le scratchpad
        data[i] = ds.read();       // Et on stock les octets reçus

      // Calcul de la température en degré Celsius
      temperature = ((data[1] << 8) | data[0]) * 0.0625;
    }
  }
}

/* CAPTEUR PH */
void capteurPh(){
  int buf[10],temp;
  
  for(int i=0;i<10;i++)       //On prend 10 valeurs pour moyenner la mesure
  { 
    buf[i]=analogRead(ACQUISITION_PHMETRE);
  }
  for(int i=0;i<9;i++)        //On tri les valeurs de la plus petite à la plus grande
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  unsigned long int avgValue=0;//Store the average value of the sensor feedback
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  ph=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  ph=3.5*ph;                      //convert the millivolt into pH value
}

/*
 * FONCTIONS PRINCIPALE
 */
void loop() {
  if(analogRead(RASPI_READY) >= 512){
    if(positionSequentielle & 1){ledFonctionnement(0, 1, 1);}else{ledFonctionnement(0, 1, 0);}
    if(((positionSequentielle) % (intervalleCommunicationRPI/tempsSequentiel)) == 0){communication_raspberry();} 
  }else{
    if(positionSequentielle & 1){ledFonctionnement(1, 1, 0);}else{ledFonctionnement(0, 1, 0);}
  }
  
  actionPh();
  actionLuminosite();
  actionTemperature();
  actionCycle();
    
  capteurPh();
  capteurLuminosite();
  if(((positionSequentielle) % (intervalleCaptureTemperature/tempsSequentiel)) == 0){capteurTemperature();}  

  if(analogRead(RASPI_READY) >= 512){
    delay(tempsSequentiel-42);
  }else{
    delay(tempsSequentiel);
  }
  
  positionSequentielle++;
  if(positionSequentielle > (tempsNutrition+tempsPause)/tempsSequentiel)positionSequentielle=1;
}

