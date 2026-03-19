// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Main profile screen: persona connection graph on top with
 * family tree below the center persona, and an interactive
 * timeline bar at the bottom.
 *
 * The graphCanvas item carries background-level contextActions
 * that the RadialMenu discovers when no more specific child
 * element is under the pointer.
 *
 * The view supports scroll-wheel / pinch zoom to reveal more
 * connections at greater distances.
 */
Item {
    id: mainScreen

    property var contacts: []
    property var events: []
    property var familyContacts: []
    property var friendContacts: []

    // Expose the graph canvas so Main.qml can point the RadialMenu at it
    property alias graphCanvas: graphCanvas
    property var groups: []

    function reloadData() {
        contacts = Backend.listContacts()
        groups = Backend.listGroups()
        events = Backend.listEvents()

        var fam = [], fri = []
        for (var i = 0; i < contacts.length; i++) {
            if (contacts[i].relationshipType === "family")
                fam.push(contacts[i])
            else
                fri.push(contacts[i])
        }
        familyContacts = fam
        friendContacts = fri
    }

    anchors.fill: parent

    Component.onCompleted: reloadData()

    Connections {
        function onDataChanged() {
            reloadData()
        }

        target: Backend
    }

    // Zoomable / pannable graph area
    Flickable {
        id: graphFlickable

        anchors.bottom: timelineBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: graphCanvas.height * graphCanvas.scale
        contentWidth: graphCanvas.width * graphCanvas.scale

        Component.onCompleted: {
            // Center the central persona in the viewport on first load
            Qt.callLater(function () {
                var cx = graphCanvas.width / 2 - graphFlickable.width / 2
                var cy = graphCanvas.height * 0.35 - graphFlickable.height / 2
                contentX = Math.max(0, Math.min(cx, contentWidth - width))
                contentY = Math.max(0, Math.min(cy, contentHeight - height))
            })
        }

        WheelHandler {
            property: "scale"
            target: graphCanvas

            onActiveChanged: {
                if (graphCanvas.scale < 0.3)
                    graphCanvas.scale = 0.3
                if (graphCanvas.scale > 3.0)
                    graphCanvas.scale = 3.0
            }
        }
        PinchHandler {
            maximumScale: 3.0
            minimumScale: 0.3
            target: graphCanvas
        }
        Item {
            id: graphCanvas

            // ---- background-level context actions -------------------------
            // The RadialMenu will find these when the click lands on empty
            // canvas space (no child with its own contextActions above).
            property var contextActions: [
                {
                    label: qsTr("Add Contact"),
                    icon: "+",
                    action: "addContact",
                    section: "background",
                    handler: function () { /* TODO: contact creation dialog */ }
                },
                {
                    label: qsTr("New Message"),
                    icon: "\u2709",
                    action: "newMessage",
                    section: "background",
                    handler: function () { /* TODO: message compose window */ }
                },
                {
                    label: qsTr("New Event"),
                    icon: "\u2605",
                    action: "newEvent",
                    section: "background",
                    handler: function () { /* TODO: event creation dialog */ }
                },
                {
                    label: qsTr("New Group"),
                    icon: "\u2726",
                    action: "newGroup",
                    section: "background",
                    handler: function () { /* TODO: group creation dialog */ }
                },
                {
                    label: qsTr("Search"),
                    icon: "\u2315",
                    action: "search",
                    section: "background",
                    handler: function () { /* TODO: search overlay */ }
                },
                {
                    label: qsTr("Refresh"),
                    icon: "\u21BB",
                    action: "refresh",
                    section: "background",
                    handler: function () {
                        Backend.refresh()
                    }
                },
            ]

            height: (mainScreen.height - 100) * 2
            scale: 1.0
            transformOrigin: Item.Center
            width: mainScreen.width * 2

            Behavior on scale {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.OutQuad
                }
            }

            // Connection lines (drawn behind nodes)
            ConnectionLines {
                id: connectionLines

                anchors.fill: parent
                centerX: centralPersona.x + centralPersona.width / 2
                centerY: centralPersona.y + centralPersona.height / 2
                contactNodes: contactRepeater
                contacts: mainScreen.friendContacts
                groups: mainScreen.groups
            }

            // Central persona (the active persona)
            PersonaNode {
                id: centralPersona

                displayName: Backend.activePersonaName || "You"
                groups: {
                    var g = Backend.listGroups()
                    var result = []
                    for (var i = 0; i < g.length; i++)
                        result.push({
                            name: g[i].name,
                            color: g[i].color || "#888888"
                        })
                    return result
                }
                isCentral: true
                nodeWidth: 160
                trustLevel: 100
                x: parent.width / 2 - width / 2
                y: parent.height * 0.35 - height / 2

                onClicked: function (uid) {
                // TODO: Open persona detail view
                }
            }

            // Friend contacts arranged in a circle
            Repeater {
                id: contactRepeater

                model: mainScreen.friendContacts

                PersonaNode {
                    id: friendNode

                    property real angle: (index / Math.max(1, mainScreen.friendContacts.length)) * 2 * Math.PI - Math.PI / 2
                    required property int index
                    required property var modelData
                    property real orbitRadius: 280

                    birthday: modelData.birthday || ""
                    contactUid: modelData.uid || ""
                    displayName: modelData.displayName || ""
                    groups: {
                        var g = Backend.groupsForContact(modelData.uid)
                        var result = []
                        for (var i = 0; i < g.length; i++)
                            result.push({
                                name: g[i].name,
                                color: g[i].color || "#888888"
                            })
                        return result
                    }
                    nodeWidth: 100
                    trustLevel: modelData.trustLevel || 0
                    x: centralPersona.x + centralPersona.width / 2 + Math.cos(angle) * orbitRadius - width / 2
                    y: centralPersona.y + centralPersona.height / 2 + Math.sin(angle) * orbitRadius - height / 2

                    onClicked: function (uid) {
                    // TODO: Open contact detail view
                    }
                }
            }

            // Family tree below the central persona
            FamilyTree {
                id: familyTree

                anchors.horizontalCenter: centralPersona.horizontalCenter
                centralPersonaY: centralPersona.y + centralPersona.height
                familyMembers: mainScreen.familyContacts
                width: parent.width * 0.6
                y: centralPersona.y + centralPersona.height + 60
            }
        }
    }

    // Interactive timeline at the bottom
    TimelineBar {
        id: timelineBar

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        events: mainScreen.events
        height: 80
    }
}
