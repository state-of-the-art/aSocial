// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick

/**
 * Family tree section displayed below the central persona.
 * Shows parents, grandparents and siblings arranged in
 * hierarchical rows with connecting lines.
 *
 * The tree itself provides a "family" section of context actions,
 * and each PersonaNode inside inherits its own contact-level actions.
 */
Item {
    id: familyRoot

    property real centralPersonaY: 0

    // ---- family-level context actions ------------------------------------
    property var contextActions: [
        {
            label: qsTr("Add Relative"),
            icon: "+",
            action: "addFamilyMember",
            section: "family",
            handler: function () { /* TODO: add family member dialog */ }
        },
    ]
    property var familyMembers: []
    property var parents: {
        var result = []
        for (var i = 0; i < familyMembers.length; i++) {
            var m = familyMembers[i]
            if (!m.parentContactUid || m.parentContactUid === "")
                result.push(m)
        }
        return result
    }
    property var siblings: {
        var result = []
        for (var i = 0; i < familyMembers.length; i++) {
            var m = familyMembers[i]
            if (m.parentContactUid && m.parentContactUid !== "")
                result.push(m)
        }
        return result
    }

    height: generationColumn.implicitHeight + 40

    // Vertical connector
    Rectangle {
        id: verticalLine

        anchors.horizontalCenter: parent.horizontalCenter
        color: Qt.rgba(1, 1, 1, 0.25)
        height: 40
        width: 2
        y: 0
    }
    Column {
        id: generationColumn

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: verticalLine.bottom
        spacing: 40

        // Parents row
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 40
            visible: parents.length > 0

            Repeater {
                model: familyRoot.parents

                delegate: PersonaNode {
                    required property var modelData

                    birthday: modelData.birthday || ""
                    contactUid: modelData.uid || ""
                    displayName: modelData.displayName || ""
                    nodeWidth: 80
                    trustLevel: modelData.trustLevel || 0
                }
            }
        }

        // Siblings row
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 30
            visible: siblings.length > 0

            Repeater {
                model: familyRoot.siblings

                delegate: PersonaNode {
                    required property var modelData

                    birthday: modelData.birthday || ""
                    contactUid: modelData.uid || ""
                    displayName: modelData.displayName || ""
                    nodeWidth: 70
                    trustLevel: modelData.trustLevel || 0
                }
            }
        }
    }

    // Horizontal connector lines between parents
    Canvas {
        anchors.fill: generationColumn

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.2)
            ctx.lineWidth = 1.5

            if (parents.length > 1) {
                var startX = generationColumn.width / 2 - (parents.length - 1) * 60
                var endX = generationColumn.width / 2 + (parents.length - 1) * 60
                var lineY = 50
                ctx.beginPath()
                ctx.moveTo(startX, lineY)
                ctx.lineTo(endX, lineY)
                ctx.stroke()
            }
        }
    }
}
