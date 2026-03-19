// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick

/**
 * Canvas that draws connection lines between the central persona and
 * each contact node.  Line width encodes trust level; colour encodes
 * shared group membership.
 */
Canvas {
    id: lineCanvas

    property real centerX: 0
    property real centerY: 0
    property var contactNodes: null
    property var contacts: []

    // Group colour palette for shared-group connection tinting
    property var groupColors: {
        var map = {}
        for (var i = 0; i < groups.length; i++) {
            map[groups[i].uid] = groups[i].color || randomColor(i)
        }
        return map
    }
    property var groups: []

    function randomColor(seed) {
        var hue = (seed * 137.508) % 360
        return Qt.hsla(hue / 360, 0.6, 0.5, 1)
    }

    onCenterXChanged: requestPaint()
    onCenterYChanged: requestPaint()
    onContactsChanged: requestPaint()
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        if (!contactNodes || !contactNodes.count)
            return
        for (var i = 0; i < contactNodes.count; i++) {
            var node = contactNodes.itemAt(i)
            if (!node)
                continue
            var contact = contacts[i]
            if (!contact)
                continue
            var nx = node.x + node.width / 2
            var ny = node.y + node.height / 2
            var trust = contact.trustLevel || 1;

            // Line width proportional to trust (1..6 px)
            var lineWidth = 1 + (trust / 100.0) * 5;

            // Base colour: blend between red (low trust) and green (high trust)
            var t = trust / 100.0
            var r = Math.round((1.0 - t) * 200 + 80)
            var g = Math.round(t * 200 + 80)
            var baseColor = "rgba(" + r + "," + g + ",120,0.6)"

            ctx.beginPath()
            ctx.moveTo(centerX, centerY);

            // Slight curve for visual interest
            var cpX = (centerX + nx) / 2 + (ny - centerY) * 0.1
            var cpY = (centerY + ny) / 2 - (nx - centerX) * 0.1
            ctx.quadraticCurveTo(cpX, cpY, nx, ny)

            ctx.strokeStyle = baseColor
            ctx.lineWidth = lineWidth
            ctx.stroke()
        }
    }
}
