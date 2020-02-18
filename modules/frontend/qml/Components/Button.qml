import QtQuick 2.3

Rectangle {
    id: button

    property alias caption: caption.text

    property color color
    property color colorSub: Qt.darker(button.color, 2.0)
    property color colorBorder: Qt.lighter(button.color, 1.0)
    property color colorText: Qt.tint(button.color, "#55ffffff")

    signal clicked
    signal entered

    border.color: button.colorBorder
    radius: 4 * screenScale

    state: "nofocus"

    gradient: Gradient {
        GradientStop {
            id: gradientStop0
            position: 0
            color: button.color
        }
        GradientStop {
            id: gradientStop1
            position: 1
            color: button.colorSub
        }
    }

    Text {
        id: caption
        anchors.fill: parent
        clip: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font {
            bold: true
            family: "monospace"
            pointSize: (parent.width / 30) + 10
        }
        color: button.colorText
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        hoverEnabled: true
        propagateComposedEvents: true

        onEntered: {
            button.state = "focus";
            button.entered()
        }
        onExited:  button.state = "nofocus"

        onPressed:  button.state = "pressed"
        onReleased: button.state = "focus"

        onClicked: button.clicked()
    }

    transitions: [
        Transition {
            to: "nofocus"
            ParallelAnimation {
                ColorAnimation { target: gradientStop0; property: "color"; to: button.color; duration: 200 }
                ColorAnimation { target: gradientStop1; property: "color"; to: button.colorSub; duration: 200 }
                ColorAnimation { target: button; property: "border.color"; to: button.colorBorder; duration: 200 }
            }
        },
        Transition {
            to: "focus"
            ParallelAnimation {
                ColorAnimation { target: gradientStop0; property: "color"; to: Qt.lighter(button.color); duration: 200 }
                ColorAnimation { target: gradientStop1; property: "color"; to: Qt.lighter(button.colorSub); duration: 200 }
                ColorAnimation { target: button; property: "border.color"; to: Qt.lighter(button.colorBorder, 2.0); duration: 200 }
            }
        },
        Transition {
            to: "pressed"
            ParallelAnimation {
                ColorAnimation { target: gradientStop0; property: "color"; to: Qt.darker(button.color, 2.0); duration: 50 }
                ColorAnimation { target: gradientStop1; property: "color"; to: Qt.darker(button.colorSub, 2.0); duration: 50 }
            }
        }
    ]
}
