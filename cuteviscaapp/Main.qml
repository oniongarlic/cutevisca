import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.tal.visca
import org.tal.mqtt
import org.tal.bm

import QtGamepadLegacy

ApplicationWindow {
    width: 800
    height: 480
    visible: true
    title: qsTr("CuteVISCA")

    ViscaController {
        id: v

        onIsConnectedChanged: mq.publish('cutevisca/state', isConnected ? 'connected' : 'disconnected')
        onIsPoweredChanged: mq.publish('cutevisca/powered', isPowered ? 'on' : 'off')
    }

    VideoRouterClient {
        id: vrc
        Component.onCompleted: {
            connectToHost("192.168.0.60", 9990)
        }
        onRoutingChanged: {
            console.debug("Route", output, input)
            if (!v.isConnected)
                return

            if (output==13) {
                switch (input) {
                case 11:
                    v.recallMemoryPreset(0)
                    break;
                case 12:
                    v.recallMemoryPreset(1)
                    break;
                case 13:
                    v.recallMemoryPreset(2)
                    break;
                case 2:
                    v.recallMemoryPreset(3)
                    break;
                }
            }
        }
        onTablesInitialized: {
            console.debug(getInputsCount(), getOutputsCount())
        }
    }

    QtObject {
        property real pan: gamepad.axisLeftX
        property real tilt: gamepad.axisLeftY
        property bool zoomIn: gamepad.buttonA
        property bool zoomOut: gamepad.buttonB
        
        onPanChanged: panAndTilt();
        onTiltChanged: panAndTilt();

        onZoomInChanged: if (zoomIn) v.zoomIn(); else v.zoomStop();
        onZoomOutChanged: if (zoomOut) v.zoomOut(); else v.zoomStop();

        function doZoom() {
            console.debug(zoom)

            if (!v.isConnected)
                return

            if (zoom==0) {
                v.zoomStop();
            } else if (zoom<0) {
                v.zoomIn();
            } else if (zoom>0) {
                v.zoomOut();
            }
        }

        function panAndTilt() {
            console.debug(pan, tilt)

            let p=3;
            let t=3;

            if (pan>0.1) p=2; else if (pan<-0.1) p=1;
            if (tilt>0.1) t=2; else if (tilt<-0.1) t=1;

            let ps=1+Math.abs(pan*17);
            let ts=1+Math.abs(tilt*17);

            console.debug(ps, ts, p, t)
            if (v.isConnected)
                v.panTilt(ps ,ts, p, t)
        }
    }

    Connections {
        target: GamepadManager
        function onGamepadConnected(deviceId) { gamepad.deviceId = deviceId }
    }

    Gamepad {
        id: gamepad
        deviceId: GamepadManager.connectedGamepads.length > 0 ? GamepadManager.connectedGamepads[0] : -1
    }

    Component.onCompleted: {
        //mq.setHostname('localhost')
        mq.setPort(1883)

        mq.connectToHost();
    }

    MqttClient {
        id: mq
        clientId: "cutevisca"
        hostname: "localhost"
        //port: 1883

        onConnected: {
            console.debug("MQTT Connected")
            subscribe('video/0/facedetect/face')
            publish('cutevisca/state', 'idle')
        }

        onErrorChanged: console.debug("MQTT Error", error)

        onStateChanged: console.debug("MQTT State", state)

        onMessageReceived: {
            console.debug("MQTT", topicString(topic), message)
            let s=topicString(topic);
            console.debug("topic", s)
            if (s=='video/0/facedetect/face') {
                let d=JSON.parse(message)
                if (d["face"]) {
                    let fh=d["face"][0]
                    let fw=d["face"][1]
                    let fa=d["face"][2]
                    console.debug("Face at", fh, fw, fa)
                    faceTracker.track(fh, fw);
                    faceTracker.trackSize(fa);
                    faceSize.text=fa;
                    facePos.text=fh+" : "+fw;
                } else {
                    console.debug("No face detected")
                    faceTracker.noFace();
                }
            }
        }
    }

    QtObject {
        id: faceTracker
        property bool faceTracking: true
        property bool faceSizeTracking: true

        property bool centering: false
        property bool vCentering: false
        property bool hCentering: false
        property bool zooming: false

        property double faceSizeMin: 0.09
        property double faceSizeMax: 0.20
        property double faceSizeMargin: 0.05

        property int vSpeed: 0
        property int hSpeed: 0

        property int vDirection: 3
        property int hDirection: 3

        function noFace() {
            stopTracking();
            stopZooming();
        }

        function stopTracking() {
            if (vCentering || hCentering)
                v.stopMove();
            vCentering=false;
            hCentering=false;
            hDirection=3;
            vDirection=3;
        }

        function stopZooming() {
            if (zooming)
                v.zoomStop();
            zooming=false;
        }

        function trackSize(s) {
            if (zooming && s>(faceSizeMin+faceSizeMargin) && s<(faceSizeMax-faceSizeMargin)) {
                zooming=false;
                v.zoomStop();
                console.debug("ZOOM STOP")
            } else if (s<faceSizeMin && !zooming) {
                zooming=true;
                v.zoomIn();
                console.debug("ZOOM IN")
            } else if (s>faceSizeMax && !zooming) {
                zooming=true;
                v.zoomOut();
                console.debug("ZOOM OUT")
            }
            console.debug("FaceSize: ", s, zooming)
        }

        function trackVertical(fw) {
            if (vCentering && fw<0.10 && fw>-0.10) {
                vCentering=false;
                vSpeed=0;
                vDirection=3;
                return false;
            }

            vSpeed=Math.abs(fw*16)+3;

            if (fw>0.18 && !vCentering) {
                vCentering=true
                vDirection=2;
                return true;
            } else if (fw<-0.18 && !vCentering) {
                vCentering=true;
                vDirection=1;
                return true;
            }
            return false;
        }

        function trackHorizontal(fh, fw) {
            if (hCentering && fh<0.10 && fh>-0.10) {
                hCentering=false;
                hDirection=3
                return false;
            }

            hSpeed=Math.abs(fh*14)+4;

            if (fh>0.18 && !hCentering) {
                hCentering=true
                hDirection=2;
            } else if (fh<-0.18 && !hCentering) {
                hCentering=true;
                hDirection=1
            }
            return false;
        }

        function track(fh, fw) {
            if (!faceTracking)
                return;

            trackHorizontal(fh)
            trackVertical(fw)

            console.debug("FaceTrack", hSpeed, vSpeed, hDirection, vDirection)

            v.panTilt(hSpeed, vSpeed, hDirection, vDirection)

        }

    }

    header: ToolBar {
        RowLayout {
            ToolButton {
                text: "Connect"
                onClicked: v.connectCamera()
            }
            ToolButton {
                text: "Emulator"
                onClicked: {
                    v.setCamera("127.0.0.1", 52388)
                    v.connectCamera()
                }
            }
            ToolButton {
                text: "Power on"
                enabled: !v.isPowered && v.isConnected
                onClicked: v.powerOn()
            }
            ToolButton {
                text: "Power off"
                enabled: v.isPowered && v.isConnected
                onClicked: v.powerOff()
            }
            Slider {

            }
        }
    }

    ColumnLayout {
        id: c
        enabled: v.isConnected
        spacing: 4
        anchors.fill: parent
        anchors.margins: 4
        GridLayout {
            rows: 3
            columns: 3
            columnSpacing: 4
            rowSpacing: 4
            Button {
                text: "Up Left"
                onClicked: v.panLeftUp()
            }
            Button {
                text: "Up"
                onClicked: v.tiltUp()
            }
            Button {
                text: "Up Right"
                onClicked: v.panRightUp()
            }

            Button {
                text: "Left"
                onClicked: v.panLeft()
            }
            Button {
                text: "Stop"
                onClicked: v.stopMove();
            }
            Button {
                text: "Right"
                onClicked: v.panRight()
            }

            Button {
                text: "Down Left"
                onClicked: v.panLeftDown()
            }
            Button {
                text: "Down"
                onClicked: v.tiltDown()
            }
            Button {
                text: "Down Right"
                onClicked: v.panRightDown()
            }
        }

        RowLayout {
            spacing: 4
            Button {
                text: "Zoom In"
                onClicked: v.zoomIn()
            }
            Button {
                text: "In"
                onPressed: {
                    v.zoomIn()
                }
                onReleased: {
                    v.zoomStop()
                }
            }
            Button {
                text: "Zoom Out"
                onClicked: v.zoomOut()
            }
            Button {
                text: "Out"
                onPressed: {
                    v.zoomOut()
                }
                onReleased: {
                    v.zoomStop()
                }
            }
            Button {
                text: "Zoom Stop"
                onClicked: v.zoomStop()
            }
            SpinBox {
                from: 0
                to: 16384
                stepSize: 64
                editable: true
                onValueChanged: {
                    console.debug("Zoom to", value)
                    v.zoomSet(value)
                }
            }
            Button {
                text: "Get zoom"
                onClicked: v.inquireZoom();
            }
        }
        RowLayout {
            spacing: 4
            Button {
                text: "Zoom In"
                onClicked: v.zoomIn(zoomSpeed.value)
            }
            Button {
                text: "Zoom Out"
                onClicked: v.zoomOut(zoomSpeed.value)
            }
            SpinBox {
                id: zoomSpeed
                from: 0
                to: 7
                value: 7
                stepSize: 1
                editable: true
            }
            Label {
                text: v.zoomPosition
            }
        }

        RowLayout {
            spacing: 4
            Button {
                text: "Focus auto"
                onClicked: v.focusAuto()
            }
            Button {
                text: "Focus manual"
                onClicked: v.focusManual()
            }
            Button {
                text: "Infinity"
                onClicked: v.focusInfinity()
            }

            Button {
                text: "AF Trigger"
                onClicked: v.focusTrigger()
            }
        }

        RowLayout {
            spacing: 4
            Button {
                text: "Exp auto"
                onClicked: v.expModeAuto()
            }
            Button {
                text: "Exp Iris"
                onClicked: v.expModeIrisPriority()
            }
            Button {
                text: "Exp shutter"
                onClicked: v.expModeShutterPriority()
            }
            Button {
                text: "Exp Gain"
                onClicked: v.expModeGainPriority()
            }
            Button {
                text: "Exp manual"
                onClicked: v.expModeManual();
            }
            Button {
                text: "Get"
                onClicked: v.inquireExpMode();
            }
        }

        RowLayout {
            spacing: 4
            Button {
                text: "Iris reset"
                onClicked: v.irisReset()
            }
            Button {
                text: "UP"
                onClicked: v.irisUp()
            }
            Button {
                text: "Down"
                onClicked: v.irisDown()
            }
        }

        RowLayout {
            spacing: 4
            Layout.alignment: Qt.AlignTop

            ColumnLayout {
                Label {
                    text: "Whitebalance"
                }
                Button {
                    text: "Auto 1"
                    onClicked: v.wbAuto1()
                }
                Button {
                    text: "Auto 2"
                    onClicked: v.wbAuto2()
                }
                Button {
                    text: "Indoor"
                    onClicked: v.wbIndoor()
                }
                Button {
                    text: "Outdoor"
                    onClicked: v.wbOutdoor()
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: 4
                Label {
                    text: "Memory Recall"
                }
                Repeater {
                    model: 5
                    delegate: Button {
                        required property int index
                        text: index+1
                        onClicked: v.recallMemoryPreset(index+1)
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: 4
                Label {
                    text: "Memory Save"
                }
                Repeater {
                    model: 5
                    delegate: DelayButton {
                        required property int index
                        text: index+1
                        onClicked: v.setMemoryPreset(index+1)
                    }
                }
            }

            ColumnLayout {
                spacing: 4
                Button {
                    text: "Tally-On"
                    onClicked: v.tallyOn()
                }
                Button {
                    text: "Tally-Off"
                    onClicked: v.tallyOff()
                }

            }

        }

        Rectangle {
            color: "green"
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

    }

    footer: ToolBar {
        RowLayout {
            Label {
                id: facePos
            }
            Label {
                id: faceSize
            }
        }
    }
}
