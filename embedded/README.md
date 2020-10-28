Collect temperature data using ESP32, display it in a web dashboard, and notify user is temperature is out of expected range.

* `server` contains the code used in the server to host the Node-RED and flask servers for email notifications when temperature is out of a range.
* `machine-learning` has the trained model to detect outliers during temperature monitoring.
* `client-esp32` contains the code to be run in the ESP-32 for temperature and humidity data collection.