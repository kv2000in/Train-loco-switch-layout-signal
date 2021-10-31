/*
best to run checkflashconfig before uploading to a new board.
today 10/30/2021
06:46:22.306 -> Flash ide  size: 1048576 bytes
06:46:22.308 -> Flash ide speed: 40000000 Hz
06:46:22.311 -> Flash ide mode:  DOUT
06:46:22.313 -> Flash Chip configuration ok.
06:46:22.338 -> 

Upload config

*/
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <string.h>
#include <ArduinoOTA.h>

#ifndef APSSID
#define APSSID "Train-Master"
#define APPSK  "revoRym123"
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8000);

static const char PROGMEM MANIFEST_JSON[] = R"rawliteral(
{
"name": "myROVER",
"short_name": "myrover",
"description": "A simple rover controller app.",
"display": "standalone",
"scope": "/",
"orientation":  "landscape"
}
)rawliteral";

static const char PROGMEM CHECK_SENSORS[] = R"rawliteral(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
</head>
<body>
</body>
</html>
)rawliteral";
static const char PROGMEM INDEX_HTML[] = R"rawliteral(<!DOCTYPE html>
<!--
/*
OC0 A PD6 D6 Timer 0 980 Hz PWM with analogwrite(). Timer 0 is used for delay and millis().
OC0 B PD5 D5 Timer 0 980 Hz PWM with analogwrite(). Timer 0 is used for delay and millis().
OC1A  PB1 D9 490 Hz PWM with analogwrite(). Timer 1. Can't use Timer 1 if using Servo library
OC1B  PB2 D10 490 Hz PWM with analogwrite(). Timer 1. Can't use Timer 1 if using Servo library
OC2A PB3 D11 490 Hz PWM with analogwrite(). Timer 2.
OC2B PD3 D3 490 Hz PWM with analogwrite(). Timer 2. D3 is also INT 1 (hardware interrupt).
PD2 D2 INT0
PD3 D3 INT1 (and see above)
A4 SDA for i2c NEED TO BE PULLED UP BY EXTERNAL 4.7K RESISTORS TO VCC
A5 SCL for i2c NEED TO BE PULLED UP BY EXTERNAL 4.7K RESISTORS TO VCC
Can use analog pins as outputs to drive servos using  servo library.
When using  analog functions, the pins are numbered 0-5. but these same pins are numbered 14-19 when used with digital functions.
Servo 50 Hz PWM - Period = 1/50 S = 20 ms.
1 ms High, 19 ms Low - Full left, 1.5 ms High, 18.5 ms Low - middle, 2 ms High, 18 ms Low - full right.
C-F Forward Motor A Returns C-F:OK
C-f forward Motor B
C-R Reverse A
C-r reverse B
C-X stop A
C-x stop B
A-A adjustPWM using A for Analog ADC battery voltage read. Returns H-255:OK, h-255:OK, L-255:OK, l-255:OK for PWMAMAX, PWMBMAX, PWMAMIN, PWMBMIN
A-D adjustPWM using D for Digital SMBUS battery voltage read. Returns H-255:OK, h-255:OK, L-255:OK, l-255:OK for PWMAMAX, PWMBMAX, PWMAMIN, PWMBMIN
S-255 sets PWMA to 255, returns S-255:OK
s-255 sets PWMB to 255, returns s-255:OK
b-V analog read battery voltage (mV) Returns v-13245:OK
B-V SMBUS battery voltage (mV) Returns V-13245:OK
B-I SMBUS Current (mA) Returns I--245:OK
B-C SMBUS Relative State of Charge (%) Returns c-45:OK
B-R SMBUS Remaining capacity (mAH) Returns R-3245:OK
B-T SMBUS Battery Temperature (deg C) Returns T-13.2:OK
B-F SMBUS Battery Full Capacity (mAH) Returns F-5245:OK
B-O SMBUS Battery LED OFF Returns O-OFF:OK
B-N SMBUS Battery LED ON Returns O-ON:OK
W-135 sets servo W to 135 degrees, returns W-135:OK
X-135 sets servo X to 135 degrees, returns X-135:OK
Y-135 sets servo Y to 135 degrees, returns Y-135:OK
Z-135 sets servo Z to 135 degrees, returns Z-135:OK
w-a attach servoW returns w-a:OK
w-d detach servoW returns w-d:OK
x-a attach servoX
x-d detach servoX
y-a attach servoY
y-d detach servoY
z-a attach servoZ
z-d detach servoZ
K-N LEDs turn on Returns K-N:OK
K-O LEDs turn off Returns K-O:OK
N-N Siren turn on Returns N-N:OK
N-O Siren turn off Returns N-O:OK
D-X returns distance read by HCSR04 as D-124:OK
0-X returns counter0 value as 0-2333:OK
1-X returns counter1 value as 1-2344:OK
0-0 sets counter0 to 0 and returns 0-0:OK
1-0 sets counter1 to 0 and returns 1-0:OK
*/
-->
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=0">
<link rel="manifest" href="manifest.json">    
<style>



/*        disable scrolling    */
html{
margin:0;
padding:0;
overflow: hidden;
}
body{
position: absolute;
width:100%;
height:100%;
overflow: auto;
margin:0px;
background: black;
}



* {
margin: 0;
padding: 0;
box-sizing: border-box;
}

/*
input
{
width:0px;
height:0px;


}
input:checked + .slider-knob
{
background: green;
-webkit-transform: translateY(-26px);
-ms-transform: translateY(-26px);
transform: translateY(-26px);
}
*/
.sliderbuttons
{
position: absolute;
bottom:40%;
width:100%;
height:20%;
}
.slidercolumn
{
width:100%;
height:100%;
position: relative;
}
.servosliderbuttons
{
position: absolute;
bottom:0%;
width:100%;
height:20%;
}
.fuelgaugecolumn
{
width:49%;
height:100%;

}
.individual-status-lights
{
width:100%;
box-sizing: border-box;
height:33.3%;
}
/*     top 10%           */
.notification-bar
{
width:100%;
display:flex;
position: fixed;
height:10%;
background: white;
}
.selectables{
width: 20%;
}
.console
{
width:20%;
left:0px;
}

.numericals{
border: solid;
border-radius: 20%;
width:10%;
}
/*                bottom 90%*/
.half
{
width:100%;
display:flex;
position: fixed;
height:45%;
}
/*                position top half-10% away from top*/
#top-half
{
top:10%;
}
/*                position bottom half -at bottom*/
.bottom
{
right:0;
bottom:0;
}
/*                divide 5 even width columns*/
.twenty
{
width:20%;
}
.ten
{
width:10%;
}
.fifty
{
width:50%;
}
.sixty
{
width:60%;
}
.horizontalslider
{
position: relative;
height:50%; 
}
/*                if height is not sufficient - make room for steering*/
@media only screen and (max-height: 444px) {
.horizontalslider
{
position: relative;
height:10%; 
}
}

.slider-knob
{
height:33.3%;
top:33.3%;
position: relative;
background: green;
transition: .4s;
border-radius: 1%;
}
.gearbuttons
{
width:100%;
height:33.3%;
border-radius:20%;
}
.vertical3slider
{
position:relative;
top: 0px;
left: 0px;
border: 2px solid blueviolet;
align-self: center;
height:100%;
margin:0 auto; 
display: flex;
}
.genericbuttons{

height: 100%;

border-radius: 10%;
}
.vertical2slider
{
position:relative;
top: 0px;
left: 0px;
border: 2px solid blueviolet;
align-self: center;
height:100%;
margin:0 auto;  
}
.verticalslider
{
position:relative;
top: 0px;
left: 0px;
border: 2px solid blueviolet;
align-self: center;
height:100%;
margin:0 auto;  
display: flex;
}
.steering-container 
{
position: relative;
z-index: 1;
bottom:-50px;

}
.steeringbuttonswrapper
{
position: absolute;
top: 21%;
left: 40%;
}
/*    CSS for Outer ring of joystick*/  
.steering-outer-circle 
{
position:relative;
top: 0px;
left: 0px;
border: 2px solid blueviolet;
align-self: center;
width: 300px;
height:300px;
border-radius: 300px;
margin:0 auto;
}
/*CSS for inner ring of joystick*/        
.steering-inner-circle 
{
position:absolute;
border: 0px;
top: 50px;
left:50px;
/*opacity: 0.2;*/
border: solid blueviolet;
width: 250px;
height:250px;
border-radius: 250px;
}
.steering-knob 
{
position:absolute;
border: solid blueviolet;
top: -50px;
left: 100px;
background: blueviolet;
/*opacity: 0.2;*/
border: solid blueviolet;
width: 100px;
height:100px;
border-radius: 100px;
}

.three-slider
{
width:100%;
height:100%;
\
}
.three-lights
{
width:49%;
height:100%;
background: red;

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
background-color: #bf1f1f;
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
</style>

<title>myRover</title>
</head>
<body>

<!--<img id="mjpeg_dest"src="/cam_pic_new.php" style="width:100%;"> -->
<!--<div class="joystick-container" id="joystick-container">
<div class="outer" id="outer"><div class="inner" id="inner"></div></div>
</div>-->

<div class="notification-bar" id="notification">
<div class = "console" id = "mylocalconsole"></div>
<div class="selectables">
</div>
<div class="smbusnumericals numericals" id = "smbusbatteryI"></div>
<div class="smbusnumericals numericals" id = "smbusbatteryV"></div>
<div class="smbusnumericals numericals" id = "smbusbatteryT"></div>
<div class="smbusnumericals numericals" id = "smbusbatteryc"></div>
<div class="smbusnumericals numericals" id = "smbusbatteryR"></div>
<div class="smbusnumericals numericals" id = "smbusbatteryF"></div>
<div class="numericals" id = "analogbatteryV"></div>

</div>    
<div class ="half" id="top-half">
<div class="ten vertical3slider" id = "top-left-1" id="gearB">

<div class = "three-slider" id="gearBslider">

<button class = "gearbuttons" id="forwardgearbuttonB" onclick='doSend("<C-f>")'>
F
</button>

<button class = "gearbuttons" id="stopgearbuttonB" onclick='doSend("<C-x>")'>
X
</button>
<button class = "gearbuttons" id="reversegearbuttonB" onclick='doSend("<C-r>")'>
R
</button>
</div>




</div>
<div class="ten vertical2slider" id = "top-left-2">                                       
<div class="slidercolumn" id="slidercolumnB"><button class = "sliderbuttons" id="speedcontrolB">
S
</button></div>



</div>

<div class="ten vertical2slider" id = "top-left-3">
<div class="slidercolumn servoslidercolumn" id="slidercolumnX"><button class = "servosliderbuttons" id="speedcontrolX">
X
</button></div>
</div>
<div class="ten vertical2slider" id = "top-left-4">
<div class="slidercolumn servoslidercolumn" id="slidercolumnY"><button class = "servosliderbuttons" id="speedcontrolY">
Y
</button></div>
</div>
<div class="ten vertical2slider" id = "top-left-5">
<div class="slidercolumn servoslidercolumn" id="slidercolumnZ"><button class = "servosliderbuttons" id="speedcontrolZ">
Z
</button></div>
</div>
<div class = "fifty">     <!-- pwrlight01 = pwrlight server 0 and gpio 1-->
<button class="pwrlight" id = "pwrlight"></button> 
<label class="switch">
<input type="checkbox" id="lightswitch" onclick='togglelightswitch()'>
<div class="slider"></div>
</label>

<button class = "genericbuttons" id="connectWS" onclick='doConnect()'>
Connect
</button>
<button class = "genericbuttons" id="closeWS" onclick='doClose()'>
Close
</button> 
<button class = "genericbuttons" id="reloadfromsource" onclick='location.reload()'>
Reload
</button> 
<button class = "genericbuttons" id="getbatteryinforbtn" onclick='getbatteryinfo'>
Battery
</button> 
</div>
</div>
<div class ="half bottom" id="bottom-half">
<div class="sixty" id = "bottom-middle"><div class="horizontalslider">  <button class ="genericbuttons" id = "lefttrimset" onclick="setmidsteeringangle('left')">Trim L</button>
<button class ="genericbuttons" id = "righttrimset" onclick="setmidsteeringangle('right')">Trim R</button> </div>
<div class="steering-container" id="steering-container">
<div class="steering-outer-circle" id="steering-outer-circle">
<!--                            <div class="steering-inner-circle" id="steering-inner-circle"></div>-->
<div class ="steering-knob" id="steering-knob"></div>
<div class = "steeringbuttonswrapper">

</div>
</div>
</div>
</div>
<div class="twenty vertical3slider" id = "gearA">
<div class="slidercolumn" id="slidercolumnA"><button class = "sliderbuttons" id="speedcontrolA">
S
</button></div>
</div>

<div class="twenty verticalslider" id = "bottom-left">
<div class="sendcommand" id="sendcommand">
<input list = "availablecommands"  id="mycommands"><button id="sendcommandbutton" class="genericbutton" onclick= 'sendcommandfromtext()'>Send</button>

<datalist id="availablecommands">
<option value="<0-X>">
<option value="<1-X>">
<option value="<0-0>">
<option value="<1-0>">
<option value="<D-X>">
<option value="<N-N>">
<option value="<N-O>">
<option value="<K-N>">
<option value="<K-O>">
<option value="<B-O>">
<option value="<B-V>">
<option value="<b-V>">

</datalist>
</div>


</div>

</div>



<div class = "detailedlistofcommands"><!--todo!--></div>

</body>

<script>

/*       For eventhandler passive support option, see here: https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener
*/


let passiveSupported = false;
try {
const options = {
get passive() { // This function will be called when the browser
//   attempts to access the passive property.
passiveSupported = true;
return false;
}
};
window.addEventListener("test", null, options);
window.removeEventListener("test", null, options);
} catch(err) {
passiveSupported = false;
}


//var PWMAMAX;
var PWMAMAX=255; //changed to allow for max PWM output.
var PWMBMAX;
var PWMAMIN=155; //since using 7.5 V regulated - easier to just set this manually. 
var PWMBMIN;
var PWMA;
var PWMB;
var PWMASTEPS = 25;
var PWMBSTEPS = 25;
var previousY =0;
var smartbattery;
var rpmenabled = true;
var encoder0ppr = 20;
var encoder1ppr = 20;
var encoder0prevreading=0;
var encoder1prevreading=0;
var currentrpmA=0;
var currentrpmB=0;
var checkrpminterval=500;
var checkobstacelenabled=true;
var checkobstacleinterval=500;
var obstacledistance=0; 
var recurringbatteryinfo = setInterval(getbatteryinfo, 10000); 
var checkingrpmA;
var checkingrpmB;
var checkingobstacle;

function sendcommandfromtext(){

doSend(document.getElementById("mycommands").value);
}

function getbatteryinfo(){
doSend("<B-V>");
doSend("<B-I>");
doSend("<B-T>");
doSend("<B-C>");
doSend("<B-R>");
doSend("<B-F>");    
doSend("<b-V>");

}


function parse_incoming_websocket_messages(data){
//Data is S-255:OK or S-255:FAIL or C-F:OK or C-F:FAIL
//Determine if it is OK response or FAIL response, splice it on ":", log it if it is FAIL, proceed if it is OK
var myResponse=data.split(":")[1];
var returningdata=data.split(":")[0];
switch(myResponse){
case ("FAIL"):
document.getElementById("mylocalconsole").innerHTML=data;
break;
case("OK"):
var returningdata=data.split(":")[0];
//Now find the first character and do something
response_based_on_first_char(returningdata);
default:
document.getElementById("mylocalconsole").innerHTML=data;
break;

}

}
function checkrpm(whichencoder,zeroornot){
if(rpmenabled){
if (zeroornot=="0"){
if (whichencoder=="0"){
encoder0prevreading=0;  
}
else if (whichencoder=="1"){
encoder1prevreading=0;

}

}
doSend("<"+whichencoder+"-"+zeroornot+">");

}

}
function handlerpm(whichencoder,value){
switch(whichencoder){
case ("0"):
currentrpmA= ((value-encoder0prevreading)*(1000/checkrpminterval)*60)/encoder0ppr;
encoder0prevreading=value;
break;
case ("1"):
currentrpmB= ((value-encoder1prevreading)*(1000/checkrpminterval)*60)/encoder1ppr;
encoder1prevreading=value;
break;

default:
console.log(whichencoder,value);
break;
}

}
function handleobstacle(startorend){
if(checkobstacelenabled){


if (startorend=="start"){
checkingobstacle=setInterval(doSend,checkobstacleinterval,"<D-X>");
}
else if (startorend=="end")
{
clearInterval(checkingobstacle);
}

}

}

function handlereturnsfromcommands(whichcommand){

//C-F:OK , OK has been dealt with, C has been dealt with, now deal with F
switch(whichcommand){
case("F"):
//Motor A is rotating forward, change the status light and calculate RPM/Speed if RPM is enabled 
//checkrpm("0","0");
//checkingrpmA = setInterval(checkrpm,checkrpminterval,"0","X");
//handleobstacle("start");
break;
case("f"):
//Motor B is rotating forward, change the status light and calculate RPM/Speed if RPM is enabled
//checkrpm("1","0");
//checkingrpmB = setInterval(checkrpm,checkrpminterval,"1","X");
break;
case("R"):
//Motor A is rotating reverse, change the status light and calculate RPM/Speed if RPM is enabled
//checkrpm("0","0");
//checkingrpmA = setInterval(checkrpm,checkrpminterval,"0","X");
break;
case("r"):
//Motor B is rotating reverse, change the status light and calculate RPM/Speed if RPM is enabled
//checkrpm("1","0");
//checkingrpmB = setInterval(checkrpm,checkrpminterval,"1","X");
break;
case("X"):
//Motor A is stopped change the status light and disable RPM
//clearInterval(checkingrpmA);
//handleobstacle("end");
break;
case("x"):
//Motor B is stopped, change the status light and disable RPM.
//clearInterval(checkingrpmB);
break;
default:
console.log(whichcommand);
break;
}

}
function togglelightswitch()
{
var myswitch=document.getElementById("lightswitch");
if (myswitch.checked){
doSend("<K-N>");
}
else {
doSend("<K-O>");
}
}        
function response_based_on_first_char(mylocalvar){
//mylocalvar = S-255 or C-F or H-135 etc , see above
switch(mylocalvar.split("-")[0])
{
//PWMAMAX
case ("H"):
//PWMAMAX=Number(mylocalvar.split("-")[1]);
break;
//PWMBMAX
case ("h"):
PWMBMAX=Number(mylocalvar.split("-")[1]);
break;
//PWMAMIN    
case ("L"):
//PWMAMIN=Number(mylocalvar.split("-")[1]);
break;
//PWMBMIN    
case ("l"):
PWMBMIN=Number(mylocalvar.split("-")[1]);
break;
//PWMA    
case ("S"):
PWMA=Number(mylocalvar.split("-")[1]);
break;
//PWMB    
case ("s"):
PWMB=Number(mylocalvar.split("-")[1]);
break;
//Commands    
case ("C"):
handlereturnsfromcommands(mylocalvar.split("-")[1]);
break;
//Servo W angle    
case ("W"):
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
//Servo X angle
case ("X"):
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
//Servo Y angle
case ("Y"):
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
//Servo Z angle    
case ("Z"):
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
//Battery Smbus Voltage    
case ("V"):
document.getElementById("smbusbatteryV").innerHTML=mylocalvar.split("-")[1];
break;
//Battery ADC analog Voltage    
case ("v"):
document.getElementById("analogbatteryV").innerHTML=mylocalvar.split("-")[1];
break;
//Battery smbus current - current returns as -144 (negative 144) so its I--144 hence 2nd element.
case ("I"):
document.getElementById("smbusbatteryI").innerHTML=mylocalvar.split("-")[2];
break;
//Battery smbus remaining capacity (mAH)
case ("R"):
document.getElementById("smbusbatteryR").innerHTML=mylocalvar.split("-")[1];
break;
//Battery smbus remaining charge (%)    
case ("c"):
document.getElementById("smbusbatteryc").innerHTML=mylocalvar.split("-")[1];
break;
//Battery smbus temperature (degC)    
case ("T"):
document.getElementById("smbusbatteryT").innerHTML=mylocalvar.split("-")[1];
break;
//Battery smbus full charge capacity (mAH)    
case ("F"):
document.getElementById("smbusbatteryF").innerHTML=mylocalvar.split("-")[1];
break;
//Battery SMBUS LED on or off    
case ("O"):
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
//Truck light Leds
case ("K"):

if (mylocalvar.split("-")[1] == "N"){document.getElementById("pwrlight").style.backgroundColor="red";}
if (mylocalvar.split("-")[1] == "O"){document.getElementById("pwrlight").style.backgroundColor="white";}
break;
//Truck siren
case ("N"):
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
//encoder0 reading
case ("0"):
handlerpm("0",Number(mylocalvar.split("-")[1]));
break;
//encoder1 reading
case ("1"):
handlerpm("1",Number(mylocalvar.split("-")[1]));
break;
//obstacle reading
case ("D"):
obstacledistance=Number(mylocalvar.split("-")[1]);
break;

default:
document.getElementById("mylocalconsole").innerHTML=mylocalvar;
break;
}
}
function attachordetachservos(whichservo,attachordetach){
doSend("<"+whichservo+"-"+attachordetach+">");

}

/*function handleOrientation(event) {
var y = event.gamma;
var z = event.alpha;
var x = event.beta;
//console.log("alpha: " + z + "\n");
//document.getElementById("mylocalconsole").innerHTML = "alpha: " + z + "\n";
//Device has moved more than the step
if(Math.abs(y-previousY)>steeringanlgesteps){

document.getElementById("mylocalconsole").innerHTML = "<W-"+(y+90)+">";  
//previousY=y;
}


}*/

/*
works on chrome 52 on android (GalaxyS5). I think beyond chrome 60 - needs window.isSecureContext to be true - for access to sensors
secure context means everything has to be ssl. Doesn't work even if I save the file locally on the phone and just connect to websocket. On the other hand, saving it locally on the computer and using it - becomes a secure context and sensors can be accessed. Yes, macbook has orientation sensors too.

window.addEventListener('deviceorientation', handleOrientation);
*/
/*    with x1 and y1 being the coordinates of the center of a circle, (x-x1)^2+(y-y1)^2 = radius^2
so for any given value of x, y = sqrt(radius^2-(x-x1)^2)+y1*/


var draggable = document.getElementById('steering-knob');
var outer = document.getElementById('steering-outer-circle');
var touchstartX;
var touchstartY;
//get center coordinates of the steering-outer-circle
var centerofsteeringcircleX;
var centerofsteeringcircleY;
var rect1 = outer.getBoundingClientRect();
var rect;
var myX;
var myY;
var myoffsetfromcontainerX;
var myoffsetfromcontainerY;
var mymaxY;
var myangle;
var steeringanlgesteps =5;  
var previoussentangle=0;
var steeringrange=65; //32.5 degrees left and 32.5 degrees right.
var steeringservomidangle=90;
var steeringservomaxangle = 170; //don't go out of bounds else cheap servo can stall.
var steeringservominangle = 10;
centerofsteeringcircleX = rect1.left+((rect1.right -rect1.left)/2);
centerofsteeringcircleY= rect1.top+((rect1.bottom -rect1.top)/2);

draggable.addEventListener('touchstart', function(event) {
var touch = event.targetTouches[0];
rect = draggable.getBoundingClientRect();
//Start point -center of steering knob
touchstartX=rect.left+((rect.width)/2);
touchstartY=rect.top+((rect.height)/2);
rect1=outer.getBoundingClientRect();
//don't want the steering knob to go below horizon. Find the lowermost allowable steeringknob center Y pixel. it should be steering-knob's radius away from the bottom of the steering container
mymaxY=window.innerHeight-(rect.height)/2;
//                                myoffsetfromcontainerX=rect.left-rect1.left;
//                                myoffsetfromcontainerY=rect.top-rect1.top;
myoffsetfromcontainerX = draggable.offsetLeft;
myoffsetfromcontainerY=draggable.offsetTop;

//                 console.log("touchstartx="+touchstartX+", touchstartY="+touchstartY+", myoffsetX="+myoffsetfromcontainerX+", myoffsetY="+myoffsetfromcontainerY+", mymaxY="+mymaxY);
}, passiveSupported
? { passive: true } : false);
draggable.addEventListener('touchmove', function(event) {
var touch = event.targetTouches[0];
//get the x point of touch;
myX = touch.pageX;
myY=centerofsteeringcircleY-Math.sqrt((150*150)-(Math.pow((myX-centerofsteeringcircleX),2)));




//check if falling below horizon
if (myY<mymaxY)
{
draggable.style.left = myoffsetfromcontainerX+(myX-touchstartX)+'px';
draggable.style.top = myoffsetfromcontainerY+(myY-touchstartY)+'px';


/*                            calculate angle off center at current position
myangle = -135(Left) to -45 (right). multiplied by 2 to get -270 to -90.
convert to 0-180 by adding 270.
Convert 180 scale to "Range" scale by *Range/180
Shift the mid point of the scale so that Range/2 = midservoangle.*/
myangle=parseInt((((Math.atan2(myY - centerofsteeringcircleY, myX - centerofsteeringcircleX) * 180 / Math.PI)*2)+270)*(steeringrange/180))+(steeringservomidangle-steeringrange/2);


}




if(Math.abs(myangle-previoussentangle)>steeringanlgesteps){

//send command only if it is with in the bounds of the steering servo
if (myangle >= steeringservominangle && myangle <= steeringservomaxangle){
doSend("<W-"+myangle+">"); 

previoussentangle=myangle;

}

}
//event.preventDefault();
}, passiveSupported
? { passive: true } : false);

draggable.addEventListener('touchend', function(event) {
//console.log("touch ended");
},passiveSupported
? { passive: true } : false);          
function setmidsteeringangle(trimwhichway){
if (trimwhichway=="left"){
steeringservomidangle = steeringservomidangle-5;
} else if (trimwhichway=="right"){
steeringservomidangle = steeringservomidangle+5;
}
document.getElementById("mylocalconsole").innerHTML="Mid="+steeringservomidangle;
}
function gotosteeringmidposition(){
draggable.style.left = '100px';
draggable.style.top = '-50px';


doSend("<W-"+steeringservomidangle+">");

}
//sliderbasefunction returns at what percent of total slidable height -is the slider at
function sliderbasefunction(event,whocalledme,whichelement){
var mytotalheight;
var myminY;
var mymaxY;
var mytouchY;
var mystylefromtop;
var mypercentageslide;
var myouterboundrect=document.getElementById('slidercolumn'+whichelement).getBoundingClientRect();
//var myslider = document.getElementById('speedcontrol'+whichelement);
var mysliderrect=whocalledme.getBoundingClientRect();
mytotalheight= myouterboundrect.height;
myminY = myouterboundrect.top+(mysliderrect.height/2);
mymaxY = myminY+(mytotalheight-mysliderrect.height);
var myslidertouch = event.targetTouches[0];
mytouchY = myslidertouch.pageY;
if (mytouchY >= myminY && mytouchY <= mymaxY)
{
//move whocalledme to mytouchY

mypercentageslide = ((mymaxY-mytouchY)/(mymaxY-myminY))*100;

if ((80-mypercentageslide)>0){mystylefromtop = 80-mypercentageslide;} else {mystylefromtop = 0;}
whocalledme.style.top=mystylefromtop+'%';
//console.log("mytouchY="+mytouchY+" myminY="+myminY+" mymaxy="+mymaxY+" mypercentageslide ="+mypercentageslide+" style from top % ="+mystylefromtop);



}
else {

//sliding out of bounds - lower
if (mytouchY >= myminY){
whocalledme.style.top=80+'%';
mypercentageslide=0;
}
//sliding out of bounds - upper
else if (mytouchY <= mymaxY){

whocalledme.style.top=0+'%';
mypercentageslide=100; 
}


}
return Math.round(mypercentageslide);
}
var mysliderA = document.getElementById('speedcontrolA');
var mysliderB = document.getElementById('speedcontrolB');
//servo sliders - somewhat similar to speed sliders so kept the same nomenclature
var mysliderX = document.getElementById('speedcontrolX');
var mysliderY = document.getElementById('speedcontrolY');
var mysliderZ = document.getElementById('speedcontrolZ');
var mypreviousPWMAsent = 0;
var mypreviousPWMBsent = 0;
var mypreviousvalueXsent=0;
var mypreviousvalueYsent=0;
var mypreviousvalueZsent=0;
var valuetosendA =0;
var valuetosendB =0;
var valuetosendX=0;
var valuetosendY=0;
var valuetosendZ=0;
var servoXmaxangle = 150;
var servoXminangle = 30;
var servoXanglesteps = 5;
var servoYmaxangle = 150;
var servoYminangle = 30;
var servoYanglesteps = 5;
var servoZmaxangle = 150;
var servoZminangle = 30;
var servoZanglesteps = 5;
var motorAmovingforward=false;
var motorAmoving=false;

mysliderA.addEventListener('touchmove',function(event){
var whatpercentslide = sliderbasefunction(event,this,"A");

//if slider is between 60-100% - move forward with increasing speed. 
//if slider is between 40-0% - move backwards with increasing speed.
//if slider is between 40-60% - stop.

if(!motorAmoving)
{


if (whatpercentslide<40){

motorAmoving=true;
motorAmovingforward=false;
doSend("<C-R>");


} else if (whatpercentslide>60) {

motorAmoving=true;
motorAmovingforward=true;
doSend("<C-F>");

}
else{


}

}

else {
//motorA is moving and we cross midline, wishing to change the direction
if (whatpercentslide<40){

if(motorAmovingforward){
doSend("<C-R>");
motorAmovingforward=false;
}
} else if (whatpercentslide>60) {

if(!motorAmovingforward){
doSend("<C-F>");
motorAmovingforward=true;
}   
}

}

valuetosendA= Math.round(PWMAMIN+((PWMAMAX-PWMAMIN)*((Math.abs(50-whatpercentslide)*2)/100)));

if (Math.abs(valuetosendA-mypreviousPWMAsent)>Number(PWMASTEPS)){


doSend("<S-"+valuetosendA+">");
//console.log("<S-"+valuetosendA+">");
mypreviousPWMAsent=valuetosendA;
}


},passiveSupported
? { passive: true } : false);

//when releasing speedcontrol button - it returns to mid position and motor stops.
mysliderA.addEventListener('touchend',function(event){
motorAmoving=false;
doSend("<C-X>");
doSend("<S-"+PWMAMIN+">");
mysliderA.style.top=40+'%';

},passiveSupported
? { passive: true } : false);

mysliderB.addEventListener('touchmove',function(event){
var whatpercentslide = sliderbasefunction(event,this,"B");

valuetosendB= Math.round(PWMBMIN+((PWMBMAX-PWMBMIN)*(whatpercentslide/100)));
//console.log("whatpercentslide = "+whatpercentslide+ "valuetosend ="+valuetosend+"PWMAMAX = "+PWMAMAX+ "PWMAMIN = "+PWMAMIN);
if (Math.abs(Number(valuetosendB)-Number(mypreviousPWMBsent))>Number(PWMBSTEPS)){


doSend("<s-"+valuetosendB+">");
//console.log("<s-"+valuetosendB+">");
mypreviousPWMBsent=valuetosendB;
}


},passiveSupported
? { passive: true } : false);     

mysliderX.addEventListener('touchstart',function(event){

attachordetachservos("x","a");

},passiveSupported
? { passive: true } : false);
mysliderX.addEventListener('touchend',function(event){

attachordetachservos("x","d");

},passiveSupported
? { passive: true } : false);
mysliderX.addEventListener('touchmove',function(event){
var whatpercentslide = sliderbasefunction(event,this,"X");

valuetosendX= Math.round(servoXminangle+((servoXmaxangle-servoXminangle)*(whatpercentslide/100)));
if (Math.abs(Number(valuetosendX)-Number(mypreviousvalueXsent))>Number(servoXanglesteps)){


doSend("<X-"+valuetosendX+">");
//console.log("<X-"+valuetosendX+">");
mypreviousvalueXsent=valuetosendX;
}


},passiveSupported
? { passive: true } : false); 
mysliderY.addEventListener('touchstart',function(event){

attachordetachservos("y","a");

},passiveSupported
? { passive: true } : false);
mysliderY.addEventListener('touchend',function(event){

attachordetachservos("y","d");

},passiveSupported
? { passive: true } : false);
mysliderY.addEventListener('touchmove',function(event){
var whatpercentslide = sliderbasefunction(event,this,"Y");

valuetosendY= Math.round(servoYminangle+((servoYmaxangle-servoYminangle)*(whatpercentslide/100)));
if (Math.abs(Number(valuetosendY)-Number(mypreviousvalueYsent))>Number(servoYanglesteps)){


doSend("<Y-"+valuetosendY+">");
//console.log("<Y-"+valuetosendY+">");
mypreviousvalueYsent=valuetosendY;
}


},passiveSupported
? { passive: true } : false); 
mysliderZ.addEventListener('touchstart',function(event){

attachordetachservos("z","a");

},passiveSupported
? { passive: true } : false);
mysliderZ.addEventListener('touchend',function(event){

attachordetachservos("z","d");

},passiveSupported
? { passive: true } : false);
mysliderZ.addEventListener('touchmove',function(event){
var whatpercentslide = sliderbasefunction(event,this,"Z");
valuetosendZ= Math.round(servoZminangle+((servoZmaxangle-servoZminangle)*(whatpercentslide/100)));
if (Math.abs(Number(valuetosendZ)-Number(mypreviousvalueZsent))>Number(servoZanglesteps)){


doSend("<Z-"+valuetosendZ+">");
//console.log("<Z-"+valuetosendZ+">");
mypreviousvalueZsent=valuetosendZ;
}


},passiveSupported
? { passive: true } : false); 
var onlongtouch; 
var timer;
var touchduration = 750; //length of time we want the user to touch before we do something

function touchstart(id,val) {
timer = setTimeout(onlongtouch, touchduration,id,val); 

}

function touchend() {

//stops short touches from firing the event
if (timer)
clearTimeout(timer); // clearTimeout, not cleartimeout..
}
/*
//if holding the steering knob for longer than 750 ms, send the current angle. 
onlongtouch=doSend("<W-"+myangle+">");*/


var websock;
var iswebsocketconnected=false;
function doConnect()
{
// websock = new WebSocket('ws://' + window.location.hostname + ':8000/');
websock = new WebSocket('ws://192.168.4.1:8000/');
websock.onopen = function(evt) 
{ 
console.log('websock open'); 
iswebsocketconnected=true;
if(smartbattery)
{
doSend("<A-D>"); //Send adjust PWM Digital on connecting }
} 
else 
{
doSend("<A-A>"); //Send adjust PWM  on connecting }

}
//attach the steeringservo

attachordetachservos("w","a");


}


websock.onclose = function(evt) 
{ 
iswebsocketconnected=false;
console.log('websock close'); 
//clear all the intervals - doesn't work. still dealing with "ghost" setIntervals
clearInterval(recurringbatteryinfo);
clearInterval(checkingobstacle);
clearInterval(checkingrpmA);
clearInterval(checkingrpmB);

}
websock.onerror = function(evt) 
{
console.log(evt); 
}
websock.onmessage = function(evt) 
{ 
console.log(evt); 
//console.log(evt.data);
parse_incoming_websocket_messages(evt.data);
}
}
function doClose()
{
//detach steering servo
attachordetachservos("w","d");

websock.close();
}
function doSend(message)
{
if(iswebsocketconnected==true)
{
if(websock.readyState==websock.OPEN){
websock.send(message);


}
else {

console.log("websocket is in an indeterminate state");
}
}
}
</script>
</html>
)rawliteral";


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
Serial1.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
switch(type) {
case WStype_DISCONNECTED:
Serial1.printf("[%u] Disconnected!\r\n", num);
break;
case WStype_CONNECTED:
{
IPAddress ip = webSocket.remoteIP(num);
Serial1.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
//itoa( cm, str, 10 );
//  webSocket.sendTXT(num, str, strlen(str));
}
break;
case WStype_TEXT:
{
Serial1.printf("[%u] get Text: %s\r\n", num, payload);
//Send whatever comes on the WS to Atmega.
char *mystring = (char *)payload;
Serial.print(mystring);

}
break;
case WStype_BIN:
Serial1.printf("[%u] get binary length: %u\r\n", num, length);
break;
default:
Serial1.printf("Invalid WStype [%d]\r\n", type);
break;
}
}

void handleRoot()
{
server.sendHeader("Cache-Control","max-age=604800");
server.send(200, "text/html", INDEX_HTML);

}
void handleIndex()
{
//server.send_P(200, "text/html", INDEX_HTML);
server.sendHeader("Cache-Control","max-age=604800");
server.send(200, "text/html", INDEX_HTML);

}


void handleManifest()
{
server.sendHeader("Cache-Control","max-age=604800");
server.send(200, "application/json", MANIFEST_JSON);

}
void handleCheck()
{
server.sendHeader("Cache-Control","max-age=604800");
server.send(200, "text/html", CHECK_SENSORS);

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


/*******************Serial Read Functions ************************/
//Serial Read stuff

const byte numChars = 32;
char receivedChars[numChars];



void recvWithStartEndMarkers() {
static boolean recvInProgress = false;
static byte ndx = 0;
char startMarker = '<';
char endMarker = '>';
char rc;

while (Serial.available() > 0 ) {
rc = Serial.read();

if (recvInProgress == true) {
if (rc != endMarker) {
receivedChars[ndx] = rc;
ndx++;
if (ndx >= numChars) {
ndx = numChars - 1;
}
}
else {
receivedChars[ndx] = '\0'; // terminate the string
recvInProgress = false;
webSocket.sendTXT(0,receivedChars,ndx);
Serial1.println(receivedChars);
Serial.println(receivedChars);
ndx = 0;

}
}

else if (rc == startMarker) {
recvInProgress = true;
}
}
}




/*******************Serial Read Functions ************************/

void setup()
{ 


Serial1.begin(115200);
delay(10);
Serial.begin(57600);
//Serial1.setDebugOutput(true);

Serial1.println();
Serial1.println();
Serial1.println();

for(uint8_t t = 4; t > 0; t--) {
Serial1.printf("[SETUP] BOOT WAIT %d...\r\n", t);
Serial1.flush();
delay(1000);
//Serial.print("<f-10000>");
}
/***************** AP mode*******************/
Serial1.print("Configuring access point...");
WiFi.softAP(ssid, password);
WiFi.printDiag(Serial1);
IPAddress myIP = WiFi.softAPIP();
Serial1.print("AP IP address: ");
Serial1.println(myIP);

/***********************************************/

/*****************Client Mode******************/
/*
WiFiMulti.addAP(ssid, password);
while(WiFiMulti.run() != WL_CONNECTED) {
Serial1.print(".");
delay(100);
}
Serial1.println("");
Serial1.print("Connected to ");
Serial1.println(ssid);
Serial1.print("IP address: ");
Serial1.println(WiFi.localIP());
*/
/**********************************************/

server.on("/", handleRoot);
server.on("/index.html",handleIndex);
server.on("/manifest.json",handleManifest);
server.on("/check.html",handleCheck);
server.onNotFound(handleNotFound);
//  server.onNotFound([]() {                              // If the client requests any URI
//    if (!handleFileRead(server.uri()))                  // send it if it exists
//      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
//  });

server.begin();

webSocket.begin();
webSocket.onEvent(webSocketEvent);


/* ************SPIFFS********************* */
//  if (SPIFFS.begin()){Serial1.println("file system mounted");};
//
//  //Open the "Save.txt" file and check if we were saving before the reset happened
//  File q = SPIFFS.open("/Save.txt", "r");
//  if (q.find("Y")){saveData=true;}
//  q.close();

/*********************************************/

/* ************OTA********************* */

// Port defaults to 8266
// ArduinoOTA.setPort(8266);

// Hostname defaults to esp8266-[ChipID]
// ArduinoOTA.setHostname("myesp8266");

// No authentication by default
// ArduinoOTA.setPassword((const char *)"123");

ArduinoOTA.onStart([]() {
Serial1.println("Start");
});
ArduinoOTA.onEnd([]() {
Serial1.println("\nEnd");
});
ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
Serial1.printf("Progress: %u%%\r", (progress / (total / 100)));
});
ArduinoOTA.onError([](ota_error_t error) {
Serial1.printf("Error[%u]: ", error);
if (error == OTA_AUTH_ERROR) Serial1.println("Auth Failed");
else if (error == OTA_BEGIN_ERROR) Serial1.println("Begin Failed");
else if (error == OTA_CONNECT_ERROR) Serial1.println("Connect Failed");
else if (error == OTA_RECEIVE_ERROR) Serial1.println("Receive Failed");
else if (error == OTA_END_ERROR) Serial1.println("End Failed");
});
ArduinoOTA.begin();

/****************************************************/  


}



void loop()
{
webSocket.loop();
server.handleClient();
ArduinoOTA.handle();
recvWithStartEndMarkers();
}
