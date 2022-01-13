//this code is written for the seeeduino xiao by david taylor and released under the 
//GNU general public license V3.0. Permissions of this strong copyleft license are 
//conditioned on making available complete source code of licensed works and modifications,
//which include larger works using a licensed work, under the same license. Copyright and 
//license notices must be preserved. Contributors provide an express grant of patent rights.

//pin d3 controls the solenoid
//pin d2 is the rc channel input
//adjust reference temp (kelvin), pressure (paschal), and rc channel pin here
const int reference_temp = 283;
const int reference_pressure = 100000;
const int rc_functionPin = 2;

//OLED addresses are 0x3C or 0x3D
#define OLED_ADDR   0x3C

//BMP280 addresses are 0x76 or 0x77
#define BMP280_ADDR   0x76

//****************************************************************************************
// **********************  DO NOT EDIT ANYTHING BELOW THIS LINE  *************************
//****************************************************************************************

unsigned long old_time;
int update_display;
const long display_interval = 1000; 
volatile unsigned long pulse_start_time = 0L;
volatile unsigned long pulse_now = 0L;
volatile unsigned long channel_pulse = 0L;
volatile unsigned long pulse_old = 0L;
volatile unsigned long adjusted_pulse = 0L;
volatile unsigned long solenoid_pulse = 0L;
volatile unsigned long usable_pulse = 0L;
int channel;
char _buffer[9];

#include <SPort.h> 
#include <Wire.h>
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver
#include <Adafruit_BMP280.h>   // include Adafruit BMP280 sensor library

Adafruit_BMP280 bmp; // I2C
Adafruit_SSD1306 display(-1);  // initialize Adafruit display library, share reset with xiao reset

SPortHub hub(0x12, 0);                    //Hardware ID 0x12, Software serial pin 0
CustomSPortSensor intake_temperature(getSensorData);  //Sensor with a callback function to get the data
CustomSPortSensor intake_pressure(getSensorData1);  //Sensor with a callback function to get the data
CustomSPortSensor compensation_ratio(getSensorData2);  //Sensor with a callback function to get the data
CustomSPortSensor solenoid_command(getSensorData3);  //Sensor with a callback function to get the data

void setup() {

  hub.registerSensor(intake_temperature);          //Add sensor to the hub
  hub.registerSensor(intake_pressure);             //Add sensor to the hub
  hub.registerSensor(compensation_ratio);          //Add sensor to the hub
  hub.registerSensor(solenoid_command);            //Add sensor to the hub


  hub.begin();                            //Start listening for s.port data

// initialize the SSD1306 OLED display with I2C address = 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
// clear the display buffer.
  display.clearDisplay();
  display.setTextSize(1);   // text size = 1
  display.setTextColor(WHITE, BLACK);  // set text color to white and black background
  
//uncomment the next line to debug  
//SerialUSB.begin(115200);

  pinMode(rc_functionPin, INPUT);
  attachInterrupt(rc_functionPin, RCpinread, CHANGE);

  if (!bmp.begin(BMP280_ADDR, BMP280_CHIPID)) {
  SerialUSB.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    while (1) delay(10);
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

//setup timer regisiters
  pinMode(3, OUTPUT);
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1000) |       // Divide the main clock down by some factor to get generic clock
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

// Enable the port multiplexer for the digital pin. 
  PORT->Group[g_APinDescription[3].ulPort].PINCFG[g_APinDescription[3].ulPin].bit.PMUXEN = 1;
  
//Connect the TCC0 timer to digital output - port pins are paired odd PMUO and even PMUXE 
  PORT->Group[g_APinDescription[2].ulPort].PMUX[g_APinDescription[2].ulPin >> 1].reg = PORT_PMUX_PMUXO_E; // | PORT_PMUX_PMUXE_F;

// Feed GCLK4 to TCC0 and TCC1
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;   // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

//Set for Single slope PWM operation: timers or counters count up to TOP value and then repeat
  REG_TCC1_WAVE |= TCC_WAVE_WAVEGEN_NPWM;        // Reverse the output polarity on all TCC0 outputs
  while (TCC1->SYNCBUSY.bit.WAVE);               // Wait for synchronization
 
  REG_TCC1_PER = 6144;                           // This sets the rate or frequency of PWM signal 
  while (TCC1->SYNCBUSY.bit.PER);                // Wait for synchronization
  
// Set the PWM signal to output 50% duty cycle initially 
  REG_TCC1_CC1 = 3072;                           // this sets the pwm duty cycle. values from 0 6145 are valid. 
  while (TCC1->SYNCBUSY.bit.CC1);                // Wait for synchronization

// Set prescaler and enable the outputs
  REG_TCC1_CTRLA |= TCC_CTRLA_PRESCALER_DIV1 |    // Divide GCLK4 by 1
                    TCC_CTRLA_ENABLE;             // Enable the TCC0 output
  while (TCC1->SYNCBUSY.bit.ENABLE);              // Wait for synchronization


}

//############################################################################

void loop() {
  
  hub.handle();                           //Handle new s.port data
  
//adjust for temp and pressure  
    adjusted_pulse = (usable_pulse * (reference_temp/(bmp.readTemperature()+273)*(bmp.readPressure()/reference_pressure))) * 6.145; 
    solenoid_pulse = constrain(adjusted_pulse, 0, 6145);
    solenoid_pulse = map(solenoid_pulse, 0, 6145, 6145, 0);

// load pulse value into timer register
    REG_TCC1_CC1 = (solenoid_pulse); 
    while (TCC1->SYNCBUSY.bit.CC1);

/*/  uncomment this section to debug  
    SerialUSB.print(bmp.readTemperature());
    SerialUSB.print(" *C     ");
    SerialUSB.print(bmp.readPressure());
    SerialUSB.print(" Pa     ");
    SerialUSB.print(adjusted_pulse);
    SerialUSB.print("  ");     
    SerialUSB.print(solenoid_pulse);
    SerialUSB.print("  ");
    SerialUSB.println(channel_pulse);
    SerialUSB.println(usable_pulse);
*/    
    


// do this once every display_interval.
  unsigned long current_time = millis();
  if(current_time - old_time >= display_interval){
    update_display = 1;                       //update the display

  if(update_display = 1){
    
// get temperature and pressure from library
  float temp     = bmp.readTemperature();   // get temperature
  float pressure = bmp.readPressure();      // get pressure
  float corr     = 100; 
// print data on the LCD

//clear the display
  display.clearDisplay();
  
// print temperature
  if(temp < 0)
    sprintf(_buffer, "-%02u.%01u C", (int)abs(temp), (int)(abs(temp) * 100) % 10 );
  else
    sprintf(_buffer, "%02u.%01u C", (int)temp, (int)(temp * 100) % 10 );
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print(_buffer);
 
// print degree symbol ( ° )
 if(temp < 0)
   display.drawCircle(32, 1, 1, WHITE);
 else
   display.drawCircle(26, 1, 1, WHITE);
   
// print pressure
  if(pressure < 100000)
    sprintf(_buffer, "%03u.%01u mB", (int)(pressure/100), (int)((uint32_t)pressure % 10));
  else
    sprintf(_buffer, "%04u.%01umB", (int)(pressure/100), (int)((uint32_t)pressure % 10));
  display.setCursor(0,8);
  display.setTextSize(1);
  display.print(_buffer);


//print correction
  
  corr =  1000 * (reference_temp/(temp+273)*(pressure/reference_pressure)); 
  if(corr<1000){
    sprintf(_buffer, "%02u.%01u", (int)(corr/10), (int)((uint32_t)corr % 10));
    display.setCursor(70,0);
    }
  else {
    sprintf(_buffer, "%03u.%01u", (int)(corr/10), (int)((uint32_t)corr % 10));
    display.setCursor(60,0);
    }
  display.setTextSize(2);
  display.print(_buffer);

// print a % symbol
  display.drawCircle(120, 1, 1, WHITE);
  display.drawCircle(123, 8, 1, WHITE);
  display.drawLine(123,1,120,8,WHITE);

//print channel pulse

  sprintf(_buffer, "%04u Channel Pulse", (int)(channel));
  display.setCursor(0,16);
  display.setTextSize(1);
  display.print(_buffer);  


// print solenoid pulse
  sprintf(_buffer, "%04u Solenoid Pulse", (int)(solenoid_pulse));
  display.setCursor(0,24);
  display.setTextSize(1);
  display.print(_buffer);  

// update the display
  display.display();

//reset the timer flag and set a new one
  old_time = millis();
  update_display=0;
}
}
}


// this function reads the rc channel pulse time
void RCpinread() {
  pulse_now = micros();
  pulse_old = channel_pulse;
  channel_pulse = pulse_now - pulse_start_time;
  pulse_start_time = pulse_now;
  if(channel_pulse < 988) {
    channel_pulse = pulse_old; //throw out low values
  }else if(channel_pulse > 2012) {
    channel_pulse = pulse_old; //throw out high values
  }
  channel_pulse = constrain (channel_pulse, 1000, 2000);
  channel = channel_pulse;
  usable_pulse = channel_pulse - 1000;
}

sportData getSensorData(CustomSPortSensor* intake_temperature) {
  sportData data; 
  data.applicationId = 0x5900;            //Set the sensor id for the current data poll. Set to 0 to discard the data, skip to the next sensor
  data.value = int(bmp.readTemperature()*10);                      //Set the sensor value 
  return data;
}

sportData getSensorData1(CustomSPortSensor* intake_pressure) {
  sportData data; 
  data.applicationId = 0x5901;            //Set the sensor id for the current data poll. Set to 0 to discard the data, skip to the next sensor
  data.value = int(bmp.readPressure()/10);                      //Set the sensor value 
  return data;
}

sportData getSensorData2(CustomSPortSensor* compensation_ratio) {
  sportData data; 
  data.applicationId = 0x5902;            //Set the sensor id for the current data poll. Set to 0 to discard the data, skip to the next sensor
  data.value = int(1000 * (reference_temp/(bmp.readTemperature()+273)*(bmp.readPressure()/reference_pressure)));                      //Set the sensor value 
  return data;
}

sportData getSensorData3(CustomSPortSensor* solenoid_command) {
  sportData data; 
  data.applicationId = 0x5903;            //Set the sensor id for the current data poll. Set to 0 to discard the data, skip to the next sensor
  data.value =  solenoid_pulse;                      //Set the sensor value 
  return data;
}
