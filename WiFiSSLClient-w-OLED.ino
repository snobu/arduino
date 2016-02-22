#include <SPI.h>
#include <Adafruit_WINC1500.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Define the WINC1500 board connections below.
// If you're following the Adafruit WINC1500 board
// guide you don't need to modify these:

// SPI for Mega2560
#define WINC_CS   53
#define WINC_IRQ  7
#define WINC_RST  6
#define WINC_EN   2     // or, tie EN to VCC

// The SPI pins of the WINC1500 (SCK, MOSI, MISO) should be
// connected to the hardware SPI port of the Arduino.
// On an Uno or compatible these are SCK = #13, MISO = #12, MOSI = #11.
// On an Arduino Zero use the 6-pin ICSP header, see:
//   https://www.arduino.cc/en/Reference/SPI

// Setup the WINC1500 connection with the pins above and the default hardware SPI.
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);

// Or just use hardware SPI (SCK/MOSI/MISO) and defaults, SS -> #10, INT -> #7, RST -> #5, EN -> 3-5V
//Adafruit_WINC1500 WiFi;

char ssid[] = "notandroid"; //  your network SSID (name)
char pass[] = "lup12345";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(10,105,110,156);  // numeric IP for Google (no DNS)
char server[] = "thefailchoochoo.azurewebsites.net";

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
Adafruit_WINC1500SSLClient client;

unsigned long lastConnectionTime = 0;            
const unsigned long pollingInterval = 5L * 1000L; // 5 sec polling delay, in milliseconds

void setup() {
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
display.clearDisplay();
display.display();

#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif

  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("ATWINC1500 not present");
    // don't continue:
    while (true);
  }

  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    if (pass) {
      Serial.print("Using WPA");
      status = WiFi.begin(ssid, pass);
    }
    else {
      Serial.print("No authentication");
      status = WiFi.begin(ssid);
    }

    // wait 10 seconds for connection:
    delay(15000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

 
  display.setRotation(2);
  display.setTextColor(WHITE);
      
} //end setup


void loop() {
  String response = "EMPTY";
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    //Serial.write(c);
    response.concat(c);
  }

  if (!response.equals("EMPTY")) {
    Serial.println("\n---- start response ----");
    Serial.print(response);
    Serial.println("\n---- end response ----");
      
    if (response.startsWith("HTTP/1.1 200 OK")) {
      // write 200 OK to OLED
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(3);
      display.print("200 OK");
      display.display();
    }
    else {
      // write FAIL to OLED
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(3);
      display.print("FAIL");
      display.display();
    }
  }
  
  // polling..if pollingInterval has passed
  if (millis() - lastConnectionTime > pollingInterval) {
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      Serial.println();
      Serial.println("Disconnecting from server.");
      client.stop();
    }
    // Clear OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Connecting...");
    display.display();
    Serial.println("inside if millis: calling httpRequest from loop()");
    //response = "EMPTY";
    httpRequest();
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void httpRequest() {
  client.stop();
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET / HTTP/1.1");
    client.println("Host: thefailchoochoo.azurewebsites.net");
    client.println("User-Agent: Atmel ATWINC1500");
    client.println("Connection: close");
    client.println();
  }
  // note the time that the connection was made:
  lastConnectionTime = millis();
}
