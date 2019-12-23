#include <ESP8266WiFi.h>

const char *ssid = "Autic";
const char *password = "espap123";

String html_code = "HTTP/1.1 200 OK\n\
Content-type:text/html\n\
\n\
<!DOCTYPE html>\n\
<html>\n\
  <head>\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
    <script>\n\
        function set_pozicija(pozicija) {\n\
          var xhttp = new XMLHttpRequest();\n\
          xhttp.open(\"GET\", \"./volan/\" + pozicija, false);\n\
          document.getElementById(\"v_volan\").innerHTML = pozicija;\n\
          xhttp.send();\n\
        }\n\
        function set_brzina(pozicija) {\n\
          var xhttp = new XMLHttpRequest();\n\
          xhttp.open(\"GET\", \"./brzina/\" + pozicija, false);\n\
          document.getElementById(\"v_brzina\").innerHTML = pozicija;\n\
          xhttp.send();\n\
        }\n\
        function set_svjetlo(jacina) {\n\
          var xhttp = new XMLHttpRequest();\n\
          xhttp.open(\"GET\", \"./farovi/\" + jacina, false);\n\
          xhttp.send();\n\
        }\n\
        function set_zmigavac(strana) {\n\
          var xhttp = new XMLHttpRequest();\n\
          xhttp.open(\"GET\", \"./zmigavac/\" + strana, false);\n\
          xhttp.send();\n\
        }\n\
    </script>\n\
  </head>";

String html_code1 = "<body>\n\
    <div style=\"width:90%; width: 90vw;\">\n\
      <p>\n\
        <input style=\"width:90%; width: 90vw; height: 5em;\" type=\"range\" id=\"volan\" value=\"90\" min=\"60\" max=\"120\"\n\
                oninput=\"set_pozicija(this.value)\" />\n\
        <label id=\"v_volan\">90</label>\n\
      </p>\n\
      <p>\n\
        <input style=\"width:90%; width: 90vw; height: 5em;\" type=\"range\" id=\"brzina\" value=\"0\" min=\"-10\" max=\"10\"\n\
                oninput=\"set_brzina(this.value)\" />\n\
        <label id=\"v_brzina\">90</label>\n\
      </p>\n\
    </div>\n\
       <p>\n\
        <input type=\"button\" value=\"Dugo svjetlo\" onclick=\"set_svjetlo(255)\" />\n\
        <input type=\"button\" value=\"Oboreno svjetlo\" onclick=\"set_svjetlo(100)\" />\n\
        <input type=\"button\" value=\"Iskljuci svjetlo\" onclick=\"set_svjetlo(0)\" />\n\
       </p>\n\
       <p>\n\
        <input type=\"button\" value=\"Zmigavac lijevo\" onclick=\"set_zmigavac(1)\" />\n\
        <input type=\"button\" value=\"Zmigavac desno\" onclick=\"set_zmigavac(0)\" />\n\
    </p>\n\
  </body>";

WiFiServer server(80);
int incomingByte = 0;
int volan_pozicija = 90;
enum komanda_tip {
  NISTA, //
  VOLAN, //
  BRZINA, //
  FAROVI, //
  ZMIGAVAC
};

String current_param;
komanda_tip tekuca_komanda = NISTA;
bool isLisening = false;

void setup() {
  setup_brzina();
  Serial.begin(74880);
  delay(1000);
  Serial.println("Staring App: ");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.print(myIP);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    switch (incomingByte) {
      case 97:
        printStatus();
        break;
      case 100:
        Serial.println(html_code);
        break;
      case 115: //s
        send_farovi(255);
        break;
      case 102: //f
        send_farovi(100);
        break;
      case 103: //g
        send_farovi(0);
        break;
    }
  }

  servo_detach();  //Iskjučuje servo ako nije potreban
  iskjuci_stop();
  zmigavac_togle();
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        // Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println(html_code);
            client.println(html_code1);
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Serial.print("Current line: ");
        // Serial.println(currentLine);

        switch (tekuca_komanda) {
          case NISTA:
            if (currentLine.endsWith("GET /volan/")) {
              tekuca_komanda = VOLAN;
              current_param = "";
            }
            if (currentLine.endsWith("GET /brzina/")) {
              tekuca_komanda = BRZINA;
              current_param = "";
            }
            if (currentLine.endsWith("GET /farovi/")) {
              Serial.println("farovi");
              tekuca_komanda = FAROVI;
              current_param = "";
            }
            if (currentLine.endsWith("GET /zmigavac/")) {
              Serial.println("farovi");
              tekuca_komanda = ZMIGAVAC;
              current_param = "";
            }
            break;
          case VOLAN:
            if (c > 32) {
              current_param = current_param + c;
            } else {
              Serial.print("Volan: ");
              Serial.println(current_param);
              volan(current_param.toInt());
              tekuca_komanda = NISTA;
            }
            break;
          case BRZINA:
            if (c > 32) {
              current_param = current_param + c;
            } else {
              Serial.print("Brzina: ");
              Serial.println(current_param);
              brzina(current_param.toInt());
              tekuca_komanda = NISTA;
            }
            break;
          case FAROVI:
            if (c > 32) {
              current_param = current_param + c;
            } else {
              Serial.print("Farovi: ");
              Serial.println(current_param);
              send_farovi(current_param.toInt());
              tekuca_komanda = NISTA;
            }
            break;
          case ZMIGAVAC:
            if (c > 32) {
              current_param = current_param + c;
            } else {
              Serial.print("Zmigavci: ");
              Serial.println(current_param);
              send_zmigavac(current_param.toInt());
              tekuca_komanda = NISTA;
            }
            break;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printStatus() {
  if (isLisening) {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.print("To connect, join the network and open a browser to http://");
    Serial.println(ip);
  } else {
    Serial.println("Creating access point failed");
  }

}
