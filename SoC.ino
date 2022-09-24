#include <WiFi.h>
#include <esp_spiffs.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
//#include <WebSocketServer.h>

//Consts
const char *ssid = "Shell_Recharge";
const char *passwd = "R41z&nM3!";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 8080;
double x = 0.10;

//Global vars 
AsyncWebServer server(80);
//WebSocketServer webSocket = WebSocketsServer(8080);
char msg_buf[10];
float V, V2, I, P, Cash;

//WebSocket Activation Funcs

void onWebScoketEvent(uint8_t client_num,
                      WStype_t type,
                      uint_t * payload,
                      size_t lenght) {
                        //WebSocket Event Type
                        switch(type){
                          case WStype_DISCONNECTED:
                            Serial.printf("[%u] Disconected\n", client_num);
                            break;

                          case WStype_CONNECTED:
                          {
                            IPAddress ip = webScoket.remoteIP(client_num);
                            Serial.printf("[%u] Call from ", client_num);
                            Serial.prinf(ip.toString());
                            }
                            break;

                          case WStype_TEXT:
                            Serial.printf("[%u] Payload ", client_num, payload);
                            //Logic Ocus Pocus and break
                            break;
                          case WStype_BIN:
                          case WStype_ERROR:
                          case WStype_FRAGMENT_TEXT_START:
                          case WStype_FRAGMENT_BIN_START:
                          case WStype_FRAGMENT:
                          case WStype_FRAGMENT_FIN:
                          default:
                            break;
                        }
                      }

//Setting up webserver --> perhaps will need some more logic ocus pocus
void onIndexRequest(AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET for" + request->url());
  request->send(SPIFFS, "index.html", "text/html"); //HTML Callback
}

void onCSSRequest(AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET for" + request->url());
  request->send(SPIFFS, "style.css", "text/css"); //CSS Callback (if needed)
}

void onNotFoundResponse(AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET for" + request->url());
  request->send(404,"text/plain", "Need some help ?"); //Non existing page callback
}

//Stats function & Analog read 
void CalculaBateria()
{
  V = map(analogRead(0), 0, 1023, 0, 24);
  V2 = (5.00 * analogRead(0)) / 1023.00;
  I = V2 / 1;
  P = I * V;
}

void CalculaFotoVolt()
{
  V = map(analogRead(2), 0, 1023, 0, 24);
  V2 = (5.00 * analogRead(3)) / 1023.00;
  I = V2 / 1;
  P = I * V;
}

void CalculaPreco()
{
  Cash = P * x;
}


void setup() {
  // put your setup code here, to run once:
  //pinMode for the necessary atuation inside PoC

  //Start Serial logging
  Serial.begin(115200);

  //File System reading check
  if( !SPIFFS.begin()){
      Serial.println("is Spiffs okay ?");
      while(1);
  }

  //Starting Up Access Point
  WiFi.softAP(ssid, passwd);

  //Getting to know our own network
  Serial.println("Starting checks\n");
  Serial.println("AP: [good]\n");
  Serial.printf("Ip Address: " WiFi.softAPIP());

  //Kickstarting webserver 
  server.on("/", HTTP_GET, onIndexRequest);
  server.on("/style.css", HTTP_GET, onCSSRequest);
  server.onNotFound(onNotFoundResponse);

  //Kickstarting websockets 
  webSocket.begin();
  webSocket.onEvent(onWebScoketEvent);
}

void loop() {

  CalculaBateria(); //Chama função para calcular disponibilidade das baterias
  CalculaFotoVolt(); //Chama função para calcular disponibilidade das celulas fotovoltaicas
  CalculaPreco(); //Calcula preço da recarga 
  webSocket.loop()
}
