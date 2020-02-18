import QtQuick 2.4
import co.asocial 1.0

Rectangle {
    id: root

    anchors.verticalCenter: parent.verticalCenter
    width: 1
    height: axis.height * 0.5
    color: "#000000"

    property string format: "dd/MM/yyyy"
    property var unixtime
    property int level: 0

    function init(prevmark) {
        mark_text.visible = true
        root.opacity = 1.0
        if( prevmark !== undefined && level <= prevmark.level ) {
            var delta = Math.abs(root.x - prevmark.x)
            if( delta < mark_text.contentWidth ) {
                if( level === WDate.YEAR )
                    mark_text.visible = parseInt(mark_text.text) % 10 === 0
                if( level === WDate.MONTH )
                    mark_text.visible = parseInt(mark_text.text) % 3 === 1
                if( level === WDate.DAY )
                    mark_text.visible = parseInt(mark_text.text) % 5 === 0
                if( level === WDate.HOUR )
                    mark_text.visible = parseInt(mark_text.text) % 3 === 0

                if( ! mark_text.visible )
                    root.opacity = 0.3
            }
        }
    }

    Text {
        id: mark_text

        anchors {
            left: parent.right
            bottom: parent.bottom
            leftMargin: 1
        }

        text: wdate.format(unixtime*1000, root.format)

        font.pixelSize: 12
        font.family: "monospaced"
    }
}
