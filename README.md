# morse_keyer
 Morse keyer by ESP12e / TCP host
### Brief description
In view of beginning to operate in HF via QRP transmitter using CW, rather than buying a cw key or paddle I thought it was worthwhile to build a cw keyer based on 8266-12 microcontroller hosting a TCP server to receive data to be converted / transmitted from a TCP client such as Putty connected "raw telnet" running on PC. 

The keyer would receive input data from PC, it would convert those in morse code to drive a passive buzzer tone at 800Hz to make audible codes driving also a relais or transistor wired to the transmitter's cw key input.

In addition this keyer can be used to exercise own morse receiving ability thanks to the audible tones supplied by a passive buzzer directly connected to PIO D4 on ESP12e.

The transmit speed can be set at various speed such as 5,10,15,20,25,30 WPM. The default speed is set at 15WPM and can be dynamically changed via a simple 5wpm / 10wpm / 20wpm etc. message sent from connected client. I chose to use the home WiFi network to support the connection in place of bluetooth since not all home PC have integrated bluetooth and bluetooth on ESP controllers is there only on ESP32 and I wanted to use a less expensive chip such as ESP12 or ESP3.

Regarding the way to connect the 3.5 - 5Vdc passive buzzer, the ESP12 can drive it directly without the need of a intermediate transistor buffer; so I did and it works nicely. I did not try a ESP3 to see if also its PIO pins can drive the passive buzzer as well.

The note tone is set by default at 800Hz but it can be changed to 400,500,600,700,900Hz if needed just sending a message like 400 or 500 or 900 etc. I noticed that the volume level varies noticebly depending on the sound frequency: 800Hz gives the maximum volume with the passive buzzer I am using.


### Morse standard used
The time of the dah is 3 times of the dit, the separation time between dit and/or dah is 1 dit time, the time separation betwenn letters/numbers/special chars is 1 dah (3xdit), the time separation between words is 7xdit. The dit time in ms is = 1200/wpm and ranges from 240ms / 5wpm to 40ms / 30wpm.


### How the morse keyer works
After completing the boot, the Arduino sketch connects the local WiFi network by its SSID name and password then it detects the local ip assigned by the router and shows it in the serial monitor then plays its morse code on the buzzer at 15wpm speed. This is done to face the case where the ESP12 is not connected to the PC serial interface and since the user must know the TCP host IP address to be inserted in the Putty client (or whatever Telnet client is chosed), then playing its morse code by the buzzer I thought it was a good idea.

The connection port is 8888 but obviously can be changed in the sketch as we like. After the IP code is morse played, the sketch enters the loop section where the TCP server is listening for connections. As the connection is made, a welcome message is sent to the client and from that moment on the client can send command messages (i.e.: 20wpm to change the speed) or else any other message that will morse played on the buzzer driving in the meantime a relais or transistor whose output is wired to transmitter's cw key input. At the moment the relais / transistor drive is not programmed in the sketch but it is easy to be done when needed as soon as I get the transmitter built / purchased.
