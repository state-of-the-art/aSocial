import QtQuick 2.4
import "../js/markdown.lib.js" as MD

Rectangle {
    id: root

    property alias font: text_main.font
    property string text: ""
    property bool editable: false
    property alias horizontalAlignment: text_main.horizontalAlignment

    property Item next_item

    signal done(string text)

    function setEdit(is) {
        root.enabled = true
        root.editable = is
        text_edit.focus = true
    }

    onEditableChanged: {
        // Save current value
        if( ! editable )
            root.text = text_edit.text
    }

    onFocusChanged: {
        if( focus )
            text_edit.focus = true
    }

    radius: 2 * screenScale
    color: "#00000000"
    clip: true

    MouseArea {
        id: mouse_area
        anchors.fill: parent
        hoverEnabled: true

        cursorShape: (root.editable || text_main.selectByMouse) ? Qt.IBeamCursor : Qt.ArrowCursor

        onWheel: {
            console.log("Wheel TextEdit")
        }
        onEntered: {
            root.border.width = 1
            tool_panel.show()
        }
        onExited: {
            root.border.width = 0
            tool_panel.show()
        }

        Flickable {
            id: text_content
            pressDelay: 300
            anchors {
                fill: parent
                margins: 2 * screenScale
            }
            contentWidth: text_main.visible ? text_main.width : text_edit.width
            contentHeight: text_main.visible ? text_main.contentHeight : text_edit.contentHeight

            TextEdit {
                id: text_main
                //anchors.fill: parent
                width: text_content.width
                clip: true
                //selectByMouse: true

                readOnly: true
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignJustify

                wrapMode: Text.WordWrap
                text: MD.makeHtml(text_edit.text)
                onLinkActivated: Qt.openUrlExternally(link)
                onLinkHovered: mouse_area.cursorShape = link ? Qt.PointingHandCursor : (selectByMouse ? Qt.IBeamCursor : Qt.ArrowCursor)
            }

            TextEdit {
                id: text_edit
                anchors.fill: parent

                selectByMouse: true
                visible: false
                enabled: false
                focus: root.focus
                text: root.text

                horizontalAlignment: text_main.horizontalAlignment
                //wrapMode: text_main.wrapMode
                //font: text_main.font
                font.family: "monospace"
            }

            states: State {
                name: "ShowBars"
                when: text_content.flicking
                PropertyChanges { target: vertical_scrollbar; opacity: 1 }
            }
            transitions: Transition {
                NumberAnimation { property: "opacity"; from: 0.2; duration: 400 }
                NumberAnimation { property: "opacity"; from: 1.0; duration: 2000; easing.type: Easing.OutCubic }
            }
        }

        ScrollBar {
            id: vertical_scrollbar
            width: 12 * screenScale; height: text_content.height
            anchors.right: text_content.right
            opacity: 0.2
            orientation: Qt.Vertical
            position: text_content.contentY / text_content.contentHeight
            pageSize: text_content.height / text_content.contentHeight
        }

        Rectangle {
            id: tool_panel
            anchors {
                top: parent.top
                right: parent.right
            }

            function show() {
                tool_panel.opacity = 1.0
                tool_panel.enabled = true
                tool_hide_timer.start()
            }

            function hide() {
                tool_panel.opacity = 0.0
                tool_panel.enabled = false
            }

            width: 30 * screenScale
            height: tool_column_view.height + 10 * screenScale

            color: "#55000000"

            Behavior on opacity {
                NumberAnimation { duration: 500 }
            }

            Column {
                id: tool_column_view
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: 5 * screenScale
                }

                Button {
                    id: button_edit
                    anchors.horizontalCenter: parent.horizontalCenter

                    width: 20 * screenScale
                    height: 20 * screenScale
                    color: "#595"

                    caption: "E"

                    onClicked: {
                        root.setEdit(true)
                    }
                }
            }

            Column {
                id: tool_column_edit
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: 5 * screenScale
                }

                visible: false

                Button {
                    id: button_close
                    anchors.horizontalCenter: parent.horizontalCenter

                    width: 20 * screenScale
                    height: 20 * screenScale
                    color: "#955"

                    caption: "X"

                    onClicked: {
                        text_edit.text = root.text
                        root.setEdit(false)
                    }
                }

                Button {
                    id: button_save
                    anchors.horizontalCenter: parent.horizontalCenter

                    width: 20 * screenScale
                    height: 20 * screenScale
                    color: "#595"

                    caption: "S"

                    onClicked: {
                        root.text = text_edit.text
                    }
                }
            }

            states: [
                State {
                    name: "edit"
                    when: root.editable
                    PropertyChanges { target: tool_panel; height: tool_column_edit.height + 10 }
                    PropertyChanges { target: tool_column_view; visible: false }
                    PropertyChanges { target: tool_column_edit; visible: true }
                }
            ]

            Timer {
                id: tool_hide_timer
                repeat: false
                running: false
                interval: 5000

                onTriggered: {
                    tool_panel.hide()
                }
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

