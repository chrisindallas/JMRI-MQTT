# JMRI-MQTT
Files to drive model railroad signals using JMRI and the MQTT protocol.

#      -> Signals  -> MQTT publisher                                             MQTT subscriber -> Red/Yellow/Green
# JMRI -> Turnouts -> MQTT publisher  <= WiFi/Ethernet => MQTT Broker <= WiFi => MQTT subscriber -> H-bridge+Tortoise/Servo
#      <- Sensors  <- MQTT subscriber     or onboard                             MQTT publisher  <- Occupancy sensor
