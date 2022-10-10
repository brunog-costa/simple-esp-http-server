/*********
  Membros: 
  Grupo: Spectrum 
  Turma: 4ECA
  Repositório disponível em:   
*********/

/*
 Observado durante experimento que o máximo gerado através da captação foi 49kWh, caso seja necessário criar uma lógica de %
*/

// Carregando Wi-Fi library
#include <WiFi.h>

// Configurando rede local para servir aplicação
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

//Tempo para recarga rápida
const float Temp1 = 1200.00; //20 minutos em segundos 
//Tempo para recarga completa
const float Temp2 = 2100.00; // 35 minutos em segundos

//Variáveis para medir geração de energia no sistema
float SV, SV2, SI, SP, SC, SC1, SC2; //S usado para denotar energia da célula fotovoltaica (energia solar)
float BV, BV2, BI, BP, BC, BC1, BC2; //B usado para denotar energia da bateria

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
  BV = map(analogRead(batPin), 0, 1023, 0, 24); //Calcula tensão
  BV2 = (5 * analogRead(batPin)) / 1023; //Calcula tensão e converte para obtensão da corrente
  BI = BV2 / 1; //Calcula corrente
  BP = BI * BV2; //Calcula potência 
  BC1 = BI * Temp1 * 0.01; //Calculo de uso de capacidade de carga por tempo (Recarga Rápida)
  BC2 = BI * Temp2 * 0.01; //Calculo de uso de capacidade de carga por tempo (Recarga Completa)
}

//Repetimos os cálculos anteriores para a captação do painel fotovoltaíco
void CalcSol()
{
  SV = map(analogRead(solPin), 0, 1023, 0, 24);
  SV2 = (5 * analogRead(solPin)) / 1023;
  SI = SV2 / 1;
  SP = SI * SV2;
  SC1 = SI * Temp1 * 0.01; //Validar calculo
  SC2 = SI * Temp2 * 0.01;
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
  String TSPC1 = String(SC1); 
  String TSPC2 = String(SC2);
  String TBPC1 = String(BC1);
  String TBPC2 = String(BC2);

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
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            //Criando uma barra de progresso para indicar a carga
            client.println(".container { background-color: rgb(192, 192, 192); width: 80%; border-radius: 15px; }");
            client.println(".aaa{ background-color: rgb(116, 194, 92); color: white; padding: 1%; text-align: center; font-size: 20px; border-radius: 15px;}");
            client.println(".aa { width: "+ TBPC1 + "px; }");
            client.println(".bb { width: "+ TSPC1 + "px; }");
            // Título da página web
            client.println("<body><h1>Shell Recharge</h1>");
            
            // Exibe a disponibilidade de carga com base nas funções de cálculo
            client.println("<p>Carga Bateria" + TBPC1 + " kW/h</p>");
            client.println("<div class='container'> <div class='aaa aa'>" + TBPC1 + " </div> </div>"); 
            client.println("<p>Carga Painel Solar" + TSPC1 + " kW/h</p>");
            client.println("<div class='container'> <div class='aaa bb'>" + TSPC1 + "</div> </div>"); 
            client.println("<button class='button'>Recarga Rapida</button>");
            client.println("<button class='button'>Recarga Completa</button>");
            client.println("<br></br>");
            client.println("<br></br>");
            client.println("<br></br>");
            client.println("<h2>Medicao de energia fotovoltaica:</h2>");
            client.println("<p>Tensao: "+ String(SV) +" Corrente: "+ String(SI) +" Potencia: "+ String(SP) +" </p>");
            client.println("<h2>Medicao de energia na bateria:</h2>");
            client.println("<p>Tensao: "+ String(BV) +" Corrente: "+ String(BI) +" Potencia: "+ String(BP) +" </p>");
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


