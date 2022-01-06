# mixture-controller
RC engine mixture controller.

![](https://github.com/raleighcopter/mixture-controller/blob/main/photos/cover.jpg)

I built a controller using a seeduino Xiao an IRLD110pbf MOSFET, a 2.2k resistor, a diode, a bmp280, and optionally, a 128x32 oled display. This one has no boost converter and I've tested it on 4s NiMH and 2s LiFePo. Power comes in on the power and ground pads on the back of the Xiao wire a 1n4003 diode or something similar in the red wire before feeding power to the power pad or the Vcc pin. The solenoid, bmp280, and optional display grab 3.3 volts from the Xiao's 3.3v pin. The MOSFET is wired to D3 and the RC channel is wired to D2. There's only 1 ground pad on the back and one ground pin so I doubled up on the pad on the back because I need 3 grounds.

I wired a 2.2k resistor across the MOSFET's gate and source (an 0805 smt resistor works really well here but a 1/8 watt resistor also works) and added a short piece of wire to those pins before I slipped some heat shrink over the separate pins. I soldered the wire that runs to the solenoid to the MOSFET's drain, trimmed the pins and then covered the entire MOSFET with heat shrink. After it was covered with heat shrink, I soldered the wire from the MOSFET's gate to D3 and the wire from the MOSFET's source to ground.

SDA and SCL from the bmp280 go to D4 and D5 and the optional display also uses these connections. if you measure resistance of about 10k between SDO and ground on the bmp280 it has a base address of 0x76. otherwise it has a base address of 0x77 and you need to change the base address at the top of the controller code. 

If you're building this with an oled display, wire the display to the same 4 points that the bmp280 is connected: 3.3v, ground, SDA, and SCL. You can wire it in parallel or in series with the bmp280.

if you're using this with an opentx transmitter and want s.port data, connect your s.port to pin 0 of the Xiao.

ebay/china is the least expensive source for bmp280 sensors at about $1 each and 128x32 oled displays.

you'll need to install a few libraries in the arduino IDE: adafruit bmp280, adafruit ssd1306, adafruit GFX library and the frsky s.port sensor library by herman kruisman. you'll also need to add the seeeduino xiao board definition using the board manager.


most parts are sourced from Mouser electronics. you can source the bmp280 and 128x32 oled from ebay/china for much less than any other site so get them there. the following parts were ordered from mouser:

Description         part number       mouser link

From Mouser:
mosfet:             IRLD110PBF        https://www.mouser.com/ProductDetail/844-IRLD110PBF

2.2k resistor:      CR0805-JW-222ELF  https://www.mouser.com/ProductDetail/652-CR0805JW-222ELF  (2.2k 0805 smt)

diode               1N4003-T          https://www.mouser.com/ProductDetail/583-1N4003-T 

3/4" heat shrink    19267-0273        https://www.mouser.com/ProductDetail/538-19267-0273 (by the foot. 1 foot is enough for multiple controllers. They also make this in red if you're feeling like being different.)

1/16" heat shrink   HSTT06-48-QC      https://www.mouser.com/ProductDetail/644-HSTT06-48-QC (1 piece is 4 feet)

3/8" heat shrink    HSTTV38-48-Q      https://www.mouser.com/ProductDetail/Panduit/HSTTV38-48-Q?qs=E6Ue56mBjPwBm5eY0E1gGg%3D%3D (1 piece is 4 feet)

Seeeduino Xiao      102010328         https://www.mouser.com/ProductDetail/713-102010328


from ebay:
BMP280                                https://www.ebay.com/sch/i.html?_from=R40&_nkw=bmp280&_sacat=0&LH_TitleDesc=0&_sop=15

128x32 OLED                           https://www.ebay.com/sch/i.html?_from=R40&_nkw=128x32+oled+i2c&_sacat=0&LH_TitleDesc=0&_sop=15  (optional)

Misc Parts:
26 gage wire, stranded, various colors, minimum 4 colors but 8 different colors is easiest

servo cable (2 if you plan to build the controller with s.port telemetry)

build photos are on the wiki at https://github.com/raleighcopter/mixture-controller/wiki

Contact me at d.a.taylor.pe@gmail.com
