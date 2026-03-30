#include <Arduino.h>
#include <ESP_NOW.h>
#include "WiFi.h"

uint8_t controllerAddress[6];
bool controllerAdded = false;
bool paired = false;
bool pair_mode = false;

// put function declarations here:

enum Command
{
  LookingPeers,
  LookingHost,
  Connected,
  Heartbeat,
  Disconnect
};

enum SweepCmds
{
  Hard,
  Clean,
  Left,
  Right,
  Stop,
  None
};

typedef struct
{
  Command command;
  uint8_t addr[6];
} ConnectMessage;

typedef struct
{
  uint8_t command;
} MainMessage;

esp_now_peer_info peerInfo;

void addControllerPeer()
{
  if (controllerAdded)
  {
    return;
  }

  esp_now_peer_info_t controllerPeer = {};
  memcpy(controllerPeer.peer_addr, controllerAddress, 6);
  controllerPeer.channel = 1;
  controllerPeer.encrypt = false;
  controllerPeer.ifidx = WIFI_IF_STA;

  esp_err_t result = esp_now_add_peer(&controllerPeer);

  if (result == ESP_OK || result == ESP_ERR_ESPNOW_EXIST)
  {
    controllerAdded = true;
    Serial.println("Controller added");
  }
}

void handleCommand(int cmd)
{
  // test print
  Serial.print("CMD RECEIVED: ");
  Serial.println(cmd);

  if (!(cmd < 6 && cmd >= 0)) {
    return;
  }
  
  // handle command
}

void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len)
{
  if (len == sizeof(ConnectMessage))
  {
    ConnectMessage msg;
    memcpy(&msg, incomingData, sizeof(msg));
    if (!paired && msg.command == LookingPeers)
    {
      memcpy(controllerAddress, recv_info->src_addr, 6);
      Serial.println("pair request received");
      addControllerPeer();

      ConnectMessage reply;
      reply.command = LookingHost;
      WiFi.macAddress(reply.addr);

      esp_err_t result = esp_now_send(controllerAddress, (uint8_t *)&reply, sizeof(reply));
      if (result == ESP_OK)
      {
        paired = true;
        Serial.println("paired success");
      }
      else
      {
        Serial.println("pair reply send failed");
      }
    }
  }
  else if (len == sizeof(MainMessage))
  {
    MainMessage mmsg;
    memcpy(&mmsg, incomingData, sizeof(mmsg));

    if (paired)
    {
      handleCommand(mmsg.command);
    }
  }
  else
  {
    Serial.println("Unknow packet");
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.mode(WIFI_MODE_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(50);
}