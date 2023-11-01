# HardwareHackingVillagePH
Hardware Hacking Village Philippines Repository

This is for the HHV DIY modular badge sold and presented during Rootcon 17.

Feel free to play around with the code.

The wiring diagram has been uploaded here as well.

Contributions are most welcome.





## Overview

The HHV-RC17 Electronic Badge is a unique and educational device designed for learning about Infrared (IR) transmission, WiFi deauthentication, and captive portal technologies. This is a mix of code from stackoverflow, the original HHV badge code, and PhiSiFi (https://github.com/p3tr0s/PhiSiFi/).

## Features

- **Infrared Transmitter:** The badge is equipped with an IR transmitter that can send Power Off signals to TVs. 

- **WiFi Deauther:** The device can perform WiFi deauthentication attacks. 

- **Evil-Twin:** Learn about evil-twin attacks and its applications. 


## Disclaimer

This badge is intended for educational and informational purposes only. The creators and maintainers of this project are not responsible for any misuse or illegal activities that may be carried out with it. Users are expected to comply with all applicable laws and regulations.


## Getting Started

1. Install Arduino IDE
2. In Arduino go to File -> Preferences add this URL to Additional Boards Manager URLs -> https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json
3. In Arduino go to Tools -> Board -> Boards Manager search for and install the deauther and IRremote package
4. Download and open this repo with Arduino IDE
5. Select an ESP8266 Deauther board in Arduino under tools -> board
6. Connect your device and select the serial port in Arduino under tools -> port
7. Click Upload button


## How To Use
1. After powering up the badge, you will see the RGB light turn blue. This indicates that the wifi is on and you may connect to the Access Point named "HHV-Badge" with password "12345678" from your phone/computer. You can change the SSID and Password at lines 17 and 18 of the HHV-RC17-Badge.ino file.
2. Select the target AP you want to attack (list of available APs refreshes every 30secs - page reload is required).
3. Click the "Start Deauthing" button to start kicking devices off the selected network. The RGB module will turn purple to indiciate that Deauthing attacks are running.
4. Click the "Start Evil-Twin" button and optionally reconnect to the newly created AP named same as your target (will be open). The RGB module will turn green when the Evil-Twin attack is running.
5. You can stop the Deauthing and Evil-Twin attacks by pushing the red button once or by resetting the ESP8266.
6. Once a correct password is found, AP will be restarted with default ssid HHV-Badge / 12345678 and at the bottom of a table you should be able to see something like "Successfully got password for - TARGET_SSID - PASSWORD
7. If you power down / hard reset the gathered info will be lost.
8. You can turn off the wifi to save battery by tapping on the "Turn Off Wifi" button after connecting to the access point.
9. To turn on the wifi, simply reset the ESP8266 by pressing the button on the left side of the micro usb cable.
10. Tapping on the red button on the badge sends the infrared signals to turn off TV's. You can run only run infrared attacks when the RGB module is blue, or turned off. If the RGB module is green or purple, this means that wifi attacks are running and pressing the red  button stops the attacks.
11. Wait 5 minutes after stopping the deauth or evil twin attacks to reconnect back to the badge's AP.


## Acknowledgments

We would like to express our gratitude to the Rootcon community for their support and inspiration in the creation of this badge.

Please use this badge responsibly and in a manner that respects the rights and privacy of others.


Changelog
v2.0
- added Wifi Deauthentication and Evil-Twin attacks

v1.0.1
- changed RGB color to red when sending codes
- replaced existing TV Power codes with codes from schiaparelli of Lockpick Village's flipperzero
