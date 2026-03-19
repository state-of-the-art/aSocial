// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick

/**
 * Overlay that shows animated toast notifications (info / error).
 * Toasts appear at the bottom, stack upwards, and auto-dismiss.
 */
Item {
    id: overlay

    function addToast(text, color) {
        toastModel.insert(0, {
            text: text,
            toastColor: color,
            toastId: Date.now()
        })
        if (toastModel.count > 5)
            toastModel.remove(toastModel.count - 1)
    }
    function showError(msg) {
        addToast(msg, "#ff5050")
    }
    function showInfo(msg) {
        addToast(msg, "#4da6ff")
    }

    ListModel {
        id: toastModel

    }
    Column {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 100
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8

        Repeater {
            model: toastModel

            delegate: Rectangle {
                id: toastRect

                required property int index
                required property string text
                required property string toastColor
                required property real toastId

                border.color: toastColor
                border.width: 1
                color: Qt.rgba(0.1, 0.1, 0.15, 0.9)
                height: 40
                opacity: 0
                radius: 20
                width: toastText.implicitWidth + 32

                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                    }
                }

                Component.onCompleted: opacity = 1

                Text {
                    id: toastText

                    anchors.centerIn: parent
                    color: toastRect.toastColor
                    font.pixelSize: 13
                    text: toastRect.text
                }
                Timer {
                    interval: 4000
                    running: true

                    onTriggered: {
                        toastRect.opacity = 0
                        removeTimer.start()
                    }
                }
                Timer {
                    id: removeTimer

                    interval: 300

                    onTriggered: {
                        if (toastRect.index >= 0 && toastRect.index < toastModel.count)
                            toastModel.remove(toastRect.index)
                    }
                }
            }
        }
    }
}
