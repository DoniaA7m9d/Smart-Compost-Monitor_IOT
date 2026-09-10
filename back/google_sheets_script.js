// Google Apps Script for Smart Compost Monitor
// يستقبل البيانات من ESP32 ويخزنها في Google Sheet

function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  var temp = e.parameter.temp;
  var status = e.parameter.status || "OK";
  var hum = e.parameter.hum || "N/A"; // للتوافق مع DHT11

  // سطر جديد: [الوقت, الحرارة, الرطوبة, الحالة]
  sheet.appendRow([new Date(), temp, hum, status]);
  
  return ContentService.createTextOutput("Data logged successfully");
}

// اختياري: دالة تمسح الداتا القديمة
function clearSheet() {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  sheet.clearContents();
  sheet.appendRow(["Timestamp", "Temperature C", "Humidity %", "Status"]);
}
