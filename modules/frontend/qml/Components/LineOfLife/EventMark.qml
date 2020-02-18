import QtQuick 2.4
import QtGraphicalEffects 1.0
import co.asocial 1.0

import "../../js/lineoflife.js" as L

Rectangle {
    id: root

    anchors.verticalCenter: parent.verticalCenter
    width: 1
    height: axis.height * 0.5
    color: "#00f"

    property string format: "dd/MM/yyyy"
    property var unixtime
    property color textColor: app.getTextColor(color)

    function updatePos() {
        root.x = L.timeToPoint(unixtime)
    }

    Rectangle {
        id: text_background

        anchors {
            horizontalCenter: parent.left
            top: parent.bottom
        }

        radius: 2 * screenScale
        color: Qt.rgba(root.color.r, root.color.g, root.color.b, 0.8)

        width: mark_text.contentWidth
        height: mark_text.contentHeight

        Text {
            id: mark_text

            text: wdate.format(unixtime*1000, root.format)

            font.pixelSize: 12
            font.family: "monospaced"
            color: textColor
        }
    }

    RectangularGlow {
        id: effect
        anchors.fill: root
        visible: root.focus

        glowRadius: 10
        spread: 0.1
        color: "#fff"
        cornerRadius: root.radius + glowRadius
    }
}
