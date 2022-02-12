# mixture-controller
RC engine mixture controller.

![](https://github.com/raleighcopter/mixture-controller/blob/main/photos/controller.jpg)

This controller works with a stihl solenoid part number 0000 120 5111 and a custom solenoid housing (https://github.com/raleighcopter/mixture-controller/blob/main/photos/solenoid%20housing.pdf) to modulate gasoline flow to a conventional methanol carburator. This allows us to generate a custom fuel mixture curve on our transmitter that is slaved to the throttle channel output. The result is that we can vary the fuel:air mixture across the entire throttle range to make the engine run near stoichiometric fuel:air ratio at idle, full throttle, and every point in between. Without the solenoid and controller, our methanol engines run extremely rich in the middle. 

The controller takes an RC channel and maps a 1000 uS - 2000 uS channel pulse to a PWM solenoid pulse. Additionally, the controller measures atmospheric pressure and temperature and adjusts the fuel amount to maintain the ideal mixture regardless of the weather or altitude (which constantly changes as we fly our models). We have engines that were tuned at 85 degrees F that run just as well at 35 degrees F without touching the needles. 

I currently have my OS FT120-ii and Saito FA72 converted and Bert (the man behind the idea who designed the solenoid housing) has engines ranging from an asp 0.30 four stroke up to an ASP 1.6 boxer and a larger radial converted and running well. This conversion has been used on 2 stroke engines, 4 stroke engines, single  and twin cylinder engines and radial multi-cylinder engines.

I built the controller using a seeduino Xiao an IRLD110pbf MOSFET, a 2.2k resistor, a diode, a bmp280, and optionally, a 128x32 oled display and I've tested it on 4s NiMH and 2s LiFePo. Power comes in on the power and ground pads on the back of the Xiao. 

If you're powering your controller with a 2s LiFePo battery, wire a 1n4003 diode or something similar in the red wire before feeding power to the power pad or the Vcc pin. if you're powering your controller with a 4S NiMh, omit the diode.

The solenoid, bmp280, and optional display grab 3.3 volts from the Xiao's 3.3v pin. The MOSFET is wired to D3 and the RC channel is wired to D2. There's only 1 ground pad on the back and one ground pin so I doubled up on the pad on the back because I need 3 grounds.

I wired a 2.2k resistor across the MOSFET's gate and source (an 0805 smt resistor works really well here but a 1/8 watt resistor also works) and added a short piece of wire to those pins before I slipped some heat shrink over the separate pins. I soldered the wire that runs to the solenoid to the MOSFET's drain, trimmed the pins and then covered the entire MOSFET with heat shrink. After it was covered with heat shrink, I soldered the wire from the MOSFET's gate to D3 and the wire from the MOSFET's source to ground.

SDA and SCL from the bmp280 go to D4 and D5 and the optional display also uses these connections. if you measure resistance of about 10k between SDO and ground on the bmp280 it has a base address of 0x76. otherwise it has a base address of 0x77 and you need to change the base address at the top of the controller code. 

If you're building this with an oled display, wire the display to the same 4 points that the bmp280 is connected: 3.3v, ground, SDA, and SCL. You can wire it in parallel or in series with the bmp280.

if you're using this with an opentx transmitter and want s.port data, connect your s.port to pin 0 of the Xiao.

ebay/china is the least expensive source for bmp280 sensors at about $1 each and 128x32 oled displays at about $2.50. Be aware that we've seen fairly high failure rates on the China sourced bmp280 boards. As a result, the solenoid will signal a failed sensor at startup by buzzing for half a second and then remaining quiet for another half second and repeating indefinitely..

you'll need to install a few libraries in the arduino IDE: adafruit bmp280, adafruit ssd1306, adafruit GFX library and the frsky s.port sensor library by herman kruisman. you'll also need to add the seeeduino xiao board definition using the board manager.


build photos as well as parts lists and assembly instructions are on the wiki at https://github.com/raleighcopter/mixture-controller/wiki

Contact me at d.a.taylor.pe@gmail.com
