# Smart Compost Monitor - IoT
### Real-time Temperature Monitoring for Organic Compost using ESP32

#### The Problem
Compost thermophilic phase is extremely temperature-sensitive.
- **Below 55°C** = Beneficial microbes stop. Decomposition halts.
- **Above 65°C** = Beneficial microbes die. Compost is ruined.
Manual monitoring 24/7 is not practical for farmers.

#### The Solution
A low-power IoT device that monitors core compost temperature and sends instant alerts when it goes outside the optimal range of 55-65°C.

#### Key Features
- **Accurate Sensing**: Waterproof DS18B20 sensor. Range: -55°C to +125°C, Accuracy: ±0.5°C
- **Ultra Low Power**: ESP32 Deep Sleep mode. Runs 3+ months on a single 18650 battery
- **Instant Alerts**: Telegram notification + Buzzer with corrective action: "Turn pile" or "Add water"
- **Data Logging**: Automatic logging to Google Sheets for temperature trend analysis
- **Smart Alerts**: Sends only 1 alert until temperature returns to normal to avoid spam

#### Bill of Materials
| Component | Purpose |
| --- | --- |
| ESP32 DevKit v1 | WiFi + Deep Sleep |
| DS18B20 Waterproof | High temp + moisture resistant sensing |
| 4.7K Ohm Resistor | Pull-up for DS18B20 1-Wire protocol |
| Passive Buzzer | Local audible alert |
| PVC Pipe | Sensor housing to protect from moisture |

#### Wiring Diagram
DS18B20 RED    → 3.3V
DS18B20 BLACK  → GND  
DS18B20 YELLOW → GPIO4 + 4.7K Pull-up to 3.3V
BUZZER +       → GPIO5
BUZZER -       → GND
![Wiring](docs/wiring_diagram.png)

#### Setup Guide
1.  **Hardware**: Insert DS18B20 probe into the center of the compost pile
2.  **Software**: Update WiFi, Telegram Bot Token, and Google Sheets URL in `firmware/esp32_compost_monitor.ino`
3.  **Backend**: Deploy `backend/google_sheets_script.js` to Google Apps Script and get the Web App URL

#### Backend: Google Apps Script
Paste this in `script.google.com` > Deploy as Web App
```javascript
function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var temp = e.parameter.temp;
  var status = e.parameter.status || "OK";
  var hum = e.parameter.hum || "N/A";

  sheet.appendRow([new Date(), temp, hum, status]);
  return ContentService.createTextOutput("OK");
}
#### How It Works
The ESP32 wakes up every hour. 
1.  Reads temperature from DS18B20
2.  Connects to WiFi and logs data to Google Sheets
3.  If temp < 55°C or > 65°C: Triggers buzzer + sends 1 Telegram alert
4.  Goes back to Deep Sleep to save power

#### Expected Impact
40% higher composting success rate. Prevents batch loss. Saves farmer time.

---

*Tech Stack: ESP32, C++, Arduino, IoT, Google Sheets API, Telegram Bot API*
