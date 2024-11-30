#define LED_PIN 2
#define OPEN_DOOR_PIN 15
#include <WiFi.h>
#include <BluetoothSerial.h>
#include <Preferences.h>
#include <FirebaseESP32.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>


//time
unsigned long interval = 6UL * 60 * 60 * 1000; // 6 horas en milisegundos
unsigned long previousMillis = 0;


// Credenciales por defecto (en caso de que no haya ninguna almacenada)
String ssid = "";
String password = "";
bool streamIniciado = false;
bool isReconectedWifi = false;

unsigned long lastAttemptTime = 0;
const unsigned long reconnectionDelay = 5000; // 5 segundos de espera


/* 2. Define the API Key */
#define API_KEY "AIzaSyBVfS68bzD0AREp8yUarOxFPcvev19LDu4"

/* 3. Define the RTDB URL */
#define DATABASE_URL "porterogirasol-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "juanescorcia94@gmail.com"
#define USER_PASSWORD "portero_automatico_2024"


#define FIREBASE_PROJECT_ID "porterogirasol"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-ydnat@porterogirasol.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC6s1I/0YXuQ+IC
c4pINqBc7ENUSaY6xOD43Mzb7CYoeFtoU9q9Hy1NVNkhb7Zjn6AdIPXfcEeQfEHL
/f/pYtldaizNmbgeakWD0LjJjCyUSlWK6cexkwpv7oF13N2meV05hvShAC1wofPX
kUP9X9X8tQfn6Kp5q7N+pIYsKI3/H2dkWK6el7LPCkyC1nShWW9aU+Ul4qGoIDjI
7ee9EYu3jefXJ16IcEsXdGJrwmKYugjHLJg4gkDOKtTJoEXjKy8y+7BIu2WnP+5/
XEl0+feuKoClPSlsxEDLuSeeMOVnABH55tx25wrETkhQSTIv3hYPurJ8Rby5U5eX
wd6squKbAgMBAAECggEAWHKOgxRYPi3jmuSwvJBAfXUiWoJ/oTTXHZ7A4GkqFJ8c
MrsTE+h6tjl9mfUJ831SRapxQNa42I69Pw5IWAEP18zuh8KMssAGybEucbEaB3CF
wQcKpynxVmCn7TXivbSeQRwZtVJXtJXM5rF3g8gMoKkEo02awYRhxxvbQn098//V
WNuWFBq3UPluo31Ic9XOt5kJN6WtBKCI9V7h9qwoMw7ctOAY7damaz++zRS/IB1h
iAzZZzI8hC+X+llvP8UHZhp6G+TBlf9HVG56DFxs0jBfC650AFsoK//oKj/+q2MJ
TQwEvNZAKEXOFNlSa7ieilyW/VfEvcn07AXw8wPGCQKBgQDspvxMRFJwDoiFSYzp
acEHrLGost+kJUE6mbgd43LEEVOK9OlZ1NLBZlZdzuqxExGGQV2gpj5YvWXhKNup
jY44czxcrYZPj+FJ9wkKN1WZM7ozfluEtQWXgVxpeydevO4MJyeN9xvxJWW91FzK
uc7NPEYdCvhZn/4bjkWLWI0ROQKBgQDJ9t6Cjbcp7XNjli5Sbx0hmeNRnzynQNZM
IiR1PCocumAL3+PKTL2ZHJUtK1PLMPcapbcTihryEV7PX5Fn3wZq2x+LfaHStSFx
Qr1N1idlS9rCo/1FuUXi2K6PjqBbMgYDAKrmpjFC0Z9IENyImWUpoB05gr5Nr2MC
aKwJtJtWcwKBgClYe1fvz99YCq4OzZmyKlFm8JsitUP+ZkkQtkQyisiKpmfuph5V
uoSjlo9JKWPKixNDtFHu+ZGTdYtDEi2rV5+xMmiWwJKB9sBvcprgVAoyKWP5vY+P
+OwhY73iEFzS8VwaMyTweWdGO2JRCe05TVI6J53HXuB5vXGfB7NywyKxAoGBAJR3
XzEX3or63DVcJGdC+WZHCx1ocpz9A8vp2WU5OjfuJkb0ai6/5SzWHy+aRzn6n9w9
7+x0PMSwUkMdnPV/tOb+eB0B2ODdeF0bU7ARetpzPbiR9tC3lxFgekpb7bO8fUDd
z8RIbbQSBwZiJsxf/0bvRiyN2TmgcadGrRlhZEPTAoGBAIbuVlM2XPVOmHlMUUHP
FdxRVttSnkgZwZS5ow5h25B42j42CDsdMPiNVdKJqZgTIQLKlw2emivejBGkgmyw
HllmzAEP5Ps0ksdxixw/FpQiHlYdhABjltI/uzczZidWqF5ngVomD8Mm/EnjFdKI
Xv+ZFoiS1sW44V3LgL9lt5R4
-----END PRIVATE KEY-----
)EOF";

// Define Firebase Data object
FirebaseData fbdo;
FirebaseData stream;

FirebaseAuth auth;
FirebaseConfig config;

// Bluetooth Serial
BluetoothSerial SerialBT;

// Para guardar las credenciales
Preferences preferences;

// Función para conectar al Wi-Fi
void conectarWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(ssid);
  
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado a Wi-Fi.");
    Serial.println("IP Address: " + WiFi.localIP().toString());
    parpadear(2);
    finalizarFirebase();
    delay(5000);
    conectarFirebase();
  } else {
    Serial.println("\nError: No se pudo conectar al Wi-Fi.");
    parpadear(3);
  }
}

// Función para guardar las credenciales en la memoria
void guardarCredenciales(String newSSID, String newPassword) {
  preferences.begin("wifiCreds", false);
  preferences.putString("ssid", newSSID);
  preferences.putString("password", newPassword);
  preferences.end();
  Serial.println("Credenciales guardadas en memoria.");
}

void parpadear(int parpadeos){
  int intentos = 0;
  while (intentos < parpadeos) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    intentos++;
  }
}

// Función para cargar las credenciales de la memoria
void cargarCredenciales() {
  preferences.begin("wifiCreds", false);
  ssid = preferences.getString("ssid", "Alejandra");
  password = preferences.getString("password", "mLemou12");
  preferences.end();
}

// Función para recibir nuevas credenciales vía Bluetooth
void recibirCredencialesPorBT() {
  if (SerialBT.available()) {
    digitalWrite(LED_PIN, HIGH);
    String data = SerialBT.readStringUntil('\n');
    data.trim(); // Eliminar espacios en blanco

    int separador = data.indexOf(',');
    if (separador != -1) {
      String newSSID = data.substring(0, separador);
      String newPassword = data.substring(separador + 1);

      Serial.println("Recibido por Bluetooth:");
      Serial.println("SSID: " + newSSID);
      Serial.println("Password: " + newPassword);

      // Guardar las nuevas credenciales
      guardarCredenciales(newSSID, newPassword);

      // Cambiar a las nuevas credenciales y reconectar
      ssid = newSSID;
      password = newPassword;

      // Intentar reconectar con las nuevas credenciales
      WiFi.disconnect();
      delay(1000);  // Esperar antes de reconectar
      isReconectedWifi = true;
      conectarWiFi();
    }
    digitalWrite(LED_PIN, LOW);
  }
}

void conectarFirebase(){
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  /* Assign the sevice account credentials and private key (required) */
  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  config.service_account.data.private_key = PRIVATE_KEY;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);
  Firebase.reconnectWiFi(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 512 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  // You can use TCP KeepAlive For more reliable stream operation and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP32#enable-tcp-keepalive-for-reliable-http-streaming
  stream.keepAlive(15, 15, 1);

  iniciaEscucha();

  streamIniciado = true;  // Ahora el stream está activo

  Serial.println("Firebase inicializado y stream iniciado.");
}

void reconnectToFirebase() {
  int retryCount = 0;
  const int maxRetries = 5;
  int delayTime = 1000; // Comienza con 1 segundo de retraso

  while (!Firebase.ready() && retryCount < maxRetries) {
      Serial.printf("Reintentando conectar Firebase (Intento %d)...\n", retryCount + 1);
      finalizarFirebase();
      delay(delayTime);
      conectarFirebase();  // Reintenta la conexión
      retryCount++;
      delayTime *= 2;  // Incrementa el tiempo de espera de manera exponencial
  }
}

// Función de callback para manejar eventos de cambio en Firebase
void streamCallback(StreamData data) {
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());

  // Aquí puedes procesar los cambios de acuerdo a tus necesidades
  if (data.dataPath() == "/opendoor") {
    if (data.dataType() == "string") {

      // Leer el valor en formato de cadena de texto
      String valorString = stream.stringData();  // Obtener la cadena del stream
      Serial.println("Cadena recibida: " + valorString);

      // Dividir la cadena usando el delimitador ','
      int comaIndex = valorString.indexOf(',');  // Encontrar la posición de la coma
      if (comaIndex != -1) {
        // Obtener la primera parte antes de la coma (time)
        String timeStr = valorString.substring(0, comaIndex);
        int timeValue = timeStr.toInt();  // Convertir el valor a número entero

        // Obtener la segunda parte después de la coma (timestamp)
        String timestampStr = valorString.substring(comaIndex + 1);
        long timestampValue = timestampStr.toInt();  // Convertir a número entero

        // Imprimir los valores recibidos
        Serial.println("Time: " + String(timeValue));
        Serial.println("Timestamp: " + String(timestampValue));

        // Aquí puedes implementar tu lógica con timeValue y timestampValue
        digitalWrite(OPEN_DOOR_PIN, LOW);
        delay(timeValue);
        digitalWrite(OPEN_DOOR_PIN, HIGH);
      } else {
        Serial.println("Error: Formato de cadena incorrecto.");
      }
    } else {
      Serial.println("Error: El dato recibido no es una cadena de texto.");
    }
  }
  
  //Enviar notificacion de disponibilidad de firebase.
  if (isReconectedWifi && data.dataType() == "json" && data.dataPath() == "/"){
    isReconectedWifi = false;
    sendMessage();
  }
}

// Función de callback en caso de error en la escucha de eventos
void streamTimeoutCallback(bool timeout) {
    if (timeout && millis() - lastAttemptTime > reconnectionDelay) {
        Serial.println("Timeout en el stream, intentando reconectar...");
        lastAttemptTime = millis();
        reconnectToFirebase();
    }
}

// Iniciar la escucha de eventos en Firebase
void iniciaEscucha(){
  if (Firebase.beginStream(stream, "/command")) {
    Serial.println("Escuchando eventos en /command...");
    Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);
  } else {
    Serial.println("Error al iniciar la escucha de eventos: " + stream.errorReason());
  }
}

void sendMessage() {
  if (Firebase.ready()) {
    Serial.print("Send Firebase Cloud Messaging... ");

    // Read more details about HTTP v1 API here https://firebase.google.com/docs/reference/fcm/rest/v1/projects.messages
    FCM_HTTPv1_JSON_Message msg;

    msg.topic = "myTopic"; // Topic name to send a message to, e.g. "weather". Note: "/topics/" prefix should not be provided.
    msg.notification.body = "Notification body";
    msg.notification.title = "Notification title";

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson payload;

    // all data key-values should be string
    payload.add("temp", "28");
    payload.add("unit", "celsius");
    payload.add("timestamp", "1609815454");
    msg.data = payload.raw();

    // send message to recipient
    if (Firebase.FCM.send(&fbdo, &msg)) {
      Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}

void setup() {
  pinMode(OPEN_DOOR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(OPEN_DOOR_PIN, HIGH);

  Serial.begin(115200);
  SerialBT.begin("DOOR_BT");  // Nombre del dispositivo Bluetooth
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Cargar credenciales de la memoria
  cargarCredenciales();

  // Intentar conectar con las credenciales almacenadas
  isReconectedWifi = true;
  conectarWiFi();
}


void finalizarFirebase() {
  // Detiene cualquier flujo de datos activo
  if (streamIniciado) {
    Firebase.endStream(stream);
    Serial.println("Stream finalizado.");
    streamIniciado = false;
  }

  Serial.println("Firebase finalizado.");
}


void loop() {
    unsigned long currentMillis = millis();

     // Verifica si ha pasado el tiempo especificado
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Reinicia el ESP32
      Serial.println("Reiniciando el ESP32...");
      parpadear(4);
      esp_restart(); // Reinicia el microcontrolador
    }

    // Escuchar por nuevas credenciales a través de Bluetooth
    recibirCredencialesPorBT();

    // Verifica la conexión de Wi-Fi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Conexión Wi-Fi perdida, intentando reconectar...");
        conectarWiFi();
    }

    // Verifica si Firebase está listo
    if (WiFi.status() == WL_CONNECTED && !Firebase.ready()) {
        Serial.println("Firebase no está listo. Intentando reconectar...");
        reconnectToFirebase();
    }

    // Verifica si el stream sigue activo, si no lo está, intenta reconectar
    if (!stream.httpConnected() && streamIniciado) {
        Serial.println("Stream desconectado. Intentando reconectar el stream...");
        reconnectToFirebase();
        delay(5000);  // Espera antes de intentar reconectar nuevamente
    }

}
