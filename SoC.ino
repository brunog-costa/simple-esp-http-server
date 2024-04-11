/*********
  Autor: 
  Bruno Gois Costa
  
  Informações sobre a captação de energia que serão usados para funções deste programa: 
  Tensão de circuito aberto (Voc): 7,4 V
  Corrente de curto-circuito (Isc): 1,2 A
  Potência máxima (Pmax): 2,3 W
  Tensão na potência máxima (Vmp): 5,6 V
  Corrente na potência máxima (Imp): 410 mA
*********/

// Carregando Wi-Fi library
#include <WiFi.h>

// Configurando rede local para servir aplicação
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

//Tempo para recarga rápida 20 minutos em segundos 
const float Temp1 = 1200.00; 
//Tempo para recarga completa 35 minutos em segundos
const float Temp2 = 2100.00; 

//Variáveis para medir geração de energia no sistema
float SV, SV2, SI, SP, SC, SC1, SC2; //S usado para denotar energia da célula fotovoltaica (energia solar)
float BV, BV2, BI, BP, BC, BC1, BC2; //B usado para denotar energia da bateria
float TempMed;
// Configurando porta do webserver
WiFiServer server(80);

// Criando cabeçalho http para monitorar requisições
String header;

// Pinos GPIO que serão usados
const int batPin = 33;
const int solPin = 32;

//Sensoriamento e captação da energia disponível para recarga
void CalcBat()
{
  BV = map(analogRead(batPin), 0, 1023, 0, 7); //Calcula tensão
  BV2 = (5 * analogRead(batPin)) / 1023; //Calcula tensão e converte para obtensão da corrente
  BI = BV2 / 100; //Calcula corrente
  BP = BI * BV2; //Calcula potência 
  TempMed = (Temp1 + Temp2)/2;
  BC = BI * TempMed  * 0.1; //Calculo de uso de capacidade de carga por tempo médio 
  //BC2 = BI * Temp2 * 0.01; --> Habilitar caso cálculo esteja sendo feito via circuito em paralelo 
}

//Repetimos os cálculos anteriores para a captação do painel fotovoltaíco
void CalcSol()
{
  SV = map(analogRead(solPin), 0, 1023, 0, 7);
  SV2 = (5 * analogRead(solPin)) / 1023;
  SI = SV2 / 100; //Converte para miliampere
  SP = SI * SV2;
  TempMed = (Temp1 + Temp2)/2;
  SC = SI * TempMed * 0.1; //Validar calculo
  //SC2 = SI * Temp2 * 0.01;  --> Habilitar caso cálculo esteja sendo feito via circuito em paralelo 
  
}

void setup() {
  // Configurando baud adequado para a comunicação serial (logs)
  Serial.begin(115200);
  // Iniciando webserver através de Access Point com rede restrita
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
}

void loop(){

  // Porta configurada e aguardando conexão do cliente
  WiFiClient client = server.available(); 
  // Inicia captação dos valores via leitura analógica
  CalcSol();
  CalcBat();
  // Valores são convertidos em string para exibição

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK - Content-type:text/html - Connection: close");
            client.println();
            
            // Iniciando o documento HTTP exibido via Access Point
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv='refresh' content='30'>");
            // Adding stylesheet (CSS) to the page
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            //Criando uma barra de progresso para indicar a carga
            client.println(".container { background-color: rgb(192, 192, 192); width: 80%; border-radius: 15px; }");
            client.println(".PB { background-color: rgb(116, 194, 92); color: white; padding: 1%; text-align: center; font-size: 20px; border-radius: 15px;}");
            client.println(".BSSPB { width: 75%; height: 80%; padding-left: 10%;}"); //BSSPB = Battery Style Sheet Progress Bar
            client.println(".SSSPB { width: 75%; height: 80%; padding-left: 10%;}"); //SSSPB = Solar Style Sheet Progress Bar
            client.println("</style>");
            // Título da página web
            client.println("<body><h1>Shell Recharge</h1>");
            // Exibe a disponibilidade de carga com base nas funções de cálculo
            client.println("<p>Carga Bateria" + String(BC) + " kW/h</p>");
            client.println("<div class='container'> <div class='PB BSSPB'>" + String(BC) + " </div> </div>"); 
            client.println("<p>Carga Painel Solar" + String(SC) + " kW/h</p>");
            client.println("<div class='container'> <div class='PB SSSPB'>" + String(SC) + "</div> </div>"); 
            client.println("<br></br>");
            client.println("<h2>Medicao de energia bateria:</h2>");
            client.println("<p>Tensao: "+ String(BV) +"V Corrente: "+ String(BI) +"mA Potencia: "+ String(BP) +"W </p>");
            client.println("<h2>Medicao de energia na fotovoltaica:</h2>");
            client.println("<p>Tensao: "+ String(SV) +"V Corrente: "+ String(SI) +"mA Potencia: "+ String(SP) +"W </p>");
            client.println("</body></html>");
            // Termina a request com uma linha vazia
            client.println();
            // Fim do loop
            break;
          } else { // Se receber uma nova linha, esvazia a linha atual
            currentLine = "";
          } //Retorno do ponteiro
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    // Limpa o cabeçalho http enviado para o servidor
    header = "";
    // Encerra a conexão
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


