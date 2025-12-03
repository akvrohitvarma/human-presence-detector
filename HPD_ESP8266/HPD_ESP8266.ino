/**
 * Project: SenseWave Human/Obstacle Detector (Wi-Fi Tripwire)
 * Mode: WIFI_STA (Connects to Phone)
 * 
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>       
#include <ESPAsyncWebServer.h> 

// --- CRITICAL CONFIGURATION ---
// Update these to match your Phone's Hotspot exactly
const char* ssid = "kumara";    
const char* password = "123456789";   

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sensewave Presence</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; background-color: #111; color: #fff; text-align: center; padding: 20px; display: flex; flex-direction: column; align-items: center; min-height: 100vh; }
        
        h1 { color: #888; font-size: 1.2rem; margin-bottom: 30px; letter-spacing: 1px; }
        
        .status-box {
            width: 90%;
            padding: 40px 20px;
            border-radius: 15px;
            background-color: #222;
            margin-bottom: 30px;
            transition: background-color 0.3s, transform 0.2s;
            border: 2px solid #333;
        }
        
        .status-text { font-size: 2rem; font-weight: bold; display: block; margin-bottom: 10px; }
        .rssi-subtext { font-size: 1rem; color: #aaa; }

        .btn {
            padding: 15px 40px;
            font-size: 1.2rem;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            box-shadow: 0 4px 15px rgba(0,123,255,0.4);
            transition: transform 0.1s;
        }
        .btn:active { transform: scale(0.95); }
        .btn:disabled { background-color: #555; cursor: not-allowed; box-shadow: none; }

        .state-safe { background-color: #1a4d1a; border-color: #2f8c2f; box-shadow: 0 0 20px #1a4d1a; }
        .state-danger { background-color: #4d1a1a; border-color: #ff3333; transform: scale(1.05); box-shadow: 0 0 30px #ff0000; }
        .state-calibrating { background-color: #4d4d1a; border-color: #ffff00; }
        
    </style>
</head>
<body>

    <h1>Human Obstacle Detector <br> Team 3</h1>

    <!-- Visual Alert Box -->
    <div id="alert-box" class="status-box">
        <span id="main-status" class="status-text">System Idle</span>
        <span id="data-readout" class="rssi-subtext">Current: -- dBm</span>
    </div>

    <!-- Control Button -->
    <button id="arm-btn" class="btn" onclick="startCalibration()">Arm System</button>

    <script>
        // CONFIGURATION
        const REFRESH_RATE = 500;   // Check every 0.5s for faster detection
        const DROP_THRESHOLD = 8;   // If signal drops 8dBm below baseline, trigger alarm
        
        // VARIABLES
        let baselineRSSI = null;
        let isMonitoring = false;
        let isCalibrating = false;
        let calibrationReadings = [];
        
        // DOM ELEMENTS
        const alertBox = document.getElementById('alert-box');
        const mainStatus = document.getElementById('main-status');
        const dataReadout = document.getElementById('data-readout');
        const btn = document.getElementById('arm-btn');

        // Main Loop
        setInterval(() => {
            fetch('/rssi_data')
            .then(r => r.json())
            .then(data => {
                const currentRSSI = data.rssi;
                
                // Always update the small text
                let diff = (baselineRSSI !== null) ? (baselineRSSI - currentRSSI) : 0;
                let diffText = (isMonitoring) ? `(Drop: ${diff} dB)` : "";
                dataReadout.innerText = `Current: ${currentRSSI} dBm ${diffText}`;

                // Logic Flow
                if (isCalibrating) {
                    // Collect samples
                    calibrationReadings.push(currentRSSI);
                } 
                else if (isMonitoring) {
                    detectObstacle(currentRSSI);
                }
            })
            .catch(e => console.log(e));
        }, REFRESH_RATE);

        // Function 1: Start Calibration (5 seconds)
        function startCalibration() {
            isCalibrating = true;
            isMonitoring = false;
            calibrationReadings = [];
            
            // UI Updates
            btn.disabled = true;
            btn.innerText = "Calibrating...";
            mainStatus.innerText = "Measuring Environment...";
            alertBox.className = "status-box state-calibrating";

            // Run for 4 seconds, then calculate average
            setTimeout(() => {
                finishCalibration();
            }, 4000);
        }

        // Function 2: Finish Calibration & Start Monitoring
        function finishCalibration() {
            isCalibrating = false;
            
            // Calculate Average
            let sum = calibrationReadings.reduce((a, b) => a + b, 0);
            baselineRSSI = Math.round(sum / calibrationReadings.length);
            
            // Start Monitoring
            isMonitoring = true;
            
            // UI Updates
            btn.innerText = "System Armed"; // Keep disabled so they don't click again accidentally
            mainStatus.innerText = "Path Clear";
            alertBox.className = "status-box state-safe";
            
            console.log("Baseline Set:", baselineRSSI);
        }

        // Function 3: The Detection Logic
        function detectObstacle(current) {
            // How much signal have we lost?
            // Example: Baseline -50, Current -65. Drop = 15.
            let signalDrop = baselineRSSI - current;

            if (signalDrop >= DROP_THRESHOLD) {
                // ALARM STATE
                mainStatus.innerText = "OBSTACLE DETECTED";
                mainStatus.style.color = "#fff";
                alertBox.className = "status-box state-danger";
            } else {
                // CLEAR STATE
                mainStatus.innerText = "Path Clear";
                alertBox.className = "status-box state-safe";
            }
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n--- SenseWave System Arming ---");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // LED Status
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW); delay(100); digitalWrite(LED_BUILTIN, HIGH); delay(400);
  }

  Serial.println("CONNECTED!");
  Serial.print("IP ADDRESS: "); Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/rssi_data", HTTP_GET, [](AsyncWebServerRequest *request){
    int rssi = WiFi.RSSI();
    String json = "{\"rssi\": " + String(rssi) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {}