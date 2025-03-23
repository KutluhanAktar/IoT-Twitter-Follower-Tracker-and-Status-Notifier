         /////////////////////////////////////////////  
        //   IoT Twitter Follower Tracker and      // 
       //           Status Notifier               //
      //           -----------------             //
     //          (Arduino Nano 33 IoT)          //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

// 
// Via Nano 33 IoT, display the follower count of a given Twitter account and its current follower status - STABLE, DEC, or INC.
//
// For more information:
// https://www.theamplituhedron.com/projects/IoT_Twitter_Follower_Tracker_and_Status_Notifier
//
//
// Connections
// Arduino Nano 33 IoT :           
//                                Nokia 5110 Screen
// D2  --------------------------- SCK (Clk)
// D3  --------------------------- MOSI (Din) 
// D4  --------------------------- DC 
// D5  --------------------------- RST
// D6  --------------------------- CS (CE)
//                                JoyStick
// A0  --------------------------- VRY
// A1  --------------------------- VRX
// D9  --------------------------- SW
// 5V  --------------------------- 5V
// GND --------------------------- GND
//                                Button (Right)
// A3  --------------------------- S
//                                Button (Left)
// A2  --------------------------- S
//                                RGB LEB (RCGB)
// D12 --------------------------- R
// D11 --------------------------- G
// D10 --------------------------- B
//                                Buzzer
// D8  --------------------------- +



// Include required libraries:
#include <SPI.h>
#include <WiFiNINA.h>
#include <LCD5110_Graph.h>

char ssid[] = "<SSID>";        // your network SSID (name)
char pass[] = "<PASSWORD>";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;              // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

// Enter the IPAddress of your Raspberry Pi.
IPAddress server(192, 168, 1, 20);

// Initialize the Ethernet client library
WiFiClient client; /* WiFiSSLClient client; */

// Define screen settings.
LCD5110 myGLCD(2,3,4,5,6);

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
// Define the graphics:
extern uint8_t twitter[];

// Define controls:
#define right A3
#define left A2

// Define the joystick pinout.
#define VRY A0
#define VRX A1
#define SW 9

// Define RGB LED pins.
#define redPin 12
#define greenPin 11
#define bluePin 10

// Define the buzzer pin.
#define buzzer 8

// Define menu options:
volatile boolean followers = false, entries = false, graphics = false, _sleep = false, activated = false;

// Define the data holders:
int _right, _left, joystick_x, joystick_y, joystick_sw, selected = 0, x = 0;

// Define the Twitter Account Information:
String follower_count, account_name, account_id;

// Define status variables:
String _status = "None";
int previous_count = 0, entry_1 = 0, entry_2 = 0, entry_3 = 0;

void setup() {
  Serial.begin(9600);
  
  // Control pin settings:
  pinMode(right, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(SW, INPUT);
  digitalWrite(SW, HIGH);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  
  // Initiate screen.
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

  // Check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) { myGLCD.print("Connection Failed!", 0, 8); myGLCD.update(); while (true); }
  // Attempt to connect to the WiFi network:
  while (status != WL_CONNECTED) {
    myGLCD.print("Waiting...", 0, 8);
    myGLCD.print("Attempting to", 0, 16);
    myGLCD.print("connect to", 0, 24);
    myGLCD.print("WiFi !!!", 0, 32);
    myGLCD.update();
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // Wait 10 seconds for connection:
    delay(10000);
  }

  // Verify connection on the Nokia 5110 Screen.
  myGLCD.clrScr();
  myGLCD.print("Connected to", 0, 8);
  myGLCD.print("WiFi!!!", 0, 16);
  myGLCD.update();
  delay(2000);
  myGLCD.clrScr();
  myGLCD.update();
}

void loop() {
  read_controls();
  change_menu_options();
  interface();

  if(followers == true){
    do{
      myGLCD.invertText(true);
      myGLCD.print("A. Followers", 0, 16);
      myGLCD.invertText(false);
      myGLCD.update();
      adjustColor(255, 0, 255);
      delay(100);
      if(joystick_sw == false){
        myGLCD.clrScr();
        myGLCD.update();
        activated = true;
        // Get the Twitter Account Information from the REST API (CDN):
        make_a_get_request("/Twitter_Follower_Tracker/");
        // Check the follower status using registered status variables:
        int _followers = follower_count.toInt();
        if(_followers != previous_count){
          // If there is an increase or decrease in the follower number, notify the user:
          if(_followers > previous_count){ _status = "INC"; adjustColor(0, 255, 0); tone(buzzer, 500); }
          if(_followers < previous_count){ _status = "DEC"; adjustColor(255, 0, 0); tone(buzzer, 500); }
          // Change the last entries list:
          previous_count = _followers;
          entry_3 = entry_2;
          entry_2 = entry_1;
          entry_1 = _followers;
        }else{
          _status = "STABLE";
          adjustColor(0, 0, 255);
        }
        myGLCD.clrScr();
        myGLCD.update();
        while(activated == true){
          read_controls();
          myGLCD.print("Status: " + _status, 0, 0);
          myGLCD.print(follower_count, CENTER, 16);
          scrolling_text(" Name: " + account_name + " ", 32);
          scrolling_text(" ID:" + account_id + " ", 40);
          // Halt notification.
          if(_left == false) noTone(buzzer);
          // Exit.
          if(_right == false){ activated = false; x = 0; myGLCD.clrScr(); myGLCD.update(); noTone(buzzer); }         
        }
      }
    }while(followers == false);
  }

    if(entries == true){
    do{
      myGLCD.invertText(true);
      myGLCD.print("B. Entries", 0, 24);
      myGLCD.invertText(false);
      myGLCD.update();
      adjustColor(0, 255, 255);
      delay(100);
      if(joystick_sw == false){
        myGLCD.clrScr();
        myGLCD.update();
        activated = true;
        while(activated == true){
          read_controls();
          // Print the entries list:
          myGLCD.print("Entries: ", 0, 0);
          myGLCD.print("1 => : " + String(entry_1), 0, 16);
          myGLCD.print("2 => : " + String(entry_2), 0, 24);
          myGLCD.print("3 => : " + String(entry_3), 0, 32);
          myGLCD.update();
          // RESET THE ENTRIES LIST:
          if(_left == false){ entry_1 = 0; entry_2 = 0; entry_3 = 0; myGLCD.clrScr(); myGLCD.update(); }
          // Exit.
          if(_right == false){ activated = false; myGLCD.clrScr(); myGLCD.update(); }         
        }
      }
    }while(entries == false);
  }
  
  if(graphics == true){
    do{
      myGLCD.invertText(true);
      myGLCD.print("C. Graphics", 0, 32);
      myGLCD.invertText(false);
      myGLCD.update();
      adjustColor(255, 255, 0);
      delay(100);
      if(joystick_sw == false){
        myGLCD.clrScr();
        myGLCD.update();
        activated = true;
        while(activated == true){
          read_controls();
          // Define and print monochrome images:
          myGLCD.drawBitmap(20,0,twitter,45,45);
          myGLCD.update();
          // Exit.
          if(_right == false){ activated = false; myGLCD.clrScr(); myGLCD.update(); } 
        }
      }
    }while(graphics == false);
  }

 if(_sleep == true){
   do{
     myGLCD.invertText(true);
     myGLCD.print("D. Sleep", 0, 40);
     myGLCD.invertText(false);
     myGLCD.update();
     adjustColor(255, 69, 0);
     delay(100);
     if(joystick_sw == false){
       // Activate the sleep mode in 10 seconds.
       myGLCD.clrScr();
       myGLCD.print("Entering", CENTER, 0);
       myGLCD.print("Sleep Mode", CENTER, 8);
       myGLCD.print("in", CENTER, 16);
       myGLCD.print("Seconds", CENTER, 40);
       myGLCD.update();
       // Print remaining seconds.
       myGLCD.setFont(MediumNumbers);
       for (int s=10; s>=0; s--){ myGLCD.printNumI(s, CENTER, 24, 2, '0'); myGLCD.update(); delay(1000); }
       myGLCD.enableSleep();
       activated = true;
       while(activated == true){
         // Color Pattern:
         adjustColor(255,0,0);
         delay(500);
         adjustColor(0,255,0);
         delay(500);
         adjustColor(0,0,255);
         delay(500);
         adjustColor(255,0,255);
         delay(500);
         adjustColor(255,255,255);
         delay(500);
         // Exit.
         read_controls();
         if(_right == false){ activated = false; myGLCD.clrScr(); myGLCD.disableSleep(); myGLCD.setFont(SmallFont); adjustColor(0,0,0); }
       }
     }
   }while(_sleep == false);
 }
  
}

void read_controls(){
  // Joystick:
  joystick_x = analogRead(VRX);
  joystick_y = analogRead(VRY);
  joystick_sw = digitalRead(SW);
  // Buttons:
  _right = digitalRead(right);
  _left = digitalRead(left);
}

void interface(){
   // Define options.
   myGLCD.print("Twitter:", 0, 0);
   myGLCD.print("A. Followers", 0, 16);
   myGLCD.print("B. Entries", 0, 24);
   myGLCD.print("C. Graphics", 0, 32);
   myGLCD.print("D. Sleep", 0, 40);
   myGLCD.update();
}

void change_menu_options(){
  // Increase or decrease the option number using the joystick (VRY).
  if(joystick_y >= 900) selected--; 
  if(joystick_y <= 45) selected++; 
  if(selected < 0) selected = 4;
  if(selected > 4) selected = 1;
  delay(100);

  // Depending on the selected option number, change the boolean status.
  switch(selected){
    case 1:
      followers = true;
      entries = false;
      graphics = false;
      _sleep = false;
    break;
    case 2:     
      followers = false;
      entries = true;
      graphics = false;
      _sleep = false;
    break;
    case 3:
      followers = false;
      entries = false;
      graphics = true;
      _sleep = false;
    break;
    case 4:
      followers = false;
      entries = false;
      graphics = false;
      _sleep = true;
    break;
  }
}

void make_a_get_request(String application){
  // Connect to the web application named Twitter_Follower_Tracker. Change '80' with '443' if you are using SSL connection.
  if(client.connect(server, 80)){
    // If successful:
    myGLCD.print("Connected to", 0, 8);
    myGLCD.print("the server!!!", 0, 16);
    myGLCD.update();
    // Make an HTTP Get request:
    client.println("GET " + application + " HTTP/1.1");
    client.println("Host: 192.168.1.20");
    client.println("Connection: close");
    client.println();
  }else{
    myGLCD.print("Connection", 0, 8);
    myGLCD.print("Error!!!", 0, 16);
    myGLCD.update();
  }
  delay(2000); // Wait 2 seconds after connection...
  // If there are incoming bytes available, get the response from the web application.
  String response = "";
  while (client.available()) { char c = client.read(); response += c; }
  if(response != "" && response.endsWith("%")){
    // Split the response string by a pre-defined delimiter in a simple way. '%'(percentage) is defined as the delimiter in this project.
    int delimiter_1, delimiter_2, delimiter_3, delimiter_4;
    delimiter_1 = response.indexOf("%");
    delimiter_2 = response.indexOf("%", delimiter_1 + 1);
    delimiter_3 = response.indexOf("%", delimiter_2 + 1);
    delimiter_4 = response.indexOf("%", delimiter_3 + 1);
    // Glean information as substrings.
    follower_count = response.substring(delimiter_1 + 1, delimiter_2);
    account_name = response.substring(delimiter_2 + 1, delimiter_3);
    account_id = response.substring(delimiter_3 + 1, delimiter_4);
  }
}

void scrolling_text(String text, int y){
  int len = text.length();
  // Scroll text using the joystick (VRX).
  if(joystick_x <= 45) x--;
  if(joystick_x >= 900) x++;
  if(x>84) x = 84;
  if(x<=-(len*6)) x = -(len*6);
  // Print.
  myGLCD.print(text, x, y);
  myGLCD.update();
  delay(25);
}

void adjustColor(int red, int green, int blue){
 analogWrite(redPin, red);
 analogWrite(greenPin, green);
 analogWrite(bluePin, blue);
}
