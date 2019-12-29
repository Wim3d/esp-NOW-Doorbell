/*
   Doorbell via ESP-NOW
   Written by W. Hoogervorst, december 2019
*/

extern "C" {
#include <espnow.h>
}
// MAC address of the ESP with which it is paired (slave), global defined since used in setup and loop
uint8_t mac_addr[6] = {0x6A, 0xC6, 0x3A, 0xC4, 0xA7, 0x5C}; // MAC address of access point

const char* topic = "sensor/doorbell";   // MQTT topic to publish to
const char* payload = "RING";            // MQTT payload

uint8_t result = 1;

void setup()
{
  // Initialize the ESP-NOW protocol, if it fails, the module is restarted
  if (esp_now_init() != 0) {
    //Serial.println("*** ESP_Now init failed");
    ESP.restart();
    delay(1);
  }
  // display  MAC DATA of this of ESP on serial for debug
  // *** DECLARATION OF THE ROLE OF THE ESP DEVICE IN THE COMMUNICATION *** //
  // 0 = LOOSE, 1 = MASTER, 2 = SLAVE and 3 = MASTER + SLAVE
  esp_now_set_self_role(1);   // sender

  // *** PAIRING WITH THE SLAVE *** //
  uint8_t role = 2;   // role of receiver = slave
  uint8_t channel = 1;  // WiFi channel of receiver access point
  esp_now_add_peer(mac_addr, role, channel, NULL, 0);   // NULL means there is no key, length of key is 0

  // set up the call-back function for the confirmation of the sent data. This is executed is ESP-NOW is used further on in the program
  esp_now_register_send_cb([](uint8_t *mac,  uint8_t result2) {
    char MACslave[6];
    // in this call back function the result is stored in the "result" variable, which is important in the program
    result = result2;
    //sprintf(MACslave, "% 02X:% 02X:% 02X:% 02X:% 02X:% 02X", mac [0], mac [1], mac [2], mac [3], mac [4], mac [5]);

    // display result on serial if serial is initialized
    //Serial.print("Data sent to ESP MAC:"); Serial.print(MACslave);
    //Serial.print(". Reception(0 = 0K - 1 = ERROR):"); Serial.println(result);
  });
}

void loop()
{
  // prepare the data to send
  char DATA[40];
  strcat(DATA, topic);   // Copy the topic to the array.
  strcat(DATA, "&");   // Copy "&" symbol to the array.
  strcat(DATA, payload);   // Copy payload to the array.
  //Serial.print("DATA: "); Serial.print(DATA);

  uint8_t data[sizeof(DATA)];
  memcpy(data, &DATA, sizeof(DATA));
  uint8_t   len = sizeof(data);

  //Serial.print("char data: "); Serial.println("%s", data);
  //send the data
  esp_now_send(mac_addr, data, len);

  yield();
  delay(200); //wait for the confirmation of the slave and the execution of the call back function
  if (result == 0)
  {
    delay(100);
    ESP.deepSleep(0);
    delay(100);
  }
}
