function doGet(e) {
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    var uid = e.parameter.uid; // Mengambil parameter uid
    var name = e.parameter.name; // Mengambil parameter name
    var timestamp = new Date(); // Mendapatkan timestamp saat ini
  
    // Menambahkan baris baru ke spreadsheet dengan uid, name, dan timestamp
    sheet.appendRow([uid, name, timestamp]);
    
    // Mengembalikan respons
    return ContentService.createTextOutput("Data diterima: " + uid + ", " + name);
  }