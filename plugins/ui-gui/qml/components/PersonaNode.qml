// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls

/**
 * A single persona / contact node rendered as a circular avatar
 * with name and optional birthday below.  Group membership is
 * visualised as coloured rings around the avatar.  Trust level
 * affects the glow intensity.
 *
 * Declares `contextActions` so the RadialMenu can discover
 * context-specific actions when the user right-clicks / long-presses
 * over this node.
 */
Item {
    id: nodeRoot

    property string birthday: ""
    property var contactActions: [
        {
            label: qsTr("View Info"),
            icon: "\u2139",
            action: "viewContact",
            section: "contact",
            data: {
                uid: contactUid,
                name: displayName
            },
            handler: function (d) { /* TODO: open contact info */ }
        },
        {
            label: qsTr("Send Message"),
            icon: "\u2709",
            action: "messageContact",
            section: "contact",
            data: {
                uid: contactUid,
                name: displayName
            },
            handler: function (d) { /* TODO: open message compose */ }
        },
        {
            label: qsTr("Edit Contact"),
            icon: "\u270E",
            action: "editContact",
            section: "contact",
            data: {
                uid: contactUid
            },
            handler: function (d) { /* TODO: open contact editor */ }
        },
        {
            label: qsTr("Add to Group"),
            icon: "\u2726",
            action: "addToGroup",
            section: "contact",
            data: {
                uid: contactUid
            },
            handler: function (d) { /* TODO: group picker dialog */ }
        },
        {
            label: qsTr("Delete"),
            icon: "\u2716",
            action: "deleteContact",
            section: "contact",
            data: {
                uid: contactUid
            },
            handler: function (d) {
                Backend.deleteContact(d.uid)
            }
        },
    ]
    property string contactUid: ""

    // ---- context-aware actions -------------------------------------------
    // RadialMenu walks the item tree and picks these up automatically.

    property var contextActions: isCentral ? personaActions : contactActions
    property string displayName: ""
    property var groups: []
    property bool isCentral: false
    property bool isHovered: hoverHandler.hovered
    property int nodeWidth: 100
    property var personaActions: [
        {
            label: qsTr("Edit Persona"),
            icon: "\u270E",
            action: "editPersona",
            section: "persona",
            data: {
                uid: contactUid,
                name: displayName
            },
            handler: function (d) { /* TODO: open persona editor */ }
        },
        {
            label: qsTr("Send Message"),
            icon: "\u2709",
            action: "newMessage",
            section: "persona",
            data: {
                uid: contactUid,
                name: displayName
            },
            handler: function (d) { /* TODO: open message compose */ }
        },
        {
            label: qsTr("Create Event"),
            icon: "\u2605",
            action: "newEvent",
            section: "persona",
            data: {
                uid: contactUid
            },
            handler: function (d) { /* TODO: open event creator */ }
        },
        {
            label: qsTr("Switch To"),
            icon: "\u21C4",
            action: "switchPersona",
            section: "persona",
            data: {
                uid: contactUid
            },
            handler: function (d) {
                Backend.selectPersona(d.uid)
            }
        },
    ]
    property int trustLevel: 50

    signal clicked(string uid)

    height: nodeWidth + nameLabel.height + birthdayLabel.height + 16
    width: nodeWidth

    // ---- group rings -----------------------------------------------------

    Repeater {
        model: nodeRoot.groups

        delegate: Rectangle {
            required property int index
            required property var modelData
            property int ringSize: nodeWidth + 20 + index * 16

            anchors.centerIn: avatarCircle
            border.color: modelData.color || "#888888"
            border.width: 3
            color: "transparent"
            height: ringSize
            opacity: 0.7
            radius: ringSize / 2
            width: ringSize

            Text {
                anchors.centerIn: parent
                color: modelData.color || "#888"
                font.bold: true
                font.pixelSize: Math.max(8, ringSize * 0.12)
                opacity: 0.15
                rotation: index * 15
                text: modelData.name || ""
            }
        }
    }

    // ---- avatar circle ---------------------------------------------------

    Rectangle {
        id: avatarCircle

        anchors.horizontalCenter: parent.horizontalCenter
        border.color: {
            if (isHovered)
                return "#80c0ff"
            if (isCentral)
                return "#4da6ff"
            var t = trustLevel / 100.0
            return Qt.rgba(1.0 - t, t, 0.3, 0.8)
        }
        border.width: isCentral ? 4 : 2 + trustLevel / 25
        color: isCentral ? Qt.rgba(0.3, 0.5, 0.8, 0.9) : Qt.rgba(0.25, 0.25, 0.3, 0.85)
        height: nodeWidth
        radius: nodeWidth / 2
        scale: isHovered ? 1.12 : 1.0
        width: nodeWidth
        y: (nodeRoot.groups.length || 0) * 8

        Behavior on border.color {
            ColorAnimation {
                duration: 300
            }
        }
        Behavior on border.width {
            NumberAnimation {
                duration: 300
            }
        }
        Behavior on scale {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutBack
            }
        }

        // Glow on hover
        Rectangle {
            anchors.centerIn: parent
            border.color: isCentral ? "#4da6ff" : "#80c0ff"
            border.width: isHovered ? 6 : 0
            color: "transparent"
            height: parent.height + 20
            opacity: isHovered ? 0.5 : 0
            radius: width / 2
            width: parent.width + 20

            Behavior on border.width {
                NumberAnimation {
                    duration: 300
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
        }

        // Initials placeholder
        Text {
            anchors.centerIn: parent
            color: "#e0e0e0"
            font.bold: true
            font.pixelSize: nodeWidth * 0.3
            text: {
                var parts = displayName.split(" ")
                var initials = ""
                for (var i = 0; i < Math.min(2, parts.length); i++)
                    if (parts[i].length > 0)
                        initials += parts[i][0].toUpperCase()
                return initials
            }
        }
    }

    // ---- labels -----------------------------------------------------------

    Text {
        id: nameLabel

        anchors.horizontalCenter: avatarCircle.horizontalCenter
        anchors.top: avatarCircle.bottom
        anchors.topMargin: 6
        color: "#e0e0e0"
        elide: Text.ElideRight
        font.bold: isCentral
        font.pixelSize: isCentral ? 18 : 13
        horizontalAlignment: Text.AlignHCenter
        text: displayName
        width: nodeWidth + 40
    }
    Text {
        id: birthdayLabel

        function formatDate(iso) {
            var d = new Date(iso)
            if (isNaN(d.getTime()))
                return iso
            return Qt.formatDate(d, "dd.MM.yyyy")
        }

        anchors.horizontalCenter: nameLabel.horizontalCenter
        anchors.top: nameLabel.bottom
        anchors.topMargin: 2
        color: "#909090"
        font.pixelSize: 11
        text: birthday ? formatDate(birthday) : ""
        visible: birthday.length > 0
    }

    // ---- input handlers ---------------------------------------------------

    HoverHandler {
        id: hoverHandler

        cursorShape: Qt.PointingHandCursor
    }
    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: nodeRoot.clicked(contactUid)
    }

    // Right-click and long-press: do NOT capture here. Let the event
    // propagate to the ApplicationWindow so the RadialMenu can show.
    // The RadialMenu hit-tests and discovers contextActions on this item.
}
