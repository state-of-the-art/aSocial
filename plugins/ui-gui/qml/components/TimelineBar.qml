// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls

/**
 * Interactive timeline bar at the bottom of the main screen.
 *
 * Displays life events as blocks (range) or dots (point) along a
 * scrollable, zoomable time axis from the persona's birth to today.
 *
 * The bar itself and each event item declare contextActions so the
 * RadialMenu can discover them via hit-testing.
 */
Item {
    id: timelineRoot

    // ---- timeline-level context actions -----------------------------------
    property var contextActions: [
        {
            label: qsTr("New Event"),
            icon: "\u2605",
            action: "newTimelineEvent",
            section: "timeline",
            handler: function () { /* TODO: event creation dialog */ }
        },
        {
            label: qsTr("Zoom In"),
            icon: "+",
            action: "timelineZoomIn",
            section: "timeline",
            handler: function () {
                timelineRoot.zoomLevel = Math.min(10, timelineRoot.zoomLevel * 1.5)
            }
        },
        {
            label: qsTr("Zoom Out"),
            icon: "\u2212",
            action: "timelineZoomOut",
            section: "timeline",
            handler: function () {
                timelineRoot.zoomLevel = Math.max(0.5, timelineRoot.zoomLevel / 1.5)
            }
        },
    ]
    property real endMs: Date.now()
    property var events: []
    property real rangeMs: endMs - startMs
    property real startMs: {
        var earliest = Date.now() - 30 * 365.25 * 86400000
        for (var i = 0; i < events.length; i++) {
            if (events[i].startedMs && events[i].startedMs < earliest)
                earliest = events[i].startedMs
        }
        return earliest
    }
    property real zoomLevel: 1.0

    function categoryColor(cat) {
        switch (cat) {
        case "education":
            return "#4da6ff"
        case "work":
            return "#66cc66"
        case "travel":
            return "#cc66cc"
        case "personal":
            return "#ffaa44"
        default:
            return "#888888"
        }
    }
    function timeToX(ms) {
        if (rangeMs <= 0)
            return 0
        return ((ms - startMs) / rangeMs) * timelineContent.width
    }

    clip: true

    // Background
    Rectangle {
        anchors.fill: parent
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1
        color: Qt.rgba(0, 0, 0, 0.5)
    }

    // Scrollable content
    Flickable {
        id: timelineFlickable

        anchors.fill: parent
        anchors.margins: 4
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: parent.height - 8
        contentWidth: timelineContent.width
        flickableDirection: Flickable.HorizontalFlick

        WheelHandler {
            onWheel: function (event) {
                var factor = event.angleDelta.y > 0 ? 1.2 : 0.8
                timelineRoot.zoomLevel = Math.max(0.5, Math.min(10.0, timelineRoot.zoomLevel * factor))
            }
        }
        Item {
            id: timelineContent

            height: parent.height
            width: Math.max(timelineRoot.width * zoomLevel * 2, timelineRoot.width)

            // Time axis line
            Rectangle {
                id: axisLine

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 10
                color: Qt.rgba(1, 1, 1, 0.3)
                height: 2
            }

            // Year markers
            Repeater {
                model: {
                    var markers = []
                    var startYear = new Date(startMs).getFullYear()
                    var endYear = new Date(endMs).getFullYear()
                    var step = Math.max(1, Math.round((endYear - startYear) / (10 * zoomLevel)))
                    for (var y = startYear; y <= endYear; y += step) {
                        var ms = new Date(y, 0, 1).getTime()
                        markers.push({
                            year: y,
                            ms: ms
                        })
                    }
                    return markers
                }

                delegate: Item {
                    required property var modelData

                    anchors.verticalCenter: axisLine.verticalCenter
                    x: timeToX(modelData.ms)

                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: Qt.rgba(1, 1, 1, 0.3)
                        height: 12
                        width: 1
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 14
                        color: "#808080"
                        font.pixelSize: 10
                        text: modelData.year
                    }
                }
            }

            // Event items (each carries its own contextActions)
            Repeater {
                model: timelineRoot.events

                delegate: Item {
                    id: eventDelegate

                    // ---- per-event context actions ------------------------
                    property var contextActions: [
                        {
                            label: qsTr("View Event"),
                            icon: "\u2139",
                            action: "viewEvent",
                            section: "event",
                            data: {
                                uid: modelData.uid,
                                title: modelData.title
                            },
                            handler: function (d) { /* TODO: event detail popup */ }
                        },
                        {
                            label: qsTr("Edit Event"),
                            icon: "\u270E",
                            action: "editEvent",
                            section: "event",
                            data: {
                                uid: modelData.uid
                            },
                            handler: function (d) { /* TODO: event editor */ }
                        },
                        {
                            label: qsTr("Delete Event"),
                            icon: "\u2716",
                            action: "deleteEvent",
                            section: "event",
                            data: {
                                uid: modelData.uid
                            },
                            handler: function (d) {
                                Backend.deleteEvent(d.uid)
                            }
                        },
                    ]
                    property real evEndMs: modelData.endedMs || evStartMs
                    property real evStartMs: modelData.startedMs || Date.now()
                    required property int index
                    property bool isRange: modelData.eventType === "range" && modelData.endedMs
                    required property var modelData

                    anchors.verticalCenter: axisLine.verticalCenter
                    anchors.verticalCenterOffset: -16
                    height: 20
                    width: isRange ? Math.max(6, timeToX(evEndMs) - timeToX(evStartMs)) : 12
                    x: timeToX(evStartMs)

                    Rectangle {
                        anchors.fill: parent
                        border.color: Qt.lighter(color, 1.3)
                        border.width: eventMa.containsMouse ? 2 : 0
                        color: modelData.color || categoryColor(modelData.category)
                        opacity: eventMa.containsMouse ? 1.0 : 0.75
                        radius: isRange ? 4 : 6

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }
                    Text {
                        anchors.bottom: parent.top
                        anchors.bottomMargin: 2
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#c0c0c0"
                        elide: Text.ElideRight
                        font.pixelSize: 9
                        horizontalAlignment: Text.AlignHCenter
                        text: modelData.title || ""
                        visible: eventMa.containsMouse || zoomLevel > 2
                        width: Math.max(parent.width, 80)
                    }
                    MouseArea {
                        id: eventMa

                        acceptedButtons: Qt.LeftButton
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true

                        onClicked: { /* TODO: open event detail popup */ }
                    }
                    ToolTip {
                        delay: 400
                        text: (modelData.title || "") + "\n" + (modelData.started || "") + (isRange ? " — " + (modelData.ended || "") : "") + (modelData.location ? "\n" + modelData.location : "")
                        visible: eventMa.containsMouse
                    }
                }
            }
        }
    }
    PinchHandler {
        target: null

        onActiveScaleChanged: {
            timelineRoot.zoomLevel = Math.max(0.5, Math.min(10.0, timelineRoot.zoomLevel * activeScale))
        }
    }
}
