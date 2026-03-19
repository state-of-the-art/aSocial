// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * First-time setup screen: create a new VFS encrypted container.
 * Shown when no container file exists on disk yet.
 */
Item {
    anchors.fill: parent

    Rectangle {
        anchors.centerIn: parent
        border.color: Qt.rgba(1, 1, 1, 0.08)
        color: Qt.rgba(0, 0, 0, 0.55)
        height: col.implicitHeight + 64
        opacity: 0
        radius: 16

        // Entrance animation
        scale: 0.85
        width: Math.min(420, parent.width - 48)

        Behavior on opacity {
            NumberAnimation {
                duration: 400
            }
        }
        Behavior on scale {
            NumberAnimation {
                duration: 400
                easing.type: Easing.OutBack
            }
        }

        Component.onCompleted: {
            scale = 1.0
            opacity = 1.0
        }

        ColumnLayout {
            id: col

            anchors.fill: parent
            anchors.margins: 32
            spacing: 20

            Text {
                Layout.alignment: Qt.AlignHCenter
                color: "#e0e0e0"
                font.bold: true
                font.pixelSize: 32
                text: "aSocial"
            }
            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                color: "#a0a0a0"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Welcome! Let's create your encrypted storage.")
                wrapMode: Text.WordWrap
            }

            // Size slider
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    color: "#c0c0c0"
                    font.pixelSize: 13
                    text: qsTr("Container size: %1 MB").arg(sizeSlider.value)
                }
                Slider {
                    id: sizeSlider

                    Layout.fillWidth: true
                    from: 64
                    stepSize: 64
                    to: 4096
                    value: 512

                    background: Rectangle {
                        color: Qt.rgba(1, 1, 1, 0.1)
                        height: 6
                        implicitHeight: 6
                        implicitWidth: 200
                        radius: 3
                        width: sizeSlider.availableWidth
                        x: sizeSlider.leftPadding
                        y: sizeSlider.topPadding + sizeSlider.availableHeight / 2 - height / 2

                        Rectangle {
                            color: "#4da6ff"
                            height: parent.height
                            radius: 3
                            width: sizeSlider.visualPosition * parent.width
                        }
                    }
                    handle: Rectangle {
                        color: sizeSlider.pressed ? "#80c0ff" : "#4da6ff"
                        height: 24
                        radius: 12
                        width: 24
                        x: sizeSlider.leftPadding + sizeSlider.visualPosition * (sizeSlider.availableWidth - width)
                        y: sizeSlider.topPadding + sizeSlider.availableHeight / 2 - height / 2
                    }
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                font.bold: true
                font.pixelSize: 15
                text: qsTr("Create Container")

                background: Rectangle {
                    color: parent.hovered ? "#5bb0ff" : "#4da6ff"
                    radius: 8

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }
                contentItem: Text {
                    color: "#ffffff"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    text: parent.text
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    enabled = false
                    Backend.initContainer(sizeSlider.value)
                    enabled = true
                }

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                color: "#808080"
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Your data will be stored in an encrypted VFS container.\nMultiple profiles can share the same container\nwith plausible deniability.")
                wrapMode: Text.WordWrap
            }
        }
    }
}
