// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Slide-in menu from the left edge (three-bar / hamburger menu).
 * Provides access to profile management, settings, persona switching,
 * and application-wide actions.
 */
Item {
    id: menuRoot

    property bool isOpen: false

    function close() {
        isOpen = false
    }
    function open() {
        isOpen = true
    }
    function toggle() {
        isOpen = !isOpen
    }

    anchors.fill: parent
    visible: menuPanel.x > -menuPanel.width

    // Dim overlay
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.4)
        opacity: isOpen ? 1.0 : 0.0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: 250
            }
        }

        MouseArea {
            anchors.fill: parent

            onClicked: menuRoot.close()
        }
    }

    // Menu panel
    Rectangle {
        id: menuPanel

        border.color: Qt.rgba(1, 1, 1, 0.06)
        border.width: 1
        color: Qt.rgba(0.08, 0.08, 0.12, 0.95)
        height: parent.height
        width: Math.min(320, parent.width * 0.8)
        x: isOpen ? 0 : -width

        Behavior on x {
            NumberAnimation {
                duration: 300
                easing.type: Easing.OutCubic
            }
        }

        Flickable {
            anchors.fill: parent
            anchors.margins: 16
            clip: true
            contentHeight: menuColumn.implicitHeight

            ColumnLayout {
                id: menuColumn

                spacing: 4
                width: parent.width

                // Header
                ColumnLayout {
                    Layout.bottomMargin: 16
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        color: "#e0e0e0"
                        font.bold: true
                        font.pixelSize: 24
                        text: "aSocial"
                    }
                    Text {
                        Layout.fillWidth: true
                        color: "#808080"
                        elide: Text.ElideRight
                        font.pixelSize: 13
                        text: Backend.profileOpen ? Backend.profileName + " / " + Backend.activePersonaName : qsTr("Not logged in")
                    }
                }
                MenuSeparator {
                    Layout.fillWidth: true
                    padding: 0
                }

                // --- Profile section ---
                MenuSectionHeader {
                    text: qsTr("Profile")
                }
                MenuItem {
                    Layout.fillWidth: true
                    icon.name: "drive-harddisk"
                    text: qsTr("Initialize Storage")
                    visible: !Backend.containerInitialized

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* handled by InitScreen */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Open Profile")
                    visible: Backend.containerInitialized && !Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: menuRoot.close()
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Profile Info")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: show profile detail dialog */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Close Profile")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: {
                        Backend.closeProfile()
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Export Profile")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: show export dialog */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Import Profile")
                    visible: Backend.containerInitialized

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: show import dialog */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Delete Profile")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: confirmation dialog then Backend.deleteProfile() */
                        menuRoot.close()
                    }
                }
                MenuSeparator {
                    Layout.fillWidth: true
                    padding: 0
                }

                // --- Persona section ---
                MenuSectionHeader {
                    text: qsTr("Persona")
                    visible: Backend.profileOpen
                }
                Repeater {
                    model: Backend.profileOpen ? Backend.listPersonas() : []

                    delegate: MenuItem {
                        required property var modelData

                        Layout.fillWidth: true
                        text: modelData.displayName + (modelData.isDefault ? " [default]" : "") + (modelData.isActive ? "  ◀" : "")

                        contentItem: Text {
                            color: "#ffffff"
                            text: parent.text
                        }

                        onTriggered: {
                            Backend.selectPersona(modelData.uid)
                            menuRoot.close()
                        }
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("+ Create Persona")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: persona creation dialog */
                        menuRoot.close()
                    }
                }
                MenuSeparator {
                    Layout.fillWidth: true
                    padding: 0
                    visible: Backend.profileOpen
                }

                // --- Data section ---
                MenuSectionHeader {
                    text: qsTr("Social")
                    visible: Backend.profileOpen
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Contacts")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: contacts list view */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Groups")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: groups list view */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Messages")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: messages view */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Events / Timeline")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: events list view */
                        menuRoot.close()
                    }
                }
                MenuSeparator {
                    Layout.fillWidth: true
                    padding: 0
                }

                // --- Settings section ---
                MenuSectionHeader {
                    text: qsTr("Application")
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Settings")

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: settings view */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Profile Parameters")
                    visible: Backend.profileOpen

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: { /* TODO: params view */
                        menuRoot.close()
                    }
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("About aSocial")

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: {
                        aboutDialog.open()
                        menuRoot.close()
                    }
                }
                MenuSeparator {
                    Layout.fillWidth: true
                    padding: 0
                }
                MenuItem {
                    Layout.fillWidth: true
                    text: qsTr("Quit")

                    contentItem: Text {
                        color: "#ffffff"
                        text: parent.text
                    }

                    onTriggered: Backend.exitApp()
                }
            }
        }
    }

    // About dialog
    Dialog {
        id: aboutDialog

        anchors.centerIn: Overlay.overlay
        height: 260
        modal: true
        title: qsTr("About aSocial")
        width: 340

        background: Rectangle {
            border.color: Qt.rgba(1, 1, 1, 0.1)
            color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
            radius: 12
        }
        contentItem: Column {
            padding: 16
            spacing: 12

            Text {
                color: "#e0e0e0"
                font.bold: true
                font.pixelSize: 18
                text: "aSocial v" + (Backend.getSetting("version") || "0.1.0")
            }
            Text {
                color: "#a0a0a0"
                font.pixelSize: 12
                text: qsTr("Distributed Private Social Network\n\n" + "Fully open-source, peer-to-peer, encrypted.\n" + "Security by cryptography – no trust required.\n\n" + "Licensed under GPL-3.0-or-later")
                width: 300
                wrapMode: Text.WordWrap
            }
        }
    }
}
