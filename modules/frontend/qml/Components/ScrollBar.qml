import QtQuick 2.4

Item {
    id: scrollbar

    property real position
    property real pageSize
    property int orientation : Qt.Vertical

    Rectangle {
        id: background
        anchors.fill: parent
        radius: orientation == Qt.Vertical ? (width/2 - 1) : (height/2 - 1)
        color: "white"
        opacity: 0.3
        clip: true

        Rectangle {
            x: orientation == Qt.Vertical ? 1 : (scrollbar.position * (scrollbar.width-2) + 1)
            y: orientation == Qt.Vertical ? (scrollbar.position * (scrollbar.height-2) + 1) : 1
            width: orientation == Qt.Vertical ? (scrollbar.width-2) : (scrollbar.pageSize * (scrollbar.width-2))
            height: orientation == Qt.Vertical ? (scrollbar.pageSize * (scrollbar.height-2)) : (scrollbar.height-2)
            radius: orientation == Qt.Vertical ? (width/2 - 1) : (height/2 - 1)
            color: "black"
            opacity: 0.7
        }
    }
}
