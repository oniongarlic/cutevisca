# CuteVISCA - Qt VISCA library

C++, Qt based library and example application for controlling PTZ Cameras supporting the VISCA protocol.

Very basic at the moment, VISCA over UDP only, RS-422 not supported. Supports most of basic operations but consider it work in progress and pretty much alpha at this time.

* Pan, Tilt, Zoom
* Manual focus, auto focus
* Exposure, Iris, White Balance
* Store and recall positions

Contrains an example test application with most basic functions implemented

* Can be controlled externaly with MQTT messages

## Build requirements:
* Qt 6.6.x
* QtMQTT 6.6.x
