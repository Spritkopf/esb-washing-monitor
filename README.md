## Washing machine monitor
Making the washing machine smart. A small board attached to the washing machine, monitoring the status of the "Finished" LED via a photoresistor. When the "Finished" LED is lit, a message is sent to the smart home central. There, a Telegram bot sends a message to all subscribers to the "washing machine monitor service". The communication protocol is "Enhanced Shockburst" a proprietary RF protocol from Nordic Semiconductor.

### Hardware
Base board is a module called "E73" from EBYTE, a very small board containing an NRF52832 SOC offering BLE and ESB. Connected to one of the ADCs is a photoresistor which is used to detect the status of the "Finished"-LED of the washing machine. 

### Software
The detection of the LED is based on thresholds for the ADC rawdata of the photoresistor. Use the test cases in wmon_test.go to do a calibration. The calibration data can be stored on the device in persistent memory
