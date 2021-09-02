# morse_keyer
 Morse keyer by ESP12e / TCP host
### Brief description
In view of beginning to operate in HF via QRP transmitter using CW, rather than buying a cw key or paddle I thought it was worthwhile to build a cw keyer based on 8266-12 microcontroller hosting a TCP server to receive data to be converted / transmitted from a TCP client such as Putty connected "raw telnet" running on PC. 

The keyer would receive input data from PC, it would convert those in morse code to drive a passive buzzer tone at 800Hz to make audible codes driving also a relais or transistor wired to the transmitter's cw key input. An Oled-0.92" display is provided to display the characters being played by the buzzer to help the operator to familiarize with the morse code.

In addition to the usage with an actual transmitter, this keyer can be used to exercise own morse receiving ability thanks to the audible tones supplied by a passive buzzer directly connected to PIO D5 (PIO 14) on ESP12e.

The transmit speed can be set at various speed such as 5,10,15,20,25,30 WPM. The default speed is set at 15WPM and can be dynamically changed via a simple 5wpm / 10wpm / 20wpm etc. message sent from connected client. I chose to use the home WiFi network to support the connection in place of bluetooth since not all home PC have integrated bluetooth and bluetooth on ESP controllers is there only on ESP32 and I wanted to use a less expensive chip such as ESP12 or ESP3.

Regarding the way to connect the 3.5 - 5Vdc passive buzzer, the ESP12 can drive it directly without the need of a intermediate transistor buffer; so I did and it works nicely. I did not try a ESP3 to see if also its PIO pins can drive the passive buzzer as well.

The note tone is set by default at 800Hz but it can be changed to 400,500,600,700,900Hz if needed just sending a message like 400 or 500 or 900 etc. I noticed that the volume level varies noticebly depending on the sound frequency: 800Hz gives the maximum volume with the passive buzzer I am using.


### Morse standard used
The time of the dah is 3 times of the dit, the separation time between dit and/or dah is 1 dit time, the time separation betwenn letters/numbers/special chars is 1 dah (3xdit), the time separation between words is 7xdit. The dit time in ms is = 1200/wpm and ranges from 240ms / 5wpm to 40ms / 30wpm. These space timings can be set doubled upon request via command received from companion client (morse_trainer.py).


### How the morse keyer works
#### WIFI_STA mode
The communication between companion python PC application and ESP12_morse_keyer is aimed to WIFI_STA mode, that means a local WiFi router should be available whose ssid and password are configured in the ESP12 sketch. If a local WiFi router is not there, or the WiFi configuration in the Arduino sketch does not comply with the available WiFi router, then the sketch switches to AP mode (ESP12_AP is the ssid name) on IP 192.168.4.1 waiting the PC WiFi to connect to it. The AP IP address is shown on the oled display and as the PC is connected then the current battery voltage of the keyer device is displayed. This indicates that the companion python morse_trainer application can be started connecting the AP IP.

#### WIFI_AP mode
While the connection is active in this mode, then the local WiFi router can be set without recompiling the sketch but just sending the string: ^^yourssid yourpassword then these values are stored in the ESP12 flash memory and all eventual new boots will connect the router nicely.
The above string to configure the local WiFi router works any time the ESP12 is connected to the companion application: it works either in AP mode or STA mode.

If the local WiFi router is on and the configuration in the sketch complies with it, then the Host IP address assigned by the router appears on the display and is morse played at 15wpm speed to inform the operator to place the correct IP destination address in the companion python application to connect the morse_keyer device. The current device's battery voltage value is then appended to the oled display.

#### Connection port
The connection port is 8888 but obviously can be changed in the sketch as we like. After the IP code is morse played or the PC has connected the ESP12_AP in case no local WiFi is available, the sketch enters the loop section where the TCP server is listening for connections. As the connection is made, a welcome message is sent to the client and from that moment on the client can send command messages (i.e.: 20wpm to change the speed) or else any other message that will morse played on the buzzer driving in the meantime a relais or transistor whose output is wired to transmitter's cw key input. At the moment the relais / transistor drive is not programmed in the sketch but it is easy to be done when needed as soon as I get the transmitter built / purchased.

### ESP12 morse keyer schematic
The PDF file showing the schematic is provided here.
