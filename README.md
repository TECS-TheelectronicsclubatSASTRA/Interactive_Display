***Wireless 7-Segment interactive display using MAX-7219 and Esp-32***



An interactive display system which replicate the pattern drawn or letters written by the user using a webpage

the webpage can be accessed in any remote device connected in the same network as the Esp-32

This gives us a real-time interaction between the user and the array of 7-segment modules

The Esp-32 acts as the central microcontroller and the Max-7219 Modules allow us to configure each single led present in the array



***Key Features:***

* web-based interactive drawing and writing interface
* Wireless communication through Esp-32 Wi-Fi
* MAX-7219 modules connected in Daisy chain
* Individual LED Control
* Real-time interaction



***Architecture and Working:***



The ESP32 creates its own Wi-Fi network and hosts a custom web interface containing an interactive drawing canvas.



The user connects a smartphone or laptop to the ESP32's Wi-Fi network and draws on the canvas.



The ESP32 receives the drawing information, maps the input to the corresponding LED segments, and sends the required data through the MAX7219 daisy chain.



The physical 7-segment display array then reproduces the user's input in real time.





Draw on Canvas in webpage

&#x09;↓ 

Wi-Fi Transmission 

&#x09;↓ 

ESP32 Processing 

&#x09;↓

Segment Mapping 

&#x09;↓

MAX7219 Drivers 

&#x09;↓ 

Physical Display







***Hardware:***





|Component|Quantity|
|-|-|
|Remote device|1|
|Esp-32 Microcontroller|1|
|Max-7219 Display driver modules|4/8|
|DC Power Supply|1|
|5V Buck converter|1|
|Jumper Wires|required|





***Software:***



The coding part is done using Arduino IDE (C++)



The Drawing canvas was created as a webpage and the html code is embedded along with the code for the 7-segment modules



The code uses the WiFi, WebServer, MD\_MAX72xx, and SPI libraries







***Project Implementation:***



1\. Clone the Repository:

git clone https://github.com/YOUR-USERNAME/Interface-Matrix-System.git



2\. Open the Firmware:

Open the Arduino project in Arduino IDE.



3\. Install Required Libraries:

Install the libraries required by the firmware:



WiFi

WebServer

MD\_MAX72xx

SPI



4\. Configure the ESP32:

Select the appropriate ESP32 board and COM port in Arduino IDE.



5\. Upload the Firmware:

Upload the firmware to the ESP32.



6\. Connect to the Display:

After the ESP32 starts its Access Point, connect your smartphone or laptop to the configured Wi-Fi network.

Open the IP address provided by the ESP32 in a web browser to access the interactive interface.









***Future scope:***



* Advanced web control interface
* More flexible drawing functionality
* Enhanced Real-time interaction
* Improved mobile interface









***Team:***



This project was developed covering 





* Embedded Hardware
* Firmware Development
* IoT \& Web Interface



presented by 

&#x09;Rishi A

&#x09;Kaaviyaa sri

&#x09;Akshaya T





***License:***



The project is developed for Educational and project purpose

