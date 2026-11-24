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

    signal pressed(int x, int y)
    signal released()

    signal stop();

    Button {
        text: "Up Left"
        onPressed: ptc.pressed(-1, -1)
        onReleased: ptc.released()
    }
    Button {
        text: "Up"
        onPressed: ptc.pressed(0, -1)
        onReleased: ptc.released()
    }
    Button {
        text: "Up Right"
        onPressed: ptc.pressed(1, -1)
        onReleased: ptc.released()
    }

    Button {
        text: "Left"
        onPressed: ptc.pressed(-1, 0)
        onReleased: ptc.released()
    }
    Button {
        text: "Stop"
        onClicked: ptc.stop();
    }
    Button {
        text: "Right"
        onPressed: ptc.pressed(1, 0)
        onReleased: ptc.released()
    }

    Button {
        text: "Down Left"
        onPressed: ptc.pressed(-1, 1)
        onReleased: ptc.released()
    }
    Button {
        text: "Down"
        onPressed: ptc.pressed(0, -1)
        onReleased: ptc.released()
    }
    Button {
        text: "Down Right"
        onPressed: ptc.pressed(1, 1)
        onReleased: ptc.released()
    }
}