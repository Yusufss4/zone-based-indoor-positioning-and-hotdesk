[![Contributors][contributors-shield]][contributors-url] [![Forks][forks-shield]][forks-url] [![Stargazers][stars-shield]][stars-url] [![Issues][issues-shield]][issues-url]

<!-- PROJECT LOGO -->
<br/>
<p align="center">
  <a href="https://github.com/Yusufss4/Academy2021">
    <img src="Demo-Files/Assets/HardwareLogo.png" alt="Logo" width="140" height="140">
  </a>
  <h1 align="center">Intership Project Indoor Positioning and Hotdesk</h1>
  <p align="center">
    Borda Academy 2021 Project Co-Working Smart Office Hardware Part
    <br/>
    <a href="https://youtu.be/FV2so5sVpH8">Watch Video</a>
    ·
    <a href="https://github.com/Yusufss4/Academy2021/issues">Report Bug</a>
    ·
    <a href="https://github.com/Yusufss4/Academy2021/issues">Request Feature</a>
  </p>
   <p align="center">
    Index
    <br/>
    <a href="#about-the-project">About the Project</a>
    ·
    <a href="#products">Products</a>
    ·
    <a href="#Required-Parts-(BOM)">Part List</a>
        ·
    <a href="#How-to-Build-and-Run">How to Run</a>
            ·
      <a href="#MQTT-Connections-/-Topics">MQTT</a>
            ·
    <a href="#-Team-Behind-the-Hardware">Team</a>
  </p>
</p>




# About the Project

In this project there are three different task in the hardware section. These are;

 1. Capacity Calculation Per Room. Detecting a person if they are in the room and send information according to that.
 2. Capacity Calculation Per Office. Detecting a person if they are in the office or not using gateways. 
 3. Hotdesk and Meeting Room Scheduling. Controlling e-papers according to the information coming from the server. 
<p align="center">
<img src="Hardware-Schematics/HardwareProducts.png" alt="Project Diagram" width="700">
</p>

# Products

There are five different products in the project but only four of them can be coded and one of them can be only configured. 

## ESP32 - In Room Scanner

### Explanation
ESP32 - In Room Scanner first scans the bluetooth devices and creates a buffer filled with RSSI signals and MAC addresses of the scanned devices. Later duplicate signals are being averaged and removed from the buffer and passed to the new buffer. Later in the publish MQTT function this buffer is transformed to JSON format and sent along with the MAC address of the device itself to differentiate the rooms (devices) from each other. This information will be processed by the server application and with the threshold value determined for each room the server will decide if the person is in the room and if it is in the room which person it is and send the number of people in the room information to the /nrom topic.

### Connection Diagram
<p align="center">
 <img src="DiagramFiles/ESP32-Scanner_bb.png" alt="Connection Diagram" width="400">
 </p>

### Box Desing
<p align="center">
<img src="BoxDesigns/Images/Scanner.png" alt="Box Desing" width="700">
</p>
 
## ESP8266 Door Lighting Controller

ESP8266 always listens to the topic /nrom and whenever a message comes first by looking at the MAC addresses sent with the message it decides if the message is intended for it or not. If it is intended for that specific device according to the number of people information in the message it lights the NeoPixel LEDs. If the capacity is reached all LEDs will light in the color of red.

### Connection Diagram
<p align="center">
 <img src="DiagramFiles/ESP8266-NoLvlNoDHT_bb.png" alt="ESP8266 Door Lighting Controller Connection Diagram" width="400">
 </p>

### Box Desing
<p align="center">
<img src="BoxDesigns/Images/Door.png" alt="ESP8266 Door Lighting Controller Box Desing" width="700"> 
</p>

## ESP32 - E-Paper Controller

E-paper controller ESP32 constantly listens to the publishments coming to the MQTT topic to which it is subscribed. When there is Publishment, it runs the bluetooth functions and does a scan. When it matches the UUID advertised by the shelf label with the UUID data received by MQTT, it establishes a connection with the shelf label. If the connection is successful, it sends the data to the electronic shelf label. If not, it will scan again and try to connect. MQTT sends a warning message to the broker if it cannot find a UUID value to match by scanning. 

### Connection Diagram
<p align="center">
 <img src="DiagramFiles/ESP32-ShelfLabelController_bb.png" alt="Connection Diagram" width="400">
  </p>

### Box Desing
<p align="center">
<img src="BoxDesigns/Images/E-paper-controller.png" alt="Box Desing" width="700">
 </p>

## E-Paper

Electronic Shelf Label is ready for a connection request by ESP32 by constantly making advertisements. The shelf labels used for the hot desk have two different screens. Reserved and Available screens. According to the data received by ESP32, the shelf label display is updated. The shelf labels used for the Meeting Room also have two different screens. Busy and Next event screens. According to the data received by ESP32, the shelf label display is updated.

![resim](https://user-images.githubusercontent.com/44584158/132695852-41f96476-604e-4485-b37b-de702ea20266.png)

### Connection Diagram
<p align="center">
 <img src="images/logo.png" alt="E-Paper Connection Diagram" width="400">
 </p>

### Box Desing
<p align="center">
<img src="BoxDesigns/Images/ShelfLabel_1.png" alt="E-Paper Box Desing" width="700">
<img src="BoxDesigns/Images/ShelfLabel_2.png" alt="E-Paper Box Desing" width="700">
</p>

# Required Parts (BOM)

All the required parts are determined for 2 Hotdesk, 2 Beacon, 2 Room and one Office. If you want to increase the number of rooms that you want to install you have to multiply the ESP8266 Door Light and ESP32 In Room Scanner equipments by two. Each room has one ESP8266 and one ESP32.

## BOM For ESP8266 - Door Lighting Controller

| Name                                 | How                         | Piece | How Many          | Where         |
|--------------------------------------|-----------------------------|-------|-------------------|---------------|
| ESP32                                | Main Controller             | 1     | 1 Per Room/Device | Near the door |
| NeoPixel 24 Ring                     | Capacity Indicator          | 1     | 1 Per Room/Device | Near the door |
| 5V Adapter                           | Power for ESP32             | 1     | 1 Per Room/Device | Near the door |
| 4x6cm Soldering Prototype Board      | ESP32 Placement             | 1     | 1 Per Room/Device | Near the door |
| 470R 1/4W Resistor                   | NeoPixel Signal Resistor    | 1     | 1 Per Room/Device | Near the door |
| 5.5x2.21mm DC Power Jack             | Power for ESP32             | 1     | 1 Per Room/Device | Near the door |
| 1x40 Male Header 80 Pin 90 Degree    | ESP32 Placement             | 1     | 1 Per Room/Device | Near the door |
| 1x40 Female Header 40 Pin 180 Degree | ESP32 Placement             | 1     | 1 Per Room/Device | Near the door |
| 1000uF Capacitor                     | NeoPixel Current Protection | 1     | 1 Per Room/Device | Near the door |
| 6x6x5 Push Button                    | WiFi Access Point Reset     | 1     | 1 Per Room/Device | Near the door |
| Blue LED                             | MQTT Indicator              | 1     | 1 Per Room/Device | Near the door |
| 3D Printed Box Upper-Piece           | Box Upper                   | 1     | 1 Per Room/Device | Near the door |
| 3D Printed Box Lower-Piece           | Box Base Plate              | 1     | 1 Per Room/Device | Near the door |
| 50mm Lens or Glass                   | To See Onboard Leds         | 1     | 1 Per Room/Device | Near the door |

## BOM For ESP32 - In Room Scanner
| Name                                 | How                        | Piece | How Many          | Where           |
|--------------------------------------|----------------------------|-------|-------------------|-----------------|
| ESP32 DevkitV4                       | Main Controller            | 1     | 1 Per Room/Device | Inside the Room |
| ESP32 Taoglas 2.4Ghz                 | External Antenna for ESP32 | 1     | 1 Per Room/Device | Inside the Room |
| 5V Adapter                           | Power for ESP32            | 1     | 1 Per Room/Device | Inside the Room |
| 4x6cm Soldering Prototype Board      | ESP32 Placement            | 1     | 1 Per Room/Device | Inside the Room |
| 5.5x2.21mm DC Power Jack             | Power for ESP32            | 1     | 1 Per Room/Device | Inside the Room |
| 1x40 Male Header 80 Pin 90 Degree    | ESP32 Placement            | 1     | 1 Per Room/Device | Inside the Room |
| 1x40 Female Header 40 Pin 180 Degree | ESP32 Placement            | 1     | 1 Per Room/Device | Inside the Room |
| 6x6x5 Push Button                    | WiFi Access Point Reset    | 1     | 1 Per Room/Device | Inside the Room |
| LEDs                                 | Status LEDs                | 3     | 1 Per Room/Device | Inside the Room |
| 3D Printed Box Upper-Piece           | Box Upper                  | 1     | 1 Per Room/Device | Inside the Room |
| 3D Printed Box Lower-Piece           | Box Base Plate             | 1     | 1 Per Room/Device | Inside the Room |

## BOM For ESP32 - E-Paper Controller
| Name                                 | How                     | Piece | How Many          | Where           |
|--------------------------------------|-------------------------|-------|-------------------|-----------------|
| ESP32 DevkitV4                       | Main Controller         | 1     | 1 Per Room/Device | Inside the Room |
| 5V Adapter                           | Power for ESP32         | 1     | 1 Per Room/Device | Inside the Room |
| 4x6cm Soldering Prototype Board      | ESP32 Placement         | 1     | 1 Per Room/Device | Inside the Room |
| 5.5x2.21mm DC Power Jack             | Power for ESP32         | 1     | 1 Per Room/Device | Inside the Room |
| 1x40 Male Header 80 Pin 90 Degree    | ESP32 Placement         | 1     | 1 Per Room/Device | Inside the Room |
| 1x40 Female Header 40 Pin 180 Degree | ESP32 Placement         | 1     | 1 Per Room/Device | Inside the Room |
| 6x6x5 Push Button                    | WiFi Access Point Reset | 1     | 1 Per Room/Device | Not Added Yet   |
| LEDs                                 | Status LEDs             | 3     | 1 Per Room/Device | Not Added Yet   |
| 3D Printed Box Upper-Piece           | Box Upper               | 1     | 1 Per Room/Device | Inside the Room |
| 3D Printed Box Lower-Piece           | Box Base Plate          | 1     | 1 Per Room/Device | Inside the Room |

## Other Parts
| Name                   | How                   | Piece | How Many       | Where              |
|------------------------|-----------------------|-------|----------------|--------------------|
| MTag2901 2.9" ESL Tag  | Room Status Screen    | 1     | 1 per Room     | Near the door      |
| MTag2901 2.7" ESL Tag  | Hotdesk Status Screen | 2     | 1 per Table    | On the table       |
| CR2450 3V Battery      | Battery for ESL       | 2     | 2 per ESL      | In ESL             |
| CR2477 3V Battery      | Battery for E7 Beacon | 2     | 1 per People   | In Beacon          |
| G1-C Gateway           | BLE Office Covarage   | 1     | 1 per Office   | Middle of Office   |
| E7 Plus Beacon         | BLE Advertisement     | 2     | 1 per People   | On People's Pocket |
| Extension cable        | To Extend Power       | 2     | As you require | Near Power         |


# How to Build and Run

## ESP32 In-Room Scanner and ESP8266 Door Light Controller
1. You can run the project without all the Leds, power adapters, buttons, cables etc. just by uploading code to the necessary controller boards like ESP32 and ESP8266.
2. Download the code for each device. Each document is divided in the files too.
3. Download the ESP32 and ESP8266 Packages for board manager in Arduino IDE.
4. For ESP32 click Tools->Partition Scheme->Huge APP (3MB/No OTA) so that code can compile.
5. You have to use a MQTT Broker for publishing and subscribing topics. From the MQTT Config change the connections to whatever broker you are using.
6. If you are using a broker that requires a certificate uncomment the certificate and transform the **WifiClient** to **WiFiClientSecure**so that ESP32 or ESP8266 can transfer the certificates. ,
7. Pull the Button Pin to the 3.3V if you did not connect the button so that WiFi manager does not reset the WiFi credentials.
8. ESP32 In-Room Scanner and ESP8266 uses WifiManager so to connect the devices to the WiFi, if the device is not connected the Access Point will be opened, first you should connect to the wifi network created by device, name will be Fora - [Mac Address of Device] and later a web page application will be opened if you connect to that Access Point. Later you can connect to any wifi.
9. After connecting both ESP32 and ESP8266 to WiFi and MQTT Broker you can test the device using the MQTTLens application.
10. If you made all the connections like buttons, leds, neopixel etc. you can test the PCB by running the code PCBTester in the Demo files.

## ESP32 E-Paper Controller and E-Paper

1.
2.

# MQTT Connections / Topics

 - Topic and Data Type documentation can be reached from [here.](https://docs.google.com/document/d/1uNCvFoLJsAC_Qh4L8_6ozjhal6APVGUYTeoTuyXoGE8/edit?usp=sharing)
 - To connect the devices you have to use a MQTT Broker. For the connections steps please follow How to Run steps.
>Some of the alternatives for the MQTT Broker: 
> - Deployed RabbitMQ 
> - Adafruit IO
> - EMQX
> - HiveMQ (Not Stable)

General Diagram showing the communication can be seen below.
<p align="center">
<img src="Hardware-Schematics/MQTT-Diagram.png" alt="MQTT Diagram" width="700">
</p>

# Team Behind the Hardware

<p align="center">
<a href="https://www.linkedin.com/in/yusufss/">
 <img title="Yusuf Savaş" src="Demo-Files/Assets/YusufSavaş.png" width="100" alt="Yusuf Savaş"/> </a> 
 <a href="https://www.linkedin.com/in/utkuurkun/">
  <img title="Utku Urkun" src="Demo-Files/Assets/UtkuUrkun.png" width="100" alt="Utku Urkun" /> </a>
  <a href="https://www.linkedin.com/in/ata-korkusuz-77707118b/">
  <img title="Ata Korkusuz" src="Demo-Files/Assets/AtaKorkusuz.png" width="100" alt="Ata Korkusuz" /> </a>
</p>

<p align="center">
<a href="https://www.linkedin.com/in/berkay-arslan-3b9b95141/">
  <img title="Berkay Arslan" src="Demo-Files/Assets/BerkayArslan.png" width="100" alt="Berkay Arslan" /> </a> 
  <a href="https://www.linkedin.com/in/zeynepbilge/">
  <img title="Zeynep Bilge" src="Demo-Files/Assets/ZeynepBilge.png" width="100" alt="Zeynep Bilge" /> </a>
  <a href="https://www.linkedin.com/in/emine-acar-8036639b/">
  <img title="Emine Acar" src="Demo-Files/Assets/EmineAcar.png" width="100" alt="Emine Acar" /> </a>
</p>

<p align="center">
<a href="https://www.linkedin.com/in/eymen-topcuoglu/">
  <img title="Eymen Topçuoğlu" src="Demo-Files/Assets/EymenTopçuoğlu.png" width="100" alt="Eymen Topçuoğlu" /> </a>
</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/Yusufss4/Academy2021.svg?style=for-the-badge
[contributors-url]: https://github.com/Yusufss4/Academy2021/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/Yusufss4/Academy2021.svg?style=for-the-badge
[forks-url]: https://github.com/Yusufss4/Academy2021/network/members
[stars-shield]: https://img.shields.io/github/stars/Yusufss4/Academy2021.svg?style=for-the-badge
[stars-url]: https://github.com/Yusufss4/Academy2021/stargazers
[issues-shield]: https://img.shields.io/github/issues/Yusufss4/Academy2021.svg?style=for-the-badge
[issues-url]: https://github.com/Yusufss4/Academy2021/issues
[license-shield]: https://img.shields.io/github/license/Yusufss4/Academy2021.svg?style=for-the-badge
[license-url]: https://github.com/Yusufss4/Academy2021/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/yusufss
[product-screenshot]: images/screenshot.png
