#include "lib/firebase/FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "lib/statistic/Statistic.h"

#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include <Time.h>
#include <TimeLib.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define ID_MQTT  "DHT11"
#define TOPICO_SUBSCRIBE "dto/dht11/status"     //tópico MQTT de escuta
/** #define TOPICO_PUBLISH "dto/dht11/data" Se for publicar o valor no MQTT ao invés do Firebase***/

#define FIREBASE_HOST "outbreak-control-floripa.firebaseio.com"
#define FIREBASE_AUTH "rAqgraI9AMv4LSCS4c4g4TNZs1FtxdFXsqU6V0zu" 
#define wifiSSID "ANDROMEDA"
#define wifiPASS "6104225480"

int16_t utc = -3; //UTC -3:00 Brazil
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);
String dia, mes, hora, minuto, segundo;
int ano;

unsigned long tempomils;
String tempoPub;

uint8_t MAC_array[6];
char MAC_char[18];
int wifiConected = LOW;

// MQTT
const char* BROKER_MQTT = "mosquito.l2cv.com.br"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT
const char* USER_MQTT = "mqtt-spy";
const char* PASS_MQTT = "l2cv@123";


//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
FirebaseData firebaseData;
/*
* Defines do projeto
*/
//GPIO do NodeMCU que o pino de comunicação do sensor está ligado.
#define DHTPIN D1
  
/* A biblioteca serve para os sensores DHT11, DHT22 e DHT21.
* No nosso caso, usaremos o DHT11, porém se você desejar utilizar
* algum dos outros disponíveis, basta descomentar a linha correspondente.
*/
  
#define DHTTYPE DHT11 // DHT 11
  
DHT dht(DHTPIN, DHTTYPE);
Statistic tempStats;
Statistic umidStats;
boolean lerSensor;
  
/* prototypes */
void readSensor(void);
void init_wifi(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void initMQTT(void);
void reconnectMQTT(void);
String dataFormatada(void);
void syncClock(void);

  
void setup(){
    Serial.begin(115200);
    dht.begin();
    
   WiFi.macAddress(MAC_array);
   for (int i = 0; i < sizeof(MAC_array); ++i){
       sprintf(MAC_char,"%s%02x:",MAC_char,MAC_array[i]);
   }
    init_wifi();
    
    tempStats.clear();
    umidStats.clear();
}

/*** Formata a data para ser parte da String salva no Firebase 
     Format: 2016-02-08 15:16:32
***/
String dataFormatada(){
  String agora;
  time_t t = now(); // Get the current time
  dia = String(day(t));
  mes = String(month(t));
  ano = year(t);
  hora = String(hour(t));
  minuto = String(minute(t));
  segundo = String(second(t));

  if(dia.length()<2)
    dia = "0"+dia;
  if(mes.length()<2)
    mes = "0"+mes;
  if(hora.length()<2)
    hora = "0"+hora;  
  if(minuto.length()<2)
    minuto = "0"+minuto;
  if(segundo.length()<2)
    segundo = "0"+segundo;
    
  //agora = String(ano)+"-"+String(mes)+"-"+String(dia)+"/"+String(hora)+":"+String(minuto)+":"+segundo;
  agora = String(ano)+"-"+String(mes)+"-"+String(dia);
  return agora;
}

String horaFormatada(){
  String agora;
  time_t t = now(); // Get the current time
  hora = String(hour(t));
  minuto = String(minute(t));
  segundo = String(second(t));

  if(hora.length()<2)
    hora = "0"+hora;  
  if(minuto.length()<2)
    minuto = "0"+minuto;
  if(segundo.length()<2)
    segundo = "0"+segundo;
    
  //agora = String(ano)+"-"+String(mes)+"-"+String(dia)+"/"+String(hora)+":"+String(minuto)+":"+segundo;
  agora = String(hora)+":"+String(minuto)+":"+String(segundo);
  return agora;
}

void syncClock(){
  timeClient.begin();
  timeClient.update();
  delay(500);
  timeClient.forceUpdate();
  setTime(timeClient.getEpochTime());
}

void init_wifi(){
  int retries = 0;

  WiFi.begin(wifiSSID, wifiPASS); // Conecta na rede WI-FI
      
    while ((WiFi.status() != WL_CONNECTED) && (retries < 20)) {
        delay(500);
        Serial.print(".");
        ++retries;
    }

    if(retries>19){
      //WiFi.mode(WIFI_OFF);
      wifiConected = LOW;
      Serial.println("NAO foi possivel conectar a WIFI");
    }else{
      if(WiFi.status()==WL_CONNECTED){
        wifiConected = HIGH;
        Serial.println("Wifi Conectada...");
        initMQTT();
        syncClock();
        Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);        
        Firebase.reconnectWiFi(true);
      } 
     }
}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT, USER_MQTT, PASS_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        }else{
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT(){
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
    MQTT.connect(ID_MQTT,USER_MQTT,PASS_MQTT);
    MQTT.subscribe(TOPICO_SUBSCRIBE);
    Serial.println("INIT_MQTT");
}
  
//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    Serial.println("MQTT_callback...");
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) {
       char c = (char)payload[i];
       msg += c;
    }
    Serial.print("Payload: ");
    Serial.println(msg);
    if (msg.equals("1")){
        lerSensor=true;
    }else{
      lerSensor=false;
    }
}

void readSensor(){
    float temperatura_lida = 0.0;
    float umidade_lida = 0.0;
    
    temperatura_lida = dht.readTemperature();
    umidade_lida = dht.readHumidity();

  tempStats.add(temperatura_lida);
  umidStats.add(umidade_lida);
  String value = "";

  String leitura = "/sensores";
  leitura+="/";
  leitura+=MAC_char;
  leitura+="/";
  leitura+=dataFormatada();
  
  String umidKey = leitura+"/umidade/"+horaFormatada();
  String tempKey = leitura+"/temperatura/"+horaFormatada();
  /** String tempValue = tempKey+"/";
  tempValue+="valor";**/

  if (tempStats.count() == 1000){
  Serial.println("*****publica leitura Sensor*****");  

    Firebase.setFloat(firebaseData, tempKey+"/valor", temperatura_lida);
    Firebase.setFloat(firebaseData, tempKey+"/min", tempStats.minimum());
    Firebase.setFloat(firebaseData, tempKey+"/max", tempStats.maximum());
    Firebase.setFloat(firebaseData, tempKey+"/media", tempStats.average());
    Firebase.setFloat(firebaseData, tempKey+"/variancia", tempStats.variance());
    Firebase.setFloat(firebaseData, tempKey+"/stdev", tempStats.pop_stdev());    

    Firebase.setFloat(firebaseData, umidKey+"/valor", umidade_lida);
    Firebase.setFloat(firebaseData, umidKey+"/min", umidStats.minimum());
    Firebase.setFloat(firebaseData, umidKey+"/max", umidStats.maximum());
    Firebase.setFloat(firebaseData, umidKey+"/media", umidStats.average());
    Firebase.setFloat(firebaseData, umidKey+"/variancia", umidStats.variance());
    Firebase.setFloat(firebaseData, umidKey+"/stdev", umidStats.pop_stdev());     

    tempStats.clear();
    umidStats.clear();  
    delay(2000);  
  }
  //delay(100);
}

void loop(){
    //keep-alive da comunicação com broker MQTT
    if(!MQTT.connected()){
      init_wifi();
    }
    MQTT.loop();
    if(lerSensor){
      readSensor();
    }
}
