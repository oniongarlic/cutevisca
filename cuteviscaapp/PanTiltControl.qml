import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.tal.visca

GridLayout {
    id: ptc
    rows: 3
    columns: 3
    columnSpacing: 4
    rowSpacing: 4

    Layout.fillHeight: true
    Layout.fillWidth: true

    Layout.minimumHeight: 128
    Layout.minimumWidth: 256

    signal pressed(int x, int y)
    signal released()

    signal stop();

    Button {
        text: "Up Left"
        Layout.fillHeight: true
        onPressed: ptc.pressed(-1, -1)
        onReleased: ptc.released()
    }
    Button {
        text: "Up"
        Layout.fillHeight: true
        onPressed: ptc.pressed(0, -1)
        onReleased: ptc.released()
    }
    Button {
        text: "Up Right"
        Layout.fillHeight: true
        onPressed: ptc.pressed(1, -1)
        onReleased: ptc.released()
    }

    Button {
        text: "Left"
        Layout.fillHeight: true
        onPressed: ptc.pressed(-1, 0)
        onReleased: ptc.released()
    }
    Button {
        text: "Stop"
        Layout.fillHeight: true
        onClicked: ptc.stop();
    }
    Button {
        text: "Right"
        Layout.fillHeight: true
        onPressed: ptc.pressed(1, 0)
        onReleased: ptc.released()
    }

    Button {
        text: "Down Left"
        Layout.fillHeight: true
        onPressed: ptc.pressed(-1, 1)
        onReleased: ptc.released()
    }
    Button {
        text: "Down"
        Layout.fillHeight: true
        onPressed: ptc.pressed(0, 1)
        onReleased: ptc.released()
    }
    Button {
        text: "Down Right"
        Layout.fillHeight: true
        onPressed: ptc.pressed(1, 1)
        onReleased: ptc.released()
    }
}