const char* host = "api.timezonedb.com";

unsigned long getTimeTimer = 0;
int currentTime = 0;

void processNetwork() {
  // Get the time every 5 seconds
  if (millis() > getTimeTimer)
  {
    Serial.println("Getting time...");
    getTime();
    getTimeTimer = millis() + 600000;
  }
}

void getTime() {
  WiFiClient client;
  StaticJsonBuffer<200> jsonBuffer;
  char json[200];

  Serial.printf("\n[Connecting to % s ... ", host);
  if (client.connect(host, 80))
  {
    Serial.println("connected]");

    Serial.println("[Sending a request]");
    client.print(String("GET /v2/get-time-zone?key=WJW2Z4K0DWHF&format=json&by=zone&zone=PST&fields=timestamp,dst") + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n"
                 "Content-Type: text/html" +
                 "Connection: close\r\n" +
                 "\r\n"
                );
    Serial.println("[Response:]");
    while (client.connected())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        if (line[0] == '{') {
          Serial.println(line);
          line.toCharArray(json, 200);
        }
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");

    JsonObject& root = jsonBuffer.parseObject(json);
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    Serial.println(root["timestamp"].as<long>());
    long time = root["timestamp"].as<long>();
    currentTime = time / 3600 % 24;
    Serial.print("DST: "); Serial.println(root["dst"].as<int>());
    if(!root["dst"].as<int>())
      currentTime++;
    if (currentTime < nightTimeEnd || currentTime >= nightTimeStart)
      nightMode = true;
    //  nightMode = false;
    else
      nightMode = false;
    //  nightMode = true;

    Serial.println("Timestamp : " + String(time));
    Serial.println("Current time in hours : " + String(currentTime));
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
}
