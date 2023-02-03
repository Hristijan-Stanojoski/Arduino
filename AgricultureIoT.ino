#include <ArduinoHttpClient.h>

//WiFi
#include <SoftwareSerial.h>                        
SoftwareSerial esp8266(10,11);                   
#define serialCommunicationSpeed 115200     
#define DEBUG true       

//DHT11
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//Heater(LED)
#define LEDPIN 4

//Soil Moisture
const int Moisture_signal = A0;

//Motor
int Motor_signal = 5;


//RGB
const int redPin = 7;
const int greenPin = 8;
const int bluePin = 9;

//Photoresistor
const int photoresistorPin = A2;
int photoresistorValue = analogRead(photoresistorPin);
int photoresistorPercent = constrain(map(photoresistorValue, 0, 1023, 0, 100), 0, 100);


void setup()
{
   

  //RGB
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  //WiFi
  Serial.begin(serialCommunicationSpeed);           
  esp8266.begin(serialCommunicationSpeed);     
  InitWifiModule();    

  //DHT11
  dht.begin();

  //Grealka(LED) 
  pinMode(LEDPIN, OUTPUT);   
  pinMode(3, OUTPUT);     

  //Motor
  pinMode(Motor_signal, OUTPUT);          
}

void loop()                                                     
{
  //WiFi
  if(esp8266.available())                                           
 {    
    if(esp8266.find("+IPD,"))
    {
     delay(2000);
 
     int connectionId = esp8266.read()-48;                                                
     String webpage = "<h1>Agriculture Monitoring System</h1>";
       // Read sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int Moisture = analogRead(Moisture_signal);
  int percent = map(Moisture, 1023, 560, 0, 100);
  //rainSensorValue = analogRead(rainSensor);
  photoresistorValue = analogRead(photoresistorPin);

  // Build HTML page with sensor data
  webpage += "<p>Humidity: " + String(humidity) + " %</p>";
  webpage += "<p>Temperature: " + String(temperature) + " *C</p>";
  webpage += "<p>Soil Moisture: " + String(percent) + " %</p>";
  
  float percentage = 100 - (photoresistorPercent / 1023.0) * 100;
  webpage += "<p>Light: " + String(percentage) + "%</p>";
  webpage += "<p>Motor Status: " + String((digitalRead(Motor_signal) == HIGH ? "On" : "Off")) + "</p>";

     String cipSend = "AT+CIPSEND=";
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";
     
     sendData(cipSend,1000,DEBUG);
     sendData(webpage,1000,DEBUG);
 
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";    
     sendData(closeCommand,3000,DEBUG);
    }
  }

  //Soil Moisture
    int Moisture = analogRead(Moisture_signal);
  int percent = map(Moisture, 1023, 560, 0, 100);
  Serial.print("Soil moisture level: ");
  Serial.print(percent);
  Serial.println("%");

//Motor
  if (percent < 30) {
    digitalWrite(Motor_signal, HIGH);
    Serial.println("Motor: On");
  } else if (percent > 70) {
    digitalWrite(Motor_signal, LOW);
    Serial.println("Motor: Off");
  }
  
   //DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  //RGB
  if (temperature < 25 || percent < 30) {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  } else {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }

  //Grealka (LED)
   if (temperature < 25) {
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, LOW);
  }

  //Photoresistor
  photoresistorPercent = analogRead(photoresistorPin);
  float percentage = 100 - (photoresistorPercent / 1023.0) * 100;
  Serial.print("Light: ");
  Serial.println(percentage) + "%";

  //DHT11 Read every 5seconds
  delay(5000);
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";                                             
    esp8266.print(command);                                          
    long int time = millis();                                      
    while( (time+timeout) > millis())                                 
    {      
      while(esp8266.available())                                      
      {
        char c = esp8266.read();                                     
        response+=c;                                                  
      }  
    }    
    if(debug)                                                        
    {
      Serial.print(response);
    }    
    return response;                                                  
}

void InitWifiModule()
{
  sendData("AT+RST\r\n", 2000, DEBUG);                                                  
  sendData("AT+CWJAP=\"NETGEAR\",\"11223344\"\r\n", 2000, DEBUG);        
  delay (3000);
  sendData("AT+CWMODE=1\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIFSR\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIPMUX=1\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIPSERVER=1,80\r\n", 1500, DEBUG);                                     

}