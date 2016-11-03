#include <SPI.h>
#include <ESP8266WiFi.h>
#include <TaskScheduler.h>

char ssid[] = "ssid";          //  your network SSID (name)
char pass[] = "passwd";      // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

const int water=3;
const int ventilator=1;
int counterHourly=0;

void processHttpRequests();
void processAutomation();

Task taskHttpRequests(100, TASK_FOREVER, &processHttpRequests);
Task taskAutomation(3600000, TASK_FOREVER, &processAutomation);

Scheduler runner;             // runner

void setup() {
  Serial.println("Initializing...");
  
  Serial.begin(115200);       // initialize serial communication

  pinMode(water, OUTPUT);
  pinMode(ventilator, OUTPUT);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

//  String fv = WiFi.firmwareVersion();
//  if (fv != "1.1.0") {
//    Serial.println("Please upgrade the firmware");
//  }

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status

  runner.init();                            // initialized scheduler
  runner.addTask(taskHttpRequests);         // adding task taskHttpRequests
  taskHttpRequests.enable();
  runner.addTask(taskAutomation);           // adding task taskAutomation
  taskAutomation.enable();
}


void loop() {
  runner.execute();
}

void processAutomation() {
  if (counterHourly == 0) {
    digitalWrite(water, LOW);               // turns the water on
    delay(10000);
    digitalWrite(water, HIGH);              // turns the water off
    counterHourly++;
  } else if (counterHourly == 23) {
    counterHourly = 0;
  } else 
    counterHourly++;

  if ((counterHourly % 2) == 0) {            // each 2 hours
    digitalWrite(ventilator, LOW);           // turns the ventilator on
    delay(60000);
    digitalWrite(ventilator, HIGH);          // turns the ventilator off
  }
}

void processHttpRequests() {
    WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("<h1>Ochando's Garden Automation</h1>");
            client.print("<p>Ventilator <a href=\"/ventilatorOn\"><button>ON</button></a>&nbsp;<a href=\"/ventilatorOff\"><button>OFF</button></a></p>");
            client.print("<br>");
            client.print("<p>Water <a href=\"/waterOn\"><button>ON</button></a>&nbsp;<a href=\"/waterOff\"><button>OFF</button></a></p>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request 
        if (currentLine.endsWith("GET /ventilatorOn")) {
          digitalWrite(ventilator, LOW);               // GET /ventilatorOn turns the ventilator on
        }
        if (currentLine.endsWith("GET /ventilatorOff")) {
          digitalWrite(ventilator, HIGH);                // GET /ventilatorOff turns the ventilator off
        }
        if (currentLine.endsWith("GET /waterOn")) {
          digitalWrite(water, LOW);               // GET /waterOn turns the water on
        }
        if (currentLine.endsWith("GET /waterOff")) {
          digitalWrite(water, HIGH);                // GET /waterOff turns the water off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
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
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
