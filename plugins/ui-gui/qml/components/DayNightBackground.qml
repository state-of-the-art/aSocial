// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick

/**
 * Animated day/night background that shifts colours and shows celestial
 * bodies (sun / moon / stars) based on the real wall-clock time.
 *
 * The gradient transitions smoothly:
 *   06:00-08:00  sunrise (warm oranges)
 *   08:00-18:00  day     (light blue sky)
 *   18:00-20:00  sunset  (purple / pink)
 *   20:00-06:00  night   (dark navy with stars + moon)
 */
Item {
    id: bgRoot

    property color bottomColor: {
        if (hour >= 6 && hour < 8)
            return Qt.rgba(1.0, 0.85, 0.5, 1)
        if (hour >= 8 && hour < 18)
            return Qt.rgba(0.75, 0.88, 1.0, 1)
        if (hour >= 18 && hour < 20)
            return Qt.rgba(0.85, 0.45, 0.35, 1)
        return Qt.rgba(0.08, 0.08, 0.22, 1)
    }

    // Normalized progress through the day [0..1]
    property real dayProgress: hour / 24.0
    property real hour: {
        var d = new Date()
        return d.getHours() + d.getMinutes() / 60.0
    }

    // Stars (visible at night)
    property real starsOpacity: {
        if (hour >= 21 || hour < 5)
            return 1.0
        if (hour >= 20)
            return (hour - 20)
        // fade in 20-21
        if (hour < 6)
            return 1.0 - (hour - 5)
        // fade out 5-6
        return 0.0
    }

    // Sky gradient colours based on time of day
    property color topColor: {
        if (hour >= 6 && hour < 8)
            return Qt.rgba(0.95, 0.65, 0.35, 1)
        // sunrise warm
        if (hour >= 8 && hour < 18)
            return Qt.rgba(0.35, 0.55, 0.85, 1)
        // day blue
        if (hour >= 18 && hour < 20)
            return Qt.rgba(0.6, 0.3, 0.55, 1)
        // sunset purple
        return Qt.rgba(0.05, 0.05, 0.18, 1)
        // night dark
    }

    Behavior on bottomColor {
        ColorAnimation {
            duration: 3000
        }
    }
    Behavior on topColor {
        ColorAnimation {
            duration: 3000
        }
    }

    Timer {
        interval: 60000
        repeat: true
        running: true

        onTriggered: bgRoot.hour = Qt.binding(function () {
            var d = new Date()
            return d.getHours() + d.getMinutes() / 60.0
        })
    }
    Rectangle {
        anchors.fill: parent

        gradient: Gradient {
            GradientStop {
                color: bgRoot.topColor
                position: 0.0
            }
            GradientStop {
                color: bgRoot.bottomColor
                position: 1.0
            }
        }
    }
    Repeater {
        model: 60

        delegate: Rectangle {
            property real starSize: 1 + Math.random() * 3
            property real starX: Math.random()
            property real starY: Math.random()
            property real twinklePhase: Math.random() * Math.PI * 2

            color: "#ffffff"
            height: starSize
            opacity: bgRoot.starsOpacity * (0.4 + 0.6 * Math.abs(Math.sin(twinklePhase + starAnimTimer.elapsed * 0.001)))
            radius: starSize / 2
            width: starSize
            x: starX * bgRoot.width
            y: starY * bgRoot.height * 0.6

            Timer {
                id: starAnimTimer

                property real elapsed: 0

                interval: 50
                repeat: true
                running: true

                onTriggered: elapsed += interval
            }
        }
    }

    // Sun (visible during day)
    Rectangle {
        id: sun

        property real sunAngle: ((hour - 6) / 12.0) * Math.PI

        color: "#FFE066"
        height: 80
        opacity: (hour >= 7 && hour < 19) ? 1.0 : 0.0
        radius: 40
        width: 80
        x: bgRoot.width * 0.1 + (bgRoot.width * 0.8) * ((hour - 6) / 12.0)
        y: bgRoot.height * 0.5 - Math.sin(sunAngle) * bgRoot.height * 0.35

        Behavior on opacity {
            NumberAnimation {
                duration: 2000
            }
        }
        Behavior on x {
            NumberAnimation {
                duration: 60000
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: 60000
            }
        }

        // Glow effect
        Rectangle {
            anchors.centerIn: parent
            border.color: Qt.rgba(1, 0.9, 0.3, 0.25)
            border.width: 15
            color: "transparent"
            height: 120
            radius: 60
            width: 120

            SequentialAnimation on border.width {
                loops: Animation.Infinite

                NumberAnimation {
                    duration: 3000
                    easing.type: Easing.InOutSine
                    from: 15
                    to: 25
                }
                NumberAnimation {
                    duration: 3000
                    easing.type: Easing.InOutSine
                    from: 25
                    to: 15
                }
            }
        }
    }

    // Moon (visible at night)
    Rectangle {
        id: moon

        color: "#E8E8F0"
        height: 60
        opacity: (hour >= 20 || hour < 6) ? 0.9 : 0.0
        radius: 30
        width: 60
        x: bgRoot.width * 0.7
        y: bgRoot.height * 0.1

        Behavior on opacity {
            NumberAnimation {
                duration: 2000
            }
        }

        // Moon shadow (crescent effect)
        Rectangle {
            color: bgRoot.topColor
            height: 55
            radius: 27.5
            width: 55
            x: 12
            y: -5
        }
    }
}
