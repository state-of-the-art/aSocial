// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Pop-up message compose/view window with animated entrance.
 * Supports Markdown body, inline images (via attachments), and
 * recipient selection.
 */
Popup {
    id: msgPopup

    property var existingMessages: []
    property string recipientName: ""
    property string recipientUid: ""
    property string threadUid: ""

    function sendMsg() {
        if (composeField.text.length === 0)
            return
        var uid = Backend.sendMessage(recipientUid, composeField.text)
        if (uid.length > 0) {
            composeField.text = ""
            // Reload messages
            existingMessages = Backend.listMessages(50)
        }
    }

    anchors.centerIn: Overlay.overlay
    height: Math.min(600, parent.height - 80)
    modal: true
    width: Math.min(500, parent.width - 48)

    background: Rectangle {
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1
        color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
        radius: 16
    }
    contentItem: ColumnLayout {
        spacing: 12

        // Header
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                color: "#e0e0e0"
                font.bold: true
                font.pixelSize: 18
                text: recipientName.length > 0 ? qsTr("Message to %1").arg(recipientName) : qsTr("New Message")
            }
            RoundButton {
                flat: true
                height: 32
                width: 32

                contentItem: Text {
                    color: "#a0a0a0"
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    text: "×"
                }

                onClicked: msgPopup.close()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        // Message history (scrollable)
        ListView {
            id: messageList

            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: existingMessages
            spacing: 8

            delegate: Rectangle {
                required property var modelData

                anchors.left: modelData.senderContactUid !== "" ? messageList.contentItem.left : undefined
                anchors.right: modelData.senderContactUid === "" ? messageList.contentItem.right : undefined
                color: modelData.senderContactUid === "" ? Qt.rgba(0.2, 0.3, 0.5, 0.6)   // self (right-aligned)
                : Qt.rgba(0.2, 0.2, 0.25, 0.6)   // other

                height: msgBody.implicitHeight + 24
                radius: 8
                width: messageList.width - 16

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 4

                    Text {
                        id: msgBody

                        color: "#e0e0e0"
                        font.pixelSize: 13
                        text: modelData.body || ""
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        color: "#707070"
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignRight
                        text: modelData.createdAt || ""
                        width: parent.width
                    }
                }
            }
        }

        // Compose area
        Rectangle {
            Layout.fillWidth: true
            Layout.minimumHeight: 56
            Layout.preferredHeight: composeField.implicitHeight + 16
            border.color: composeField.activeFocus ? "#4da6ff" : Qt.rgba(1, 1, 1, 0.1)
            color: Qt.rgba(1, 1, 1, 0.06)
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                TextArea {
                    id: composeField

                    Layout.fillWidth: true
                    background: null
                    color: "#e0e0e0"
                    font.pixelSize: 13
                    placeholderText: qsTr("Type a message (Markdown supported)...")
                    placeholderTextColor: "#555"
                    wrapMode: TextArea.Wrap

                    Keys.onReturnPressed: function (event) {
                        if (event.modifiers & Qt.ShiftModifier) {
                            // Shift+Enter inserts newline (default)
                        } else {
                            sendMsg()
                            event.accepted = true
                        }
                    }
                }
                RoundButton {
                    enabled: composeField.text.length > 0
                    flat: true
                    height: 40
                    width: 40

                    contentItem: Text {
                        color: composeField.text.length > 0 ? "#4da6ff" : "#555"
                        font.pixelSize: 18
                        horizontalAlignment: Text.AlignHCenter
                        text: "➤"
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: sendMsg()

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }
        }
    }
    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                duration: 350
                from: 0
                property: "opacity"
                to: 1
            }
            NumberAnimation {
                duration: 350
                easing.type: Easing.OutBack
                from: 0.7
                property: "scale"
                to: 1.0
            }
        }
    }
    exit: Transition {
        ParallelAnimation {
            NumberAnimation {
                duration: 200
                from: 1
                property: "opacity"
                to: 0
            }
            NumberAnimation {
                duration: 200
                easing.type: Easing.InBack
                from: 1.0
                property: "scale"
                to: 0.8
            }
        }
    }
}
