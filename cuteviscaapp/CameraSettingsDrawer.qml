import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.tal.visca

Drawer {
    id: cameraDrawer
    interactive: visible
    width: parent.width/1.6
    height: root.height

    property ViscaController v;

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        Label {
            text: "Camera settings"
        }

        RowLayout {
            Layout.alignment: Qt.AlignTop
            spacing: 4

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                spacing: 4
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
                spacing: 4
                Label {
                    text: "Exposure"
                }
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
        }

    }
}
