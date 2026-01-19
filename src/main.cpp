/*
 * Interface TFT_eSPI - Layout Simples e Funcional
 * Usando APENAS recursos nativos do TFT_eSPI
 * 
 * Compatível com: T-Display, TTGO LoRa32, qualquer ESP32 + TFT_eSPI
 */

#include <TFT_eSPI.h>
#include <Button.h>
#include <esp_sleep.h>
#include "NimBLEDevice.h"

#define BTN_LEFT 0
#define BTN_RIGHT 35
#define BACKLIGHT 4 

TFT_eSPI tft = TFT_eSPI();

String opts[] = {
 "Deixar BLE visivel, 10 segundos",
 "Enviar comando BLE",
 "Desligar Backlight",
 "Entrar em modo sono profundo",
 "Ver dispositivos Bluetooth"
};

int opt_current = 0;
int prev_opt = 1;


int backoff = LOW;

NimBLEService *service= NULL;
NimBLEServer *server= NULL;
NimBLECharacteristic *feedbackEsp= NULL;
NimBLECharacteristic *feedbackMaster= NULL;
NimBLEAdvertising *advertising = NULL;

const char *uuidService = "25e29492-2f4b-4e3d-8eb8-e06bdac72f90";

String bleConn = "Off";
String bleListen = "Off";
String bleAdvertising = "On";
String msgFeedbackMaster = "top";
boolean bleAdvertisingScreen = false;
int tst = 1;

volatile boolean needsRefresh = false;
boolean needsPostStatus = false;
String statusContent = "";

void postStatus(String status){
  needsPostStatus = true;
  statusContent = String(status);
}

void simpleText(String txt,int x=120,int y=67,int font=2,uint8_t datum = MC_DATUM,uint16_t txtColor=TFT_WHITE){
  tft.setTextDatum(datum);
  tft.setTextColor(txtColor);
  tft.drawString(txt,x,y,font);
}

void refreshHeader(){
  tft.fillRect(0,0,240,20,TFT_BLACK);
  uint16_t txtColor;
  if (bleConn=="On"){
    txtColor = TFT_GREEN;
  }else{
    txtColor = TFT_RED;
  }
  simpleText(bleConn,5,2,2,TL_DATUM,txtColor);

  if (bleListen=="On"){
    txtColor = TFT_GREEN;
  }else{
    txtColor = TFT_YELLOW;
  }
  simpleText(bleListen,35,2,2,TL_DATUM,txtColor);

  if(bleAdvertising=="On"){
    txtColor = TFT_YELLOW;
  }else if(bleAdvertising=="Off"){
    txtColor = TFT_RED;
  }else{
    txtColor = TFT_BLACK;
  }
  simpleText(bleAdvertising,65,2,2,TL_DATUM,txtColor);


  simpleText(msgFeedbackMaster,230,2,2,TR_DATUM);

  Serial.println("header atualizado");
  needsRefresh=false;
}

void simplescreenText(String txt,boolean fill=true){
  if(fill){
    tft.fillScreen(TFT_BLACK);
  }
  simpleText(txt);
  refreshHeader();
}

void menuDeepSleep(){
  simplescreenText("Entrando em modo sono profundo");
  delay(1000);
  simplescreenText("3");
  delay(1000);
  simplescreenText("2");
  delay(1000);
  simplescreenText("1");
  delay(1000);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
  esp_deep_sleep_start();
}

void menuSendCommand(){
  if(bleConn=="On"){
    tst++;
    feedbackEsp->setValue(tst);
    feedbackEsp->notify();
  }else{
    simplescreenText("Sem conexão :(");
  }
}


boolean postInit = false;
int secs = 0;
long tempInit = 0;
void menuAdvertisingEnable(){
  if(bleConn=="On"){
    bleAdvertisingScreen=false;
    postInit = false;
    secs = 0;
    simplescreenText("Conectado :)");
    return;
  }
  if(!postInit){
    tempInit = millis();
    postInit=true;
    if(!advertising->isAdvertising()){
      advertising->start();
      bleAdvertising = "On";
    }
  }
  if(millis() - tempInit >= secs*1000){
    simplescreenText(String(10-secs));
    secs++;
  }
  if (secs>=10){
    bleAdvertisingScreen=false;
    bleAdvertising = "Off";
    postInit = false;
    secs = 0;
    simplescreenText("Terminado tempo.");
    advertising->stop();
  }
  

}


class ServerCallBack : public NimBLEServerCallbacks {
  public:
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        bleConn = "On";
        
        bleAdvertising = "Connected";
        needsRefresh=true;
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        bleConn = "Off";
        bleAdvertising = "Off";
        needsRefresh=true;  
    }
  
};

class ReceivedCallback : public NimBLECharacteristicCallbacks {

  public:
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInf) override {
      msgFeedbackMaster = pCharacteristic->getValue();
      postStatus("Ok enviado");
      needsRefresh=true;
    }
};

class SendedCallback : public NimBLECharacteristicCallbacks {
  public:
    void onStatus(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, int code) override{
      if(code==0){
        postStatus("tst: " + String(tst));
      }
    }
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
      if(subValue ==0){
        bleListen = "Off";
      }else{
        bleListen = "On";
      }
      needsRefresh=true;
      
    }
};




Button btn1(BTN_LEFT);
Button btn2(BTN_RIGHT);

void setup()
{
  Serial.begin(9600);  
  pinMode(BTN_RIGHT,INPUT_PULLUP);
  pinMode(BTN_LEFT,INPUT_PULLUP);
  pinMode(BACKLIGHT, OUTPUT);
  tft.init();
  tft.setRotation(1);

  NimBLEDevice::init("EspSmar");
  server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallBack);
  service = server->createService(uuidService);
  feedbackEsp = service->createCharacteristic("1985ddce-73f4-43af-b90c-0815206e41e0",NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  feedbackMaster = service->createCharacteristic("63972162-8635-40fe-9358-b33fd129010e",NIMBLE_PROPERTY::WRITE|NIMBLE_PROPERTY::WRITE_NR);
  feedbackMaster->setCallbacks(new ReceivedCallback);
  feedbackEsp->setCallbacks(new SendedCallback);
  service->start();

  advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(uuidService);
  advertising->setName("EspSmar");
  advertising->setAdvertisingCompleteCallback([](NimBLEAdvertising *padv){
    Serial.println("Advertising parado");
    bleAdvertising = "Off";
  });
  advertising->start();
}

void loop()
{
  if (!digitalRead(BACKLIGHT)){
    if(btn1.pressed() || btn2.pressed() ){
      digitalWrite(BACKLIGHT,HIGH);
    }
    return;
  }

  if(bleAdvertisingScreen){
    menuAdvertisingEnable();
    return;
  }
  if(needsRefresh){
    refreshHeader();
  }
  if(needsPostStatus){
    simplescreenText(statusContent);
    needsPostStatus = false;
  }

  if(prev_opt != opt_current){
    simplescreenText(opts[opt_current]);
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
      bleAdvertisingScreen = true;
      break;
    case 1:
      menuSendCommand();
      break;
    case 2:
      digitalWrite(BACKLIGHT,LOW);
      break;
    case 3:
      menuDeepSleep();
      break;
    case 4:
      //menuBLE();
      break;
    default:
      break;
    }
  }
  
}

