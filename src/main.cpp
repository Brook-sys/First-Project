/*
 * Interface TFT_eSPI - Layout Simples e Funcional
 * Usando APENAS recursos nativos do TFT_eSPI
 * 
 * Compatível com: T-Display, TTGO LoRa32, qualquer ESP32 + TFT_eSPI
 */

#include <TFT_eSPI.h>
#include <Button.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include "NimBLEDevice.h"

#define BTN_LEFT 0
#define BTN_RIGHT 35
#define BACKLIGHT 4 

TFT_eSPI tft = TFT_eSPI();

String opts[] = {
 "Ver texto Pre-estabelecido",
 "Ver redes Wifi",
 "Desligar Backlight",
 "Entrar em modo sono profundo",
 "Ver dispositivos Bluetooth"
};

int opt_current = 0;
int prev_opt = 1;

int backoff = LOW;

void simpleText(String txt){
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(txt,120,67,2);
}
/*
void menuWifi(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("WiFi Networks:", 10, 25, 2);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    tft.drawString("Nenhuma rede", 10, 50, 2);
  } else {
    int maxRedes = (n > 5) ? 5 : n;
    
    for (int i = 0; i < maxRedes; i++) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      int force = map(rssi, -100, -30, 0, 100);
      
      // Truncar SSID se muito longo
      if (ssid.length() > 18) {
        ssid = ssid.substring(0, 15) + "...";
      }
      
      String linha = String(i+1) + ". " + ssid + " (" + force + "%)";
      tft.drawString(linha, 10, 45 + (i * 15), 1);
    }
    
    if (n > 5) {
      tft.drawString("+" + String(n-5) + " mais", 10, 115, 1);
    }
  }
  WiFi.mode(WIFI_OFF);
}
*/

Button btn1(BTN_LEFT);
Button btn2(BTN_RIGHT);

void setup()
{
  pinMode(BTN_RIGHT,INPUT_PULLUP);
  pinMode(BTN_LEFT,INPUT_PULLUP);
  pinMode(BACKLIGHT, OUTPUT);
  tft.init();
  tft.setRotation(1);
}


void loop()
{
  if (!digitalRead(BACKLIGHT)){
    if(btn1.pressed() || btn2.pressed() ){
      digitalWrite(BACKLIGHT,HIGH);
    }
    return;
  }

  if(prev_opt != opt_current){
    simpleText(opts[opt_current]);
    prev_opt = opt_current;
  }
  if(btn1.pressed()){
    opt_current++;
    int qty_opts = sizeof(opts) / sizeof(opts[0]);
    opt_current = opt_current % qty_opts;
  }
  if(btn2.pressed()){

    switch (opt_current)
    {
    case 0:
      simpleText("PUPUPU");
      break;
    case 1:
      //menuWifi();
      break;
    case 2:
      digitalWrite(BACKLIGHT,LOW);
      break;
    case 3:
      simpleText("Iniciando modo Sono");
      
      delay(1500);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
      esp_deep_sleep_start();
      break;
    case 4:
      //menuBLE();
      break;
    default:
      break;
    }
  }
  
}

