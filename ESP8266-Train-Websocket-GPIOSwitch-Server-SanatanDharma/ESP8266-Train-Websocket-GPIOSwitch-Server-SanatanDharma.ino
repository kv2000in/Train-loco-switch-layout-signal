/*
 * ESP8266 Web server with Web Socket to control an LED.
 *
 * The web server keeps all clients' LED status up to date and any client may
 * turn the LED on or off.
 *
 * For example, clientA connects and turns the LED on. This changes the word
 * "LED" on the web page to the color red. When clientB connects, the word
 * "LED" will be red since the server knows the LED is on.  When clientB turns
 * the LED off, the word LED changes color to black on clientA and clientB web
 * pages.
 *
 * References:
 *
 * https://github.com/Links2004/arduinoWebSockets
 *
 */

/*
local GPIO_EN1
local GPIO_EN2
local GPIO_1A
local GPIO_2A
local GPIO_3A
local GPIO_4A
GPIO_EN1=2
GPIO_1A=1
GPIO_2A=7
GPIO_3A=5
GPIO_4A=6
gpio.mode(GPIO_EN1,gpio.OUTPUT)
gpio.mode(GPIO_1A,gpio.OUTPUT)
gpio.mode(GPIO_2A,gpio.OUTPUT)
gpio.mode(GPIO_3A,gpio.OUTPUT)
gpio.mode(GPIO_4A,gpio.OUTPUT)
--some gpio pins stay high on reset (eg -5). So set all of them to low
gpio.write(GPIO_EN1,gpio.LOW)
gpio.write(GPIO_1A,gpio.LOW)
gpio.write(GPIO_2A,gpio.LOW)
gpio.write(GPIO_3A,gpio.LOW)
gpio.write(GPIO_4A,gpio.LOW)

//--var GPIO_EN1 = new Pin(4); /*NodeMCU pin D2, gpio 2 -Arduino GPIO4*/
//--var GPIO_EN2 = new Pin(15); /*NodeMCU pin D8, gpio 8 - Arduino GPIO_EN15 - pulled to ground for boot - so unusable- don't use*/
//--var GPIO_1A = new Pin(5);/*NodeMCU pin D1, gpio 1 -Arduino GPIO_2A*/
//--var GPIO_2A = new Pin(13); /*NodeMCU gpio D7,gpio 7 Arduino GPIO_EN13*/
//--var GPIO_3A = new Pin(14); /*NodeMCU gpio D5,gpio 5 Arduino GPIO_EN14*/
//--var GPIO_4A = new Pin(12); /*NodeMCU gpio D6, gpio 6 Arduino GPIO_EN12*/
/*
--Forward -EN1 -1 1A -1 2A -0 3A-1 4A-0
--Reverse -EN1 -1 1A -0 2A -1 3A-0 4A-1
--Stop - EN1 -0
Only using Forward/Reverse and Stop for the train
--Left  - EN1 -1 1A-0 2A-0
--Right - EN1-1 3A-0 4A-0
--Straight - If 1A or 3A is 1 (means - we were going forward before the turn) - do 1A-1 2A-0 3A-1 4A-0 elseif 2A or 3A is 1 - do 1A-0 2A-1 3A-0 4A-1 else do EN1 -0
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <string.h>
#define MAX_STRING_LEN  32



const char *ssid = "****";
const char *password = "****";
//Strange nodemcu numbering
int GPIO_EN1 = 4;    
int GPIO_1A = 5; 
int GPIO_2A = 13;
int GPIO_3A = 14;
int GPIO_4A = 12;


ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8000);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(

<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=2.7">

  <meta  content="text/html; charset=utf-8">
<style>
  * {
    box-sizing: border-box;
}
  
  [class*="col-"] {
    float: left;
    padding: 15px;
}
/* For mobile phones: */
[class*="col-"] {
    width: 100%;
}
@media only screen and (min-width: 1024px) {
    /* For desktop: */
    .col-1 {width: 8.33%;}
    .col-2 {width: 16.66%;}
    .col-3 {width: 25%;}
    .col-4 {width: 33.33%;}
    .col-5 {width: 41.66%;}
    .col-6 {width: 50%;}
    .col-7 {width: 58.33%;}
    .col-8 {width: 66.66%;}
    .col-9 {width: 75%;}
    .col-10 {width: 83.33%;}
    .col-11 {width: 91.66%;}
    .col-12 {width: 100%;}
}
  .switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}
.svrlight{
  background-color: red;
  }
.switch input {display:none;}
.pwrlight{
  position: relative;
  height: 26px;
  width: 26px;
  right: 20px;
  bottom: 4px;
  border-radius: 10%;
  }
.pwrlight:active{
  background-color: red;
  border: none;
  }
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}
.buttonpressedeffect {
  display: inline-block;
  padding: 15px 25px;
  font-size: 24px;
  cursor: pointer;
  text-align: center;
  text-decoration: none;
  outline: none;
  color: #fff;
  background-color: #4CAF50;
  border: none;
  border-radius: 15px;
  box-shadow: 0 9px #999;
}

.buttonpressedeffect:hover {background-color: #3e8e41}

.buttonpressedeffect:active {
  background-color: #3e8e41;
  box-shadow: 0 5px #666;
  transform: translateX(4px);
 }
 
 .buttonhover {
  display: inline-block;
  border-radius: 4px;
  background-color: #f4511e;
  border: none;
  color: #FFFFFF;
  text-align: center;
  font-size: 28px;
  padding: 20px;
  width: 200px;
  transition: all 0.5s;
  cursor: pointer;
  margin: 5px;
}

.buttonhover span {
  cursor: pointer;
  display: inline-block;
  position: relative;
  transition: 0.5s;
}

.button span:after {
  content: '\00bb';
  position: absolute;
  opacity: 0;
  top: 0;
  right: -20px;
  transition: 0.5s;
}

.buttonhover:hover span {
  padding-right: 25px;
}

.button:hover span:after {
  opacity: 1;
  right: 0;
}

</style>
<script language="javascript" type="text/javascript">

 var boolConnected=false;
  function doConnect()
  {
      if (!(boolConnected)){
      /*websocket = new WebSocket(document.myform.url.value);*/
/*
         websocket = new WebSocket('ws://192.168.1.106:8000/');
*/

         
    websocket = new WebSocket('ws://' + window.location.hostname + ':8000/'); 
    boolConnected=true;
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
  }

  }
  function onOpen(evt)
  {
    console.log("connected\n");

  }

  function onClose(evt)
  {
    console.log("disconnected\n");
      /* if server disconnected - change the color to red*/

      boolConnected=false;
  }

  function onMessage(evt)
  {

      
     /* Data returned from ESP will be in the form of ON-1 OFF-1 ON-2 ON-5 OFF-2 OFF-5 CHKA-1010
      CHKA-1010 4 digits correspond to gpio reads of pin 1,2,5,6
    evt.data.slice(-1) = "1" or "2" or "5" or "6"
    evt.data.slice(0,-2)="ON" or "OFF" 
      evt.data.slice(0,-5)="CHKA" 
      
      Data returned from ESP will be in the form of FOR REV STOP
      
      */
      
      
    console.log("response: " + evt.data + '\n');
        if(evt.data=="FOR"){ /* Going forward*/
        document.getElementById("pwrlightFOR").style.backgroundColor="red";
        document.getElementById("sw_FOR").checked=true;
        document.getElementById("sw_REV").disabled=true;
        };
    if(evt.data=="REV"){ /* Going Reverse */
        document.getElementById("pwrlightREV").style.backgroundColor="red";
        document.getElementById("sw_REV").checked=true;
        document.getElementById("sw_FOR").disabled=true;
        };
    if(evt.data=="STOP"){ /* Stopped*/
        document.getElementById("pwrlightFOR").style.backgroundColor="black";
        document.getElementById("pwrlightREV").style.backgroundColor="black";
        document.getElementById("sw_FOR").disabled=false;
        document.getElementById("sw_REV").disabled=false;
        document.getElementById("sw_FOR").checked=false;
        document.getElementById("sw_REV").checked=false;
        };  

    
    
  
  }

  function onError(evt)
  {

  console.log('error: ' + evt.data + '\n');
  websocket.close();


  }

  function doSend(message)
  {
    console.log("sent: " + message + '\n');

    websocket.send(message);
  }

function closeWS(){
    
   /* Probable bug in arduino websocket - hangs if not closed properly, specially by a phone browser entering a powersaving mode*/
    websocket.close();
    boolConnected=false;
}

/*    On android - when page loads - focus event isn't fired so websocket doesn't connect*/
   
   window.addEventListener("focus",doConnect, false);
   
    
 window.addEventListener("blur",closeWS, false);
 window.addEventListener('load', function() {
    foo(true); 
     /*After page loading blur doesn't fire until focus has fired at least once*/
     
    /* window.focus();*/
},{once:true}, false);

/*window.addEventListener('blur', function() {
    foo(false);
}, {once:true}, false); */


function foo(bool) {
    if (bool){
doConnect();
    } else {
        
      
 /*   Probable bug in arduino websocket - hangs if not closed properly, specially by a phone browser entering a powersaving mode
 */       websocket.close();    
    }
}
    
</script>
<script type="text/javascript">


function queryServer1(direction)
{

  var payload;
  if (document.getElementById("sw_"+direction).checked==true){
        payload=direction;}
        else {
        
        payload="STOP";
        };
 

  doSend(payload);
 
};
</script>
<title>Train </title></head>
<body>


<div class = "col-6">
  <!-- pwrlight01 = pwrlight server 0 and gpio 1-->
  <button class="pwrlight" id = "pwrlightFOR"></button> 
<label class="switch">
  <input type="checkbox" id="sw_FOR" onclick='queryServer1("FOR")'>
  <div class="slider"></div>
</label><br>
<strong>Forward</strong><br>
  <button class="pwrlight" id = "pwrlightREV"></button>
<label class="switch">
  <input type="checkbox" id="sw_REV" onclick='queryServer1("REV")'>
  <div class="slider"></div>
</label><br>
<strong>Reverse</strong><br>
</div>

</body>


</html>
)rawliteral";





void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      //itoa( cm, str, 10 );
      //  webSocket.sendTXT(num, str, strlen(str));
      }
      break;
    case WStype_TEXT:
     {
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
//Payload will be - FOR REV STOP
      
     char *mystring = (char *)payload;
      
      if (strcmp(mystring,"FOR") == 0)
      {//Going Forward  -EN1 -1 1A -1 2A -0 3A-1 4A-0
         digitalWrite(GPIO_EN1, HIGH);
         digitalWrite(GPIO_1A, HIGH);
         digitalWrite(GPIO_2A, LOW);
         digitalWrite(GPIO_3A, HIGH);
         digitalWrite(GPIO_4A, LOW);
         
          webSocket.broadcastTXT(payload, length);
      }
      else if (strcmp(mystring,"REV") == 0) 
      {//Going --Reverse -EN1 -1 1A -0 2A -1 3A-0 4A-1
         digitalWrite(GPIO_EN1, HIGH);
         digitalWrite(GPIO_1A, LOW);
         digitalWrite(GPIO_2A, HIGH);
         digitalWrite(GPIO_3A, LOW);
         digitalWrite(GPIO_4A, HIGH);
           webSocket.broadcastTXT(payload, length);
      } 
      else if (strcmp(mystring,"STOP") == 0)
      {//-Stop - EN1 -0
        digitalWrite(GPIO_EN1, LOW);
        digitalWrite(GPIO_1A, LOW);
        digitalWrite(GPIO_2A, LOW);
        digitalWrite(GPIO_3A, LOW);
        digitalWrite(GPIO_4A, LOW);
        
          webSocket.broadcastTXT(payload, length);
       }

     }
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}



void setup()
{ 


  Serial.begin(115200);
  delay(10);
 // set all the GPIOs to OUTPUT
  pinMode(GPIO_EN1, OUTPUT);
  pinMode(GPIO_1A, OUTPUT);
  pinMode(GPIO_2A, OUTPUT);
  pinMode(GPIO_3A, OUTPUT);
  pinMode(GPIO_4A, OUTPUT);
//set all the GPIOs to LOW

  digitalWrite(GPIO_EN1, LOW);
  digitalWrite(GPIO_1A, LOW);
  digitalWrite(GPIO_2A, LOW);
  digitalWrite(GPIO_3A, LOW);
  digitalWrite(GPIO_4A, LOW);

  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  Serial.print("Connect to http://");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);



}

void loop()
{
  webSocket.loop();
  server.handleClient();

}
