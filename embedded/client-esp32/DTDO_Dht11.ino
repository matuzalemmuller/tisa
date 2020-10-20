/**
 * @author Michela Limaco - limaco@gmail.com
 * @description Esse software é parte do trabalho da discplina de Tecnicas de Implementação de Software Avançadas
 * da Pós-Graduação em Engenharia de Automação e Sistemas
 * UFSC - Universidade Federal de Santa Catarina
 * Professor: Carlos Montez
 * Professor convidado: Evandro Cantú
 * Outubro de 2020. 
 * 
 * Funcionamento: Lê um sinal do MQTT para iniciar ou parar a aquisição de dados de um sensor DHT11
 * Publica as estatísticas do valor lido em um BD NoSQL - Firebase
 * Adicionalmente implemente SVM para detecção de Outliers *  
 */

#include "FirebaseESP8266.h"
#include "FirebaseJson.h"
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "Statistic.h"

#include <Arduino.h>
#include <noveltyDetection.h>

#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include <Time.h>
#include <TimeLib.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define ID_MQTT  "DHT11"
#define TOPICO_SUBSCRIBE "dto/dht11/status"     //tópico MQTT de escuta
#define TOPICO_OUTLIER "dto/dht11/outlier"     //tópico MQTT de escuta

#define FIREBASE_HOST "seuprojeto.firebaseio.com"
#define FIREBASE_AUTH "xxxxxsegredoDoBancoxxxxxxxxx" 
#define wifiSSID "REDE_SSID"
#define wifiPASS "REDE_PASS"

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
const char* BROKER_MQTT = "seu_mqtt_broker.com";
const char* USER_MQTT = "user_mqtt"; //opcional
const char* PASS_MQTT = "senha_mqtt"; //opcional
int BROKER_PORT = 1883; // Porta do Broker MQTT

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
void injectOutlier(String);
  
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

    /*** Essa PARTE é exclusiva para o Machine Learning 
     *  1 - abre o File System
     *  2 - Inicia a EEPROM
     *  3 - Lê o modelo e arquivos de parâmetros do FileSystem
     */
    SPIFFS.begin();
    EEPROM.begin(1024);
    Serial.print("Reading model from FileSystem Card into EEPROM...");
    //Default filenames are "svm.mod" and "svm.par" for the model and parameter files, respectively
    SVM_readModelFromSPIFFS(SVM_MODEL_FILENAME, SVM_SCALE_PARAMETERS_FILENAME);
  
}

/*** Formata a data para ser parte da String salva no Firebase 
     Format: 2016-02-08
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
    
  agora = String(ano)+"-"+String(mes)+"-"+String(dia);
  return agora;
}


/*** Formata a hora para ser parte da String salva no Firebase 
     Format: HH:mm:ss
***/
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
        //if (MQTT.connect(ID_MQTT)) {
        if (MQTT.connect(ID_MQTT, USER_MQTT,PASS_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
            MQTT.subscribe(TOPICO_OUTLIER); 
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
    //MQTT.connect(ID_MQTT);
    MQTT.connect(ID_MQTT,USER_MQTT,PASS_MQTT);
    MQTT.subscribe(TOPICO_SUBSCRIBE);
    MQTT.subscribe(TOPICO_OUTLIER); 
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

    if (strcmp(topic,TOPICO_SUBSCRIBE)==0){
      if (msg.equals("1")){
        lerSensor=true;
      }else{
        lerSensor=false;
      }  
    }else{
      Serial.print("Injecao de outlier...\t");
      Serial.println(msg);  
      injectOutlier(msg);
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

  FirebaseJson jsonTemp;
  FirebaseJson jsonUmid;

  if (tempStats.count() == 1000){
  //Serial.println("*****publica leitura Sensor*****");  
    jsonTemp.set("valor",temperatura_lida);
    jsonTemp.set("min",tempStats.minimum());
    jsonTemp.set("max",tempStats.maximum());
    jsonTemp.set("media",tempStats.average());
    jsonTemp.set("variancia",tempStats.variance());
    jsonTemp.set("stdev",tempStats.pop_stdev());

    jsonUmid.set("valor",umidade_lida);
    jsonUmid.set("min",umidStats.minimum());
    jsonUmid.set("max",umidStats.maximum());
    jsonUmid.set("media",umidStats.average());
    jsonUmid.set("variancia",umidStats.variance());
    jsonUmid.set("stdev",umidStats.pop_stdev());
    
    /** 
    Passa a Temperatura Lida no Machine Learning,
    Se o valor for um outlier - muito acima ou abaixo dos valores do modelo
    então uma flag é setada    
    **/
    float sensor[] = {temperatura_lida};
    float ret = SVM_predictEEPROM(sensor, sizeof(sensor)/sizeof(float));
    ret = round(ret);
    if (ret > 0) {
      //Serial.println("NOT Novelty");
      jsonTemp.set("outlier",false);
    }else{
      jsonTemp.set("outlier",true);
     Serial.println("Novelty"); 
    }
    Firebase.setJSON(firebaseData, tempKey, jsonTemp);
    Firebase.setJSON(firebaseData, umidKey, jsonUmid);

    tempStats.clear();
    umidStats.clear();  
    delay(2000);  
  }
}
/**
 * Recebe um valor via MQTT - para simular uma perturbação
 * Passa no SVM_predict que retorna se é um Outlier
 * Publica no firebase com a flag outlier=true
 */
void injectOutlier(String valor){
  
  String leitura = "/sensores";
  leitura+="/";
  leitura+=MAC_char;
  leitura+="/";
  leitura+=dataFormatada();
  String tempKey = leitura+"/temperatura/"+horaFormatada();

  float outlier = valor.toFloat();
  FirebaseJson jsonTemp;

    jsonTemp.set("valor",outlier);
    jsonTemp.set("min",0.0);
    jsonTemp.set("max",outlier);
    jsonTemp.set("media",outlier);
    jsonTemp.set("variancia",0.0);
    jsonTemp.set("stdev",0.0);

  
  float sensor[] = {outlier};
  float ret = SVM_predictEEPROM(sensor, sizeof(sensor)/sizeof(float));
    ret = round(ret);
    if (ret > 0) {
      Serial.println("NOT Novelty");
      jsonTemp.set("outlier",false);      
    }else{
      jsonTemp.set("outlier",true);
      Serial.println("Novelty"); 
    }
    Firebase.setJSON(firebaseData, tempKey, jsonTemp);
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
