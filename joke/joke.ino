#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

const char* ssid = "iPhone";//numele retelei
const char* password = "vlad";//parola retelei
//definirra pinilor
#define BTN_PIN 22//buton 
//pini ai display-ului
#define TFT_DC 2
#define TFT_CS 5
#define TFT_RST 4
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);//obiectul tft controleaza pinii display-ului

const string url = "https://v2.jokeapi.dev/joke/Programming";//URL cu glume

//structura pentru preluare de glume
string getJoke() {
  HTTPClient http;//creez obiectul http
  http.useHTTP10(true);
  http.begin(url);//acceseaza adresa
  http.GET();//se trimite cererea pentru gluma
  string result = http.getString();//json

  DynamicJsonDocument doc(2048);//rezerv spatiu de 2048 bytes pentru documentul json numit doc
  DeserializationError error = deserializeJson(doc, result);//deserializarea pachetului

  if (error) //daca deserializarea nu a reusit
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "<error>";
  }
  //transform labelurile doc-ului in variabile string
  string type = doc["type"].as<String>();
  string joke = doc["joke"].as<String>();
  string setup = doc["setup"].as<String>();
  string delivery = doc["delivery"].as<String>();
  http.end();//inchide conexiunea cu serverul

  // verific daca tipul glumei este "single" (o singura linie)
if (type.equals("single")) {
    // daca este true, returnam doar textul glumei
    return joke;
} 
else {
    // daca este fals, înseamna ca e o gluma din doua parti ("twopart")
    // lipim întrebarea (setup) cu raspunsul (delivery)
    String glumaCompleta = setup + "  " + delivery;
    return glumaCompleta;
}
}

void nextJoke() {
  // curat ecrannul si setez cursorul de la inceput
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);

  tft.setTextColor(ILI9341_WHITE);
  tft.println("\nLoading joke...");

  // verific daca exista conexiune si se concteaza esp ul
  if (WiFi.status() != WL_CONNECTED) {
    tft.setTextColor(ILI9341_RED);
    tft.println("No WiFi connection!");
    Serial.println("No WiFi connection!");
    return;
  }

  string joke = getJoke();//se preia gluma cu ajutorul structurii getJoke()
  tft.setTextColor(ILI9341_GREEN);
  tft.println(joke);
}

void setup() {
  Serial.begin(115200);//conexiune cu seriala la viteza de 115200
  pinMode(BTN_PIN, INPUT_PULLUP);//setup buton cu IN

  WiFi.begin(ssid, password);//numele si parola de conectare

  tft.begin();//porneste ecranul
  tft.setRotation(1);//roteste imaginea o data
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Connecting to WiFi");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 50) //am setat 50 de incercari ca sa se opreasca bucla dupa 25 de secunde sa nu mearga la infinit
  {
    //apare pe ecran un punct la fiecare jumatate de secunda si creste contorul pentru conectare
    //practic punctele ne spun de cate secunde se incearca conectarea
    delay(500);
    tft.print(".");
    Serial.print(".");
    i++;
  }

  if (WiFi.status() == WL_CONNECTED)//daca s-a conectat
   {
    tft.print("\nOK! IP=");
    tft.println(WiFi.localIP());//adresa retelei de WiFi
    Serial.print("\nConnected! IP: ");
    Serial.println(WiFi.localIP());
  } 
  else//incercare nereusita dupa 25s 
  {
    tft.println("\nFailed to connect!");
    Serial.println("\nFailed to connect!");
  }

  nextJoke();
}

void loop() {
  if (digitalRead(BTN_PIN) == LOW) {
    //cand se apasa butonul apare gluma
    delay(200); // debounce mic pentru buton
    nextJoke();
  }
  delay(100);//pauza scurta intre glume
}
