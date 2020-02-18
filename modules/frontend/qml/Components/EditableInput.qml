import QtQuick 2.4

Rectangle {
    id: root

    property alias font: text_main.font
    property alias style: text_main.style
    property alias styleColor: text_main.styleColor
    property alias default_text: text_default.text
    property alias text: text_main.text
    property bool editable: false
    property int maxWidth: 0
    property alias horizontalAlignment: text_main.horizontalAlignment
    property alias inputMask: text_edit.inputMask

    property Item next_item

    signal done(string text)

    function setEdit(is) {
        root.enabled = true
        root.editable = is
    }

    color: "#00000000"

    width: maxWidth > 0 ? maxWidth : ((text_edit.length === 0) ? text_default.contentWidth + 4 : text_edit.contentWidth + 4)
    height: text_content.childrenRect.height + 4

    radius: 2

    onEditableChanged: {
        // Save current value
        if( ! editable )
            root.text = text_edit.text
    }

    onFocusChanged: {
        if( focus )
            text_edit.focus = true
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        cursorShape: Qt.IBeamCursor

        onEntered: {
            root.border.width = 1
        }
        onExited: {
            root.border.width = 0
        }

        Rectangle {
            id: text_content
            anchors {
                left: parent.left
                top: parent.top
                margins: 2
            }

            Text {
                id: text_default

                color: '#aaa'
                font: text_main.font
                style: text_main.style
                styleColor: text_main.styleColor

                horizontalAlignment: text_main.horizontalAlignment
                wrapMode: text_main.wrapMode
                width: root.maxWidth > 0 ? root.maxWidth : contentWidth

                visible: text_edit.length === 0
            }
            Text {
                id: text_main
                wrapMode: Text.WordWrap
                width: root.maxWidth > 0 ? root.maxWidth : contentWidth
            }

            TextInput {
                id: text_edit
                selectByMouse: true
                visible: false
                enabled: false
                focus: root.focus

                horizontalAlignment: text_main.horizontalAlignment
                wrapMode: text_main.wrapMode
                width: root.maxWidth > 0 ? root.maxWidth : Math.max(contentWidth, text_default.contentWidth)
                height: Math.max(contentHeight, text_default.height)

                font: text_main.font
                text: text_main.text

                onAccepted: {
                    text_main.text = text_edit.text
                    done(text_edit.text)
                    next_item.focus = true
                }
                KeyNavigation.tab: next_item
            }
        }
    }

    states: [
        State {
            name: "edit"
            when: root.editable
            PropertyChanges { target: text_edit; visible: true; enabled: true }
            PropertyChanges { target: text_main; visible: false }
            PropertyChanges { target: root; color: '#ccffffff' }
        }
    ]
    transitions: [
        Transition {
            SequentialAnimation {
                ParallelAnimation {
                    ColorAnimation { duration: 500; easing.type: Easing.OutCubic }
                }
            }
        }
    ]
}

