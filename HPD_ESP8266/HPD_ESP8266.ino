/**
 * Project: Human Presence Detector
 * Mode: WIFI_STA (Connects to Phone)
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>       
#include <ESPAsyncWebServer.h> 

const char* ssid = "YOUR_HOTSPOT_NAME";
const char* password = "YOUR_HOTSPOT_PASSWORD";

AsyncWebServer server(80);

// --- HTML & JS (Embedded) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Human Presence Detector <br> Team 3</title>
    <style>
        body { font-family: sans-serif; background-color: #000; color: #fff; text-align: center; padding: 20px; }
        h1 { color: #888; margin-bottom: 20px; }
        
        .readout { font-size: 80px; font-weight: bold; margin: 20px 0; }
        
        .bar-container { width: 100%; height: 50px; background: #333; border-radius: 25px; overflow: hidden; margin-bottom: 20px; }
        
        .bar-fill { height: 100%; width: 0%; transition: width 0.5s ease-out, background-color 0.5s; }
        
        .status { font-size: 24px; font-weight: bold; margin-top: 20px; }
    </style>
</head>
<body>
    <h2>Human Presence Detector <br> Team 3</h2>
    
    <div class="readout"><span id="val">--</span> <span style="font-size:30px">dBm</span></div>
    
    <div class="bar-container">
        <div id="bar" class="bar-fill"></div>
    </div>
    
    <div id="stat" class="status">Connecting...</div>

    <script>
        setInterval(() => {
            fetch('/rssi_data')
            .then(response => response.json())
            .then(data => {
                const rssi = data.rssi;
                updateUI(rssi);
            })
            .catch(err => console.log(err));
        }, 1000); 

        function updateUI(rssi) {
            document.getElementById('val').innerText = rssi;
            
            // Map Signal (-90 to -30) to Percentage (0 to 100)
            let pct = (rssi - -90) * (100) / (-30 - -90);
            if(pct < 5) pct = 5; 
            if(pct > 100) pct = 100;
            
            const bar = document.getElementById('bar');
            const stat = document.getElementById('stat');
            
            bar.style.width = pct + "%";
            
            if(rssi > -50) {
                bar.style.backgroundColor = "#00ff00"; // Green
                stat.innerText = "High Power Density";
                stat.style.color = "#00ff00";
            } else if(rssi > -75) {
                bar.style.backgroundColor = "yellow";
                stat.innerText = "Attenuated";
                stat.style.color = "yellow";
            } else {
                bar.style.backgroundColor = "red";
                stat.innerText = "Blocked / Weak";
                stat.style.color = "red";
            }
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n--- Human Presence Detector (Station Mode - 1Hz) ---");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Off

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to Hotspot: ");
  Serial.println(ssid);

  // Blink while connecting
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW); // On
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH); // Off
    delay(400);
    Serial.print(".");
  }

  Serial.println("\n\n=================================");
  Serial.println("CONNECTED SUCCESSFULLY!");
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP()); 
  Serial.println("=================================\n");
  
  // Solid LED to indicate connection
  digitalWrite(LED_BUILTIN, LOW); 

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/rssi_data", HTTP_GET, [](AsyncWebServerRequest *request){

    int rssi = WiFi.RSSI();

    Serial.print("RSSI: "); Serial.println(rssi);
    
    String json = "{\"rssi\": " + String(rssi) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  // If connection drops, blink fast
  if (WiFi.status() != WL_CONNECTED) {
     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
     delay(100);
  }
}