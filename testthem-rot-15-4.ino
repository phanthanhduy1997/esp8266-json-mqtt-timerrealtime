
/*
  Basic ESP8266 MQTT example
  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.
  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.
  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/
const int ledsever = D2 ;//khai bao den led sever
const int ledwifi = D3 ;//khai bao den led wifi
const int  outputpin = A0; //Khai bao chan doc nhiet do
const int  Relay = D8 ;   //Khai bao chan output vao relay

bool is_set_alarm_cycle = false;
bool is_set_alarm_date_time = false;

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266TimeAlarms.h>
#include <string.h>
// Update these with values suitable for your network.

const char* ssid = "My Friend";
const char* password = "khongcopass";
const char* mqtt_server = "149.28.141.58";
AlarmId id;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastMsg2 = 0;
unsigned long lastMsg3 = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int mode = 0;
float nhietmax = 0;
float nhietmin = 0;
bool chedo = true;
float celsius = 0;
int solan = 0;
int value = 0;
int timer = 0;
char json[100] = "{\"mode\":1,\"limit\":{\"min\":1235,\"max\":1234,\"mode_on\":false}}";   //      {"mode":2,"limit":{"min":20,"max":33,"mode_on":true}}
//       {"runtime":4,"restime":2,"applydatetime":19-15,"repest":3}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  configTime(0, 0, "0.se.pool.ntp.org");
  //Europe/Stockholm": "CET-1CEST,M3.5.0,M10.5.0/3"
  //Get JSON of Olson to TZ string using this code https://github.com/pgurenko/tzinfo
  setenv("TZ", "WIB-7", 1);
  tzset();
  Serial.print("Clock before sync: ");
  digitalClockDisplay();


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    Serial.print("Clock after Wifi: ");

    // create the alarms, to trigger at specific times
    //                  Alarm.alarmRepeat(8,30,0, MorningAlarm);  // 8:30am every day
    //                  Alarm.alarmRepeat(17,45,0,EveningAlarm);  // 5:45pm every day
    //                  Alarm.alarmRepeat(dowSaturday,8,30,30,WeeklyAlarm);  // 8:30:30 every Saturday
    //
    //                  // create timers, to trigger relative to when they're created
    //                  Alarm.timerRepeat(15, Repeats);           // timer for every 15 seconds
    //                  id = Alarm.timerRepeat(2, Repeats2);      // timer for every 2 seconds
    //                  Alarm.timerOnce(10, OnceOnly);            // called once after 10 seconds
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ( (*topic) = "setting/192168110" && payload[0] == '{' )
  {

    char* json = (char*) payload;
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    mode = root["mode"];
    float min = root["limit"]["min"];
    float max = root["limit"]["max"];
    bool mode_on = root["limit"]["mode_on"];

    Serial.println(mode);
    Serial.println(min);
    Serial.println(max);
    Serial.println(mode_on);

    switch (mode) {
      case 1:
        Serial.println("bat che do THU CONG");

        break;
      case 2:
        Serial.println("bat che do TU DONG");

        chedo = mode_on;
        nhietmax = max;
        nhietmin = min;
        Serial.println(nhietmax);
        Serial.println(nhietmin);
        break;
      case 3:
        is_set_alarm_cycle = false;
        Serial.println("bat che do CYCLE TIME");


        break;
      case 4:
        is_set_alarm_date_time = false;
        Serial.println("bat che do REAL TIME");

        break;
    }
  }
  if ( mode == 1  && payload[0] == '1') //neu topic nhan dc la turn_on/192168110 va msg la 1 thi sang
  {

    digitalWrite(Relay, HIGH);
    client.publish("turned_on/192168110", "1");

  }
  if ( mode == 1  && payload[0] == '0') { //neu topic nhan dc la turn_on/192168110 va msg la 1 thi tat

    digitalWrite(Relay, LOW);
    client.publish("turned_on/192168110", "0");

  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(ledwifi, LOW);                     //neu mat ket noi thi den tat
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("connected/192168110", "1");    //neu ket noi dc thi gui len sever connected/192168110 [1]
      Serial.println("da ket noi duoc voi sever");                //neu ket noi dc thi gui len sever connected/192168110 [1]
      // Once connected, publish an announcement...

      // ... and resubscribe
      client.subscribe("setting/192168110");
      client.subscribe("turn_on/192168110");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(Relay, OUTPUT);           // Khai bao chan output cua relay
  pinMode(ledsever, OUTPUT);     //Khai bao led sever
  pinMode(ledwifi, OUTPUT);     //Khai bao led wifi
  digitalWrite(Relay, LOW);        //  Relay on 5V
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.subscribe("setting/192168110");
  client.subscribe("turn_on/192168110");

}

void loop() {



  if (!client.connected()) {
    digitalWrite(ledsever, LOW);
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  unsigned long now2 = millis();
  unsigned long now3 = millis();

  // set cho ham cycle time
  if (now - lastMsg > 3000) {
    lastMsg = now;
    ++value;
    int analogValue = analogRead(outputpin);
    float millivolts = (analogValue / 1024.0) * 3300; //3300 is the voltage provided by NodeMCU
    float nearest = roundf(millivolts * 100) / 100;
    celsius = nearest / 10;

    snprintf (msg, MSG_BUFFER_SIZE, "%.2f", celsius);
    Serial.print("Publish message: ");
    Serial.println(msg);
    Serial.println(celsius);
    client.publish("sensor/192168110", msg);
    digitalWrite(ledsever, HIGH);
    digitalWrite(ledwifi, HIGH);
  }


  if (mode == 2) {
    if (chedo == false)
    {
      if (celsius > nhietmax || celsius < nhietmin )
      { digitalWrite(Relay, HIGH);
        client.publish("turned_on/192168110", "1");
      }
      else {
        digitalWrite(Relay, LOW);
        client.publish("turned_on/192168110", "0");
      }
    } if (chedo == true)
    {
      if (celsius < nhietmax && celsius > nhietmin )
      { digitalWrite(Relay, HIGH);
        client.publish("turned_on/192168110", "1");
      }
      else {
        digitalWrite(Relay, LOW);
        client.publish("turned_on/192168110", "0");
      }
    }
  }

  if (mode == 3 && !is_set_alarm_cycle)
  {
    is_set_alarm_cycle = true;
    Alarm.alarmRepeat(19, 32, 0, RunCycleAlarm);
  }

  if (mode == 4 && !is_set_alarm_date_time)
  {
    is_set_alarm_date_time = true;
    //    Serial.println("chuan bi thuc hien lenh real time");
    //    Alarm.alarmRepeat(11, 56, 0, MorningAlarm);
    //    Alarm.alarmRepeat(11, 57, 30, EveningAlarm);
    Alarm.timerOnce(10, RunCycleAlarm);
    digitalClockDisplay();
    Alarm.delay(1000); // wait one second between clock display

  }
}
  void RunCycleAlarm() {
    Serial.println("Alarm:         Bat");
    digitalWrite(Relay, HIGH);
    Alarm.timerOnce(10, StopCycleAlarm);

  }
  void StopCycleAlarm() {
    Serial.println("Alarm:        Tat");
    digitalWrite(Relay, LOW);
    Alarm.timerOnce(5, RunCycleAlarm);
  }

  void MorningAlarm() {
    Serial.println("Alarm:         HOAT DONG");

  }

  void EveningAlarm() {
    Serial.println("Alarm: - turn lights on");
    digitalWrite(Relay, LOW);
    client.publish("turned_on/192168110", "0");
  }

  void WeeklyAlarm() {
    Serial.println("Alarm: - its Monday Morning");
  }

  void ExplicitAlarm() {
    Serial.println("Alarm: - this triggers only at the given date and time");
  }

  void Repeats() {
    Serial.println("15 second timer");
  }

  void Repeats2() {
    Serial.println("2 second timer");
  }

  void OnceOnly() {
    Serial.println("This timer only triggers once, stop the 2 second timer");
    // use Alarm.free() to disable a timer and recycle its memory.
    Alarm.free(id);
    // optional, but safest to "forget" the ID after memory recycled
    id = dtINVALID_ALARM_ID;
    // you can also use Alarm.disable() to turn the timer off, but keep
    // it in memory, to turn back on later with Alarm.enable().
  }

  void digitalClockDisplay() {
    time_t tnow = time(nullptr);
    Serial.println(tnow);

      Serial.println(ctime(&tnow));

//    Serial.println(ctime(&tnow));

  }
