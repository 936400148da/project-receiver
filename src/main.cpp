#include <Arduino.h>
#include <ESP_NOW.h>
#include "WiFi.h"
#include <ostream>
#include <FastLED.h>

#define LED_PIN 5
#define NUM_LEDS 10

CRGB leds[NUM_LEDS];

uint8_t controllerAddress[6];
bool controllerAdded = false;
bool paired = false;
bool pair_mode = false;

// put function declarations here:
int myFunction(int, int);

enum Command : uint8_t
{
    LookingPeers = 0,
    LookingHost,
    Connected,
    Heartbeat,
    Disconnect
};

enum SweepCmds : uint8_t
{
    Hard = 0,
    Clean,
    Left,
    Right,
    Stop
};

typedef struct
{
    uint8_t command;
    uint8_t addr[6];
} ConnectMessage;

typedef struct
{
    uint8_t command;
} MainMessage;

esp_now_peer_info peerInfo;

void addControllerPeer(){
  if(controllerAdded){
    return;
  }

  esp_now_peer_info_t controllerPeer = {};
  memcpy(controllerPeer.peer_addr, controllerAddress, 6);
  controllerPeer.channel = 0;
  controllerPeer.encrypt = false;
  controllerPeer.ifidx = WIFI_IF_STA;

  esp_err_t result = esp_now_add_peer(&controllerPeer);

  if(result == ESP_OK || result ==ESP_ERR_ESPNOW_EXIST){
    controllerAdded = true;
    Serial.println("Controller added");
  }
}

void handleCommand(int cmd){
  switch (cmd)
  {
  case Hard:
    Serial.println("HARD");
    leds[0] = CRGB::Green;
    leds[2] = CRGB::Green;
    leds[4] = CRGB::Green;
    leds[6] = CRGB::Green;
    break;
  case Clean:
    Serial.println("Clean");
    leds[1] = CRGB::Blue;
    leds[3] = CRGB::Blue;
    leds[5] = CRGB::Blue;
    leds[7] = CRGB::Blue;
    break;
  case Left:
    Serial.println("Left");
    leds[0] = CRGB::White;
    leds[1] = CRGB::White;
    leds[2] = CRGB::White;
    leds[3] = CRGB::White;
    break;
  case Right:
    Serial.println("Right");
    leds[4] = CRGB::Yellow;
    leds[5] = CRGB::Yellow;
    leds[6] = CRGB::Yellow;
    leds[7] = CRGB::Yellow;
    break;
  case Stop:
    Serial.println("Stop");
    for(int i = 0; i < NUM_LEDS; i ++){
      leds[i] = CRGB::Red;
    }
    break;
  default:
    Serial.println("UNKNOW!");
    break;
  }
}

void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len){
  if(len == sizeof(ConnectMessage)){
    ConnectMessage msg;
    memcpy(&msg, incomingData, sizeof(msg));
    if(!paired && msg.command == LookingPeers){
      memcpy(controllerAddress, recv_info -> src_addr, 6);
      Serial.println("pair request received");
      addControllerPeer();

      ConnectMessage reply;
      reply.command = LookingHost;
      WiFi.macAddress(reply.addr);

      esp_now_send(controllerAddress, (uint8_t*)&reply, sizeof(reply));
      esp_err_t result = esp_now_send(controllerAddress, (uint8_t*)&reply, sizeof(reply));
      if(result == ESP_OK){
        paired = true;
        Serial.println("paired success");
      }else{
        Serial.println("pair reply send failed");
}
    }
  }else if(len == sizeof(MainMessage)){
    MainMessage mmsg;
    memcpy(&mmsg, incomingData, sizeof(mmsg));

    if(paired){
      handleCommand(mmsg.command);
    }
  }else{
    Serial.println("Unknow packet");
  } 
}

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  Serial.println("receiver ready");
  
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}
