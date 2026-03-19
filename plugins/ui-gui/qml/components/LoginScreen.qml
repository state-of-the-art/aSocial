// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Login screen: enter password to open an existing profile or create a new one.
 * Shown when the VFS container exists but no profile is currently open.
 */
Item {
    property bool createMode: false

    function doAction() {
        errorLabel.text = ""
        if (passwordField.text.length === 0) {
            errorLabel.text = qsTr("Password is required")
            return
        }
        if (createMode) {
            if (nameField.text.length === 0) {
                errorLabel.text = qsTr("Display name is required")
                return
            }
            if (passwordField.text !== confirmField.text) {
                errorLabel.text = qsTr("Passwords do not match")
                return
            }
            Backend.createProfile(passwordField.text, nameField.text)
        } else {
            Backend.openProfile(passwordField.text)
        }
        passwordField.text = ""
        confirmField.text = ""
    }

    anchors.fill: parent

    Rectangle {
        anchors.centerIn: parent
        border.color: Qt.rgba(1, 1, 1, 0.08)
        color: Qt.rgba(0, 0, 0, 0.55)
        height: formCol.implicitHeight + 64
        opacity: 0
        radius: 16
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
            id: formCol

            anchors.fill: parent
            anchors.margins: 32
            spacing: 16

            Text {
                Layout.alignment: Qt.AlignHCenter
                color: "#e0e0e0"
                font.bold: true
                font.pixelSize: 28
                text: "aSocial"
            }
            Text {
                Layout.alignment: Qt.AlignHCenter
                color: "#a0a0a0"
                font.pixelSize: 16
                text: createMode ? qsTr("Create New Profile") : qsTr("Open Profile")
            }

            // Display name (only in create mode)
            TextField {
                id: nameField

                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: "#e0e0e0"
                font.pixelSize: 14
                placeholderText: qsTr("Display name")
                placeholderTextColor: "#666"
                visible: createMode

                background: Rectangle {
                    border.color: nameField.activeFocus ? "#4da6ff" : Qt.rgba(1, 1, 1, 0.15)
                    color: Qt.rgba(1, 1, 1, 0.08)
                    radius: 8

                    Behavior on border.color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }
            }

            // Password
            TextField {
                id: passwordField

                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: "#e0e0e0"
                echoMode: TextInput.Password
                font.pixelSize: 14
                placeholderText: qsTr("Password")
                placeholderTextColor: "#666"

                background: Rectangle {
                    border.color: passwordField.activeFocus ? "#4da6ff" : Qt.rgba(1, 1, 1, 0.15)
                    color: Qt.rgba(1, 1, 1, 0.08)
                    radius: 8

                    Behavior on border.color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }

                Keys.onReturnPressed: doAction()
            }

            // Confirm password (only in create mode)
            TextField {
                id: confirmField

                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: "#e0e0e0"
                echoMode: TextInput.Password
                font.pixelSize: 14
                placeholderText: qsTr("Confirm password")
                placeholderTextColor: "#666"
                visible: createMode

                background: Rectangle {
                    border.color: confirmField.activeFocus ? "#4da6ff" : Qt.rgba(1, 1, 1, 0.15)
                    color: Qt.rgba(1, 1, 1, 0.08)
                    radius: 8

                    Behavior on border.color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }

                Keys.onReturnPressed: doAction()
            }

            // Error message
            Text {
                id: errorLabel

                Layout.alignment: Qt.AlignHCenter
                color: "#ff6060"
                font.pixelSize: 12
                text: ""
                visible: text.length > 0
            }

            // Primary action button
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                font.bold: true
                font.pixelSize: 15
                text: createMode ? qsTr("Create Profile") : qsTr("Open Profile")

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

                onClicked: doAction()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }

            // Toggle create / open
            Text {
                Layout.alignment: Qt.AlignHCenter
                color: "#4da6ff"
                font.pixelSize: 13
                font.underline: toggleMa.containsMouse
                text: createMode ? qsTr("Already have a profile? Open it") : qsTr("No profile yet? Create one")

                MouseArea {
                    id: toggleMa

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        createMode = !createMode
                        errorLabel.text = ""
                    }
                }
            }
        }
    }
}
