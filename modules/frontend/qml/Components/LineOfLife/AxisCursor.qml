import QtQuick 2.4

Rectangle {
    id: root

    width: 1
    color: "#000000"

    function setTo(unixtime) {
        root.unixtime = unixtime
    }

    property alias text: cursor_text.text
    property var unixtime

    Rectangle {
        id: text_background
        anchors {
            left: parent.right
            bottom: parent.bottom
            margins: 1
        }

        radius: 2 * screenScale
        color: "#88ffffff"

        width: cursor_text.contentWidth
        height: cursor_text.contentHeight

        Text {
            id: cursor_text

            text: wdate.format(unixtime*1000, "dd/MM/yyyy hh:mm:ss")

            font.pixelSize: root.height / 6
            font.family: "monospaced"
        }

        states: State {
            name: "left"
            when: root.x + cursor_text.contentWidth > axis_mouse_area.width
            AnchorChanges {
                target: text_background
                anchors.left: undefined
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }
        }

        transitions: Transition {
            AnchorAnimation { duration: 200 }
        }
    }
}
