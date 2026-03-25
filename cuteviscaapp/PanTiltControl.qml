import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.tal.visca

GridLayout {
    rows: 3
    columns: 3
    columnSpacing: 4
    rowSpacing: 4

    required property ViscaController visca;

    Button {
        text: "Up Left"
        onClicked: visca.panLeftUp()
    }
    Button {
        text: "Up"
        onClicked: visca.tiltUp()
    }
    Button {
        text: "Up Right"
        onClicked: visca.panRightUp()
    }

    Button {
        text: "Left"
        onClicked: visca.panLeft()
    }
    Button {
        text: "Stop"
        onClicked: visca.stopMove();
    }
    Button {
        text: "Right"
        onClicked: visca.panRight()
    }

    Button {
        text: "Down Left"
        onClicked: visca.panLeftDown()
    }
    Button {
        text: "Down"
        onClicked: visca.tiltDown()
    }
    Button {
        text: "Down Right"
        onClicked: visca.panRightDown()
    }
}