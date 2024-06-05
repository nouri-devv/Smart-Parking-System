# Smart-Parking-System

The "Car_Spot_Arduino" folder includes the arduino nano 33 IoT code.\
The "Web-Server" folder includes the NodeJS and Express app code for the web server.
Ensure that the Nano 33 and Raspi is running on the same netwoek for demo purposes.




Background

Creating a Smart Parking System that uses sensors, LEDs, a booking page and a Raspberry Pi server in harmony.
The system functions by pre-booking a parking spot, which allocates a spot for the user.  The user is sent a verification code to use upon arriving at the site, which opens the gate for the user. The LEDs and sensors are used to determine if the user is still parking. Upon leaving, the number of free spots is updated.

Existing Work

There are some companies that do similar work such as Smart Parking Limited. However, their system does not incorporate a pre-booking system, so users may arrive at the parking site only to not find any parking left.

Problem Statement

“How to ensure that there are parking spots available before driving to the parking site.”
The above problem statement focuses on the availability of parking spots, to ensure that users are able to know if there are spots available to book before driving to the parking site.

Requirements 

Software: VS Code, JavaScript, NodeJS (JavaScript Framework), Express (JavaScript Framework), Node Mailer (JavaScript Library), FS, Body Parser (JavaScript Library), Path, Arduino IDE, C++, Arduino HTTP Client (Arduino Library), WIFI Nina (Arduino Library), Arduino Json (Arduino Library).

Hardware: Raspberry Pi 4/5, Arduino Nano 33 IoT, Ultra Sonic Sensor, Breadboard, Jumper wires, Green and red LEDs, Resistors.
