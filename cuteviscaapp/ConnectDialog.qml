import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: nyaDialog
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: Math.min(parent.width/2, 400)
    height: Math.min(parent.height/1.5, 300)
    title: "Connect to camera"
    modal: true

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)

    property alias ip: ipText.text    

    ColumnLayout {
        anchors.fill: parent
        TextField {
            id: ipText
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhNoPredictiveText
            placeholderText: "Camera IP"
        }
    }    
}
