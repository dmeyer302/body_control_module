# body_control_module

Software to perform various functions for a car, specifically a custom hot rod build. Code optimized for Arduino / Teensy 3.5.


### Functions include (no particular order):

* VSS sensing / Speedometer output
* SecuriCode door-mounted lock/unlock code pad
* Intermittent wipers
* Fading/Dimming instrument cluster
* Fading dome lights
* Turn signals including short press for 3 blink lane change

#### OLED display out:

* PRND21 indication
* Odometer / Trip storage stored in EEPROM
* Engine hours stored in EEPROM
* Temp display
* RTC-based clock display
* Door ajar indication (OLED and idiot light)
* Low fuel indication (OLED and idiot light)

### Hardware used
This list is growing...

  #### Electronics
  * Teensy 3.5
  * 0.96" 128x32 OLED
  * MCP23017 I2C IO Expander
  * 2803 Transistor Arrays
  * 817C Optoisolators
  #### Car stuff
  * 98-01 Ford Ranger dash
  * 1990 Ford AOD VSS and gear sensor
  * 13-16 Ford Escape exterior temp sensor
  * 12-17 Explorer headlight switch, if I can get CAN-bus figured out
  * SecuriCode keypad F8DB-14A636-ABW (later versions probably not recommended due to using CAN-bus)


### TODO:

* Autolamp using sunload sensor
* Tach output
* Various gauges output
* OTIS

### And more...
