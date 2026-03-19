// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls

/**
 * Splash / loading screen shown while the application initialises.
 */
Item {
    anchors.fill: parent

    Column {
        anchors.centerIn: parent
        spacing: 24

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#e0e0e0"
            font.bold: true
            font.pixelSize: 42
            text: "aSocial"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#a0a0a0"
            font.pixelSize: 16
            text: qsTr("Distributed Private Social Network")
        }
        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            palette.dark: "#80c0ff"
            running: true
        }
    }
}
