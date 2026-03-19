// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Layouts

/**
 * Section header label used inside HamburgerMenu for visual grouping.
 */
Item {
    property alias text: label.text

    Layout.fillWidth: true
    height: 32

    Text {
        id: label

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        color: "#4da6ff"
        font.bold: true
        font.capitalization: Font.AllUppercase
        font.pixelSize: 11
        leftPadding: 4
    }
}
