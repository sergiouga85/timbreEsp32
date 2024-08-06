#include <Arduino.h>
#include <FS.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiManager.h>          // Incluimos la librería WiFiManager
#include <WiFi.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <EEPROM.h>


WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4);//crear un objeto lcd (DIRECCIÓN pantalla, Tamaño x, Tamño y)

// Function declarations
void handleLogin();
void handleRoot();
void handleGuardar();
void handleToggle();
void obtenerDatosEEPROM();
void Actualizar_Display();
void activar_timbre();

#define timbre 4 // GPIO 4
#define led 14 // GPIO 17

int hora;
int minuto;
int segundo;
int dia_semana;
int tiempo_timbre;
int dataActivadores[16];
int dataTime[16];
int dataDias[16];
int dataHoras[16];
int dataMinutos[16];
int dataSegundos[16];
int lunes[16];
int martes[16];
int miercoles[16];
int jueves[16];
int viernes[16];
int sabado[16];
int domingo[16];
int dataTimbres[16];

int pos_mem = 0;

bool isAuthenticated = false; // Variable global para estado de autenticación

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.south-america.pool.ntp.org", -10800, 60000);
char daysOfTheWeek[7][12] = {"PTESI", "SALI", "CRSMB", "PRSMB", "CUMA", "CTESI", "PAZAR"};


// Crear un objeto JSON para analizar los 
JsonDocument jsonDoc;


void setup() {

  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();// Coloca luz de fondo al LCD                      // Coloca luz de fondo al LCD
  lcd.clear();

  pinMode(timbre, OUTPUT);
  pinMode(led, OUTPUT);

  ///  EEPROM ///

  EEPROM.begin(256);  //Tamaño de la eeprom, 250 direcciones de memoria

  //////////// Actualizo los valores de los vectores con los valores almacenados en la EEPROM (Ultimo guardado) //////////


  for (int i = 0; i < 16; i++)
  {
    dataActivadores[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    dataHoras[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    dataMinutos[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    dataSegundos[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    lunes[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    martes[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    miercoles[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    jueves[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    viernes[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    sabado[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    domingo[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  for (int i = 0; i < 16; i++)
  {
    dataTimbres[i] = EEPROM.read(pos_mem);
    pos_mem++;
  }

  pos_mem = 0; //reseteo pos_mem para un futuro guardado de horarios



  //Mensaje de bienvenida

  lcd.setCursor(0, 0);
  lcd.print("   JS Electronica & desarrollo IoT  ");
  lcd.setCursor(0, 1);
  lcd.print(" Sist. Electronicos");
  lcd.setCursor(0, 2);
  lcd.print(" Control de Horario");
  lcd.setCursor(0, 3);
  lcd.print("Timbres v4.0 (WiFi)");

  // Creamos una instancia de WiFiManager
  WiFiManager wifiManager;

  lcd.setCursor(0, 0);
  lcd.print("conectarse a la red");
  lcd.setCursor(0, 1);
  lcd.print("     WifiManager   ");

  // Uncomment the following line for initial setup
  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect("WifiManager")) {
    Serial.println("Fallo en la conexion (timeout)");
    lcd.setCursor(0, 0);
    lcd.print(" Falla conexion ");
    lcd.setCursor(0, 1);
    lcd.print("     Time out   ");
    lcd.print("timeout");
    digitalWrite(4, HIGH);
    delay(250);
    digitalWrite(4, LOW);
    delay(250);
    ESP.restart();
  }

  delay(3000);
  Serial.println("");
  Serial.println("WiFi conectada  ");       // Indica que el NodeMCU conectado con la red WiFi
  lcd.setCursor(0, 0);
  lcd.print("   WiFi conectada  ");
  server.begin();                         // Inicia el NodeMCU en modo Station
  Serial.println("Servidor inicializado");
  lcd.setCursor(0, 1);
  lcd.print("      Servidor     ");
  lcd.setCursor(0, 2);
  lcd.print("    inicializado   ");
  delay(4000);

  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  // Asigna la IP estática

  Serial.println(WiFi.localIP());

  // Inicializa SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  // Ruta para el archivo de inicio
 server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/guardar", HTTP_POST, handleGuardar);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/datos_eeprom", HTTP_GET, obtenerDatosEEPROM);



  // Iniciamos el servidor web
  server.begin();
  WiFi.mode(WIFI_STA);
  // Initialize a NTPClient to get time
  timeClient.begin();
}

void loop() {

  server.handleClient();

  if ( WiFi.status() == WL_CONNECTED )
  {
    timeClient.update();
  }

  dia_semana = timeClient.getDay();
  hora = timeClient.getHours();
  minuto = timeClient.getMinutes();
  segundo = timeClient.getSeconds();
  Actualizar_Display();

  for (int i = 0; i < 16; i++)
  {
    if (dataActivadores[i] == 1 and dataHoras[i] == hora and dataMinutos[i] == minuto and dataSegundos[i] == segundo and ((lunes[i] == 1 and dia_semana == 1) or (martes[i] == 1 and dia_semana == 2) or (miercoles[i] == 1 and dia_semana == 3) or (jueves[i] == 1 and dia_semana == 4) or (viernes[i] == 1 and dia_semana == 5) or (sabado[i] == 1 and dia_semana == 6) or (domingo[i] == 1 and dia_semana == 0)))
    {
      tiempo_timbre = dataTimbres[i];
      activar_timbre();
    }
  }
}

void obtenerDatosEEPROM() {
  // Crear un objeto JSON para almacenar los datos
 
 DynamicJsonDocument jsonDoc(1024);

  // Crear un array para almacenar los datos de la EEPROM
  JsonArray dataArray = jsonDoc.to<JsonArray>();

  // Leer datos de la EEPROM y agregarlos al array JSON
  for (int i = 0; i < 192; i++) { // Iterar sobre todos los datos necesarios
    dataArray.add(EEPROM.read(i));
  }

  // Convertir el JSON a una cadena de texto
  String output;
  serializeJson(dataArray, output);

  // Enviar la cadena de texto JSON como respuesta al cliente
  server.send(200, "application/json", output);
}

void handleLogin() {
  String username = server.arg("username");
  String password = server.arg("password");

  if (username.equals("admin") && password.equals("12345678")) {
    isAuthenticated = true;
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(401, "text/plain", "Autenticación fallida");
  }
}


void handleRoot() {
  if (!isAuthenticated) {
    File file = SPIFFS.open("/login.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Error al abrir login.html");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  } else {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Error opening index.html");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  }
}

void handleToggle() {

  lcd.setCursor(0, 1);
  lcd.print("      Timbre      ");
  lcd.setCursor(0, 2);
  lcd.print("     Activado     ");

  digitalWrite(timbre, HIGH);
  digitalWrite(led, HIGH);
  delay(500); // Puedes ajustar este tiempo según tus necesidades
  digitalWrite(led, LOW);
  digitalWrite(timbre, LOW);
  server.send(200, "text/plain", "Toggle LED");
}



void handleGuardar() {

  if (server.method() == HTTP_POST) {

    const String jsonString = server.arg("plain");
    server.send(200, "text/plain", "Horario guardado");

    Serial.print(jsonString);
    DeserializationError error = deserializeJson(jsonDoc, jsonString);

    // Verificar errores de análisis JSON
    if (error) {
      Serial.print("Error al analizar JSON: ");
      Serial.println(error.c_str());
      return;
    }

    lcd.setCursor(0, 3);
    lcd.print("  Horario cargado ");
    Serial.print("Horario Cargado");
    delay(1000);


    JsonArray jsonActivadores = jsonDoc["dataActivadores"].as<JsonArray>();
    JsonArray jsonTime = jsonDoc["dataTime"].as<JsonArray>();
    JsonArray jsonLunes = jsonDoc["dataDias"]["lunes"].as<JsonArray>();
    JsonArray jsonMartes = jsonDoc["dataDias"]["martes"].as<JsonArray>();
    JsonArray jsonMiercoles = jsonDoc["dataDias"]["miercoles"].as<JsonArray>();
    JsonArray jsonJueves = jsonDoc["dataDias"]["jueves"].as<JsonArray>();
    JsonArray jsonViernes = jsonDoc["dataDias"]["viernes"].as<JsonArray>();
    JsonArray jsonSabado = jsonDoc["dataDias"]["sabado"].as<JsonArray>();
    JsonArray jsonDomingo = jsonDoc["dataDias"]["domingo"].as<JsonArray>();
    JsonArray jsonDias = jsonDoc["dataDias"].as<JsonArray>();
    JsonArray jsonTimbres = jsonDoc["dataTimbre"].as<JsonArray>();

    // Copiar los valores del JsonArray al array de enteros
    for (int i = 0; i < 16; i++) {
      String dataTime = jsonTime[i];
      dataActivadores[i] = jsonActivadores[i].as<int>();
      dataHoras[i] = dataTime.substring(0, 2).toInt();
      dataMinutos[i] = dataTime.substring(3, 5).toInt();
      dataSegundos[i] = dataTime.substring(6, 8).toInt();
      lunes[i] = jsonLunes[i].as<int>();
      martes[i] = jsonMartes[i].as<int>();
      miercoles[i] = jsonMiercoles[i].as<int>();
      jueves[i] = jsonJueves[i].as<int>();
      viernes[i] = jsonViernes[i].as<int>();
      sabado[i] = jsonSabado[i].as<int>();
      domingo[i] = jsonDomingo[i].as<int>();
      dataTimbres[i] = jsonTimbres[i].as<int>();
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, dataActivadores[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, dataHoras[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, dataMinutos[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, dataSegundos[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, lunes[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, martes[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, miercoles[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, jueves[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, viernes[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, sabado[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, domingo[i]);
      pos_mem++;
    }

    for (int i = 0; i < 16; i++)
    {
      EEPROM.write(pos_mem, dataTimbres[i]);
      pos_mem++;
    }

    pos_mem = 0; //reseteo pos_mem para un futuro guardado de horarios
    EEPROM.commit(); // Confirmar los cambios


    for (int i = 0; i < 16; i++) {

      Serial.println(" ");
      Serial.println("Datos de dias y horarios ingresados en HTML: ");
      Serial.println(" ");
      Serial.print("Activadores: ");
      Serial.print(dataActivadores[i]);
      Serial.print(" Horario: ");
      Serial.print(dataHoras[i]);
      Serial.print(":");
      Serial.print(dataMinutos[i]);
      Serial.print(":");
      Serial.print(dataSegundos[i]);
      Serial.print(" Lunes: ");
      Serial.print(lunes[i]);
      Serial.print(" Martes: ");
      Serial.print(martes[i]);
      Serial.print(" Miercoles: ");
      Serial.print(miercoles[i]);
      Serial.print(" Jueves: ");
      Serial.print(jueves[i]);
      Serial.print(" viernes: ");
      Serial.print(viernes[i]);
      Serial.print(" Sabados: ");
      Serial.print(sabado[i]);
      Serial.print(" Domingos: ");
      Serial.print(domingo[i]);
      Serial.print(" Tiempo de timbrado: ");
      Serial.print(dataTimbres[i]);
      Serial.println(" seg");
    }
  }
}

void activar_timbre()
{

  digitalWrite(timbre, HIGH);
  digitalWrite(led, HIGH);
  Serial.print("Timbre Activado");
  lcd.setCursor(0, 1);
  lcd.print("      Timbre      ");
  lcd.setCursor(0, 2);
  lcd.print("     Activado     ");
  delay(tiempo_timbre * 1000);
  digitalWrite(timbre, LOW);
  digitalWrite(led, LOW);

  if (tiempo_timbre <= 999)
  {
    delay(1000);
  }

  lcd.clear();
}

void Actualizar_Display()
{

  lcd.setCursor(0, 0);
  lcd.print("      Standby      ");
  lcd.setCursor(0, 3);
  lcd.print("  IP: ");
  lcd.setCursor(5, 3);
  lcd.print(WiFi.localIP());
  lcd.setCursor(5, 3);
  lcd.setCursor(0, 1);
  lcd.print("  Fecha:");
  lcd.setCursor(0, 2);
  lcd.print("  Hora:");


  if ( WiFi.status() == WL_CONNECTED )
  {
    timeClient.update();
  }


  time_t epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;
  String currentDate = String(monthDay) + "/" + String(currentMonth) + "/" + String(currentYear);

  lcd.setCursor(8, 1);
  if (monthDay < 10) lcd.print("0");
  lcd.print(monthDay, DEC);
  lcd.print("/");
  if (currentMonth < 10) lcd.print("0");
  lcd.print(currentMonth, DEC);
  lcd.print("/");
  lcd.print(currentYear, DEC);
  lcd.print(" ");

  lcd.setCursor(7, 2);
  if (timeClient.getHours() < 10) lcd.print("0");
  lcd.print(timeClient.getHours(), DEC);
  lcd.print(":");
  if (timeClient.getMinutes() < 10) lcd.print("0");
  lcd.print(timeClient.getMinutes(), DEC);
  lcd.print(":");
  if (timeClient.getSeconds() < 10) lcd.print("0");
  lcd.print(timeClient.getSeconds(), DEC);
  lcd.print(" ");

  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(":");
  Serial.print(currentSecond);
  Serial.println("   ");
  Serial.print(monthDay);
  Serial.print("/");
  Serial.print(currentMonth);
  Serial.print("/");
  Serial.println(currentYear);

}