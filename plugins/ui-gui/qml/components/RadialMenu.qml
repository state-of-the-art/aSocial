// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick

/**
 * Context-aware radial (pie) menu.
 *
 * Any Item in the scene tree may declare:
 *
 *     property var contextActions: [
 *         { label: "Edit", icon: "\u270E", action: "editContact",
 *           section: "contact", data: { uid: "..." },
 *           handler: function(data) { ... } }
 *     ]
 *
 * When the menu is invoked at a scene point, collectActions() walks
 * the item tree from the deepest hit child upward through every
 * ancestor, gathering contextActions from each layer.  The result
 * is a flat, de-duplicated list ordered top-to-bottom (most specific
 * element first, background last).
 *
 * Desktop:  right-click opens the menu; left-click picks an action.
 * Android:  long-press opens the menu; short-tap picks an action.
 * Long-press on a wedge sets it as the "default left-click action"
 * for that section (stored in profile parameters).
 */
Item {
    id: radialRoot

    /** @brief Flat list of collected actions currently displayed (raw data). */
    property var actions: []
    property int activeIndexOnPress: -1
    property real centerX: 0
    property real centerY: 0
    property int hoveredIndex: -1
    /** @brief Layouted actions with angle/ring information for rendering. */
    property var layoutActions: []
    property real menuRadius: 120
    property bool persistent: false
    property bool pressToActivate: false

    /**
     * @brief Root item whose subtree is searched for contextActions.
     * Set this to the content area that should participate in
     * hit-testing (usually the main screen's graph canvas).
     */
    property Item sceneRoot: null

    // ---- hit-test & collection -------------------------------------------

    /**
     * @brief Recursively find the deepest visible Item at (x, y)
     * inside `root`, then walk up the parent chain collecting every
     * contextActions array encountered.
     *
     * Returns a flat, de-duplicated list ordered from the most
     * specific (topmost visual) to the most general (background).
     */
    function collectActions(sx, sy) {
        var stack = []
        var seen = {};

        // Find the deepest hit item
        var deepest = deepestChildAt(sceneRoot, sx, sy);

        // Walk from deepest hit up to (and including) sceneRoot
        var item = deepest
        while (item) {
            if (Array.isArray(item.contextActions) && item.contextActions.length > 0) {
                for (var i = 0; i < item.contextActions.length; i++) {
                    var a = item.contextActions[i]
                    var key = a.action || a.label
                    if (!seen[key]) {
                        seen[key] = true
                        stack.push(a)
                    }
                }
            }
            if (item === sceneRoot)
                break
            item = item.parent
        }

        return stack
    }

    /**
     * @brief Compute a visible angular sector and distribute actions across
     * one or more concentric rings so that all wedges stay fully on-screen.
     */
    function computeLayout() {
        var result = []
        var raw = actions || []
        if (raw.length === 0) {
            layoutActions = []
            return
        }

        var R = menuRadius
        var margin = 8
        var w = radialRoot.width
        var h = radialRoot.height

        // Detect proximity to edges to decide which sector(s) are safe
        var nearTop = (centerY - R - margin) < 0
        var nearBottom = (centerY + R + margin) > h
        var nearLeft = (centerX - R - margin) < 0
        var nearRight = (centerX + R + margin) > w

        var startAngle = -Math.PI
        // default: full circle
        var endAngle = Math.PI

        // Prefer specific corner/edge cases
        if (nearBottom && !nearTop && !nearLeft && !nearRight) {
            // Use upper semicircle (0–180d) around the point
            startAngle = -Math.PI
            endAngle = 0
        } else if (nearTop && !nearBottom && !nearLeft && !nearRight) {
            // Use lower semicircle (0–180d below)
            startAngle = 0
            endAngle = Math.PI
        } else if (nearRight && nearBottom) {
            // Bottom-right corner: use upper-left quadrant (0–90d)
            startAngle = -Math.PI
            endAngle = -Math.PI / 2
        } else if (nearLeft && nearBottom) {
            // Bottom-left corner: use upper-right quadrant (90–180°)
            startAngle = -Math.PI / 2
            endAngle = 0
        } else if (nearRight && nearTop) {
            // Top-right corner: use lower-left quadrant (270–360°)
            startAngle = Math.PI / 2
            endAngle = Math.PI
        } else if (nearLeft && nearTop) {
            // Top-left corner: use lower-right quadrant (180–270°)
            startAngle = 0
            endAngle = Math.PI / 2
        } else if (nearLeft && !nearRight) {
            // Left side: prefer right semicircle
            startAngle = -Math.PI / 2
            endAngle = Math.PI / 2
        } else if (nearRight && !nearLeft) {
            // Right side: prefer left semicircle
            startAngle = Math.PI / 2
            endAngle = (3 * Math.PI) / 2
        }

        var sectorSpan = endAngle - startAngle
        if (sectorSpan <= 0)
            sectorSpan = 2 * Math.PI;

        // Wedge size: 64x64, so centers need ~32px clearance to avoid overlap
        var wedgeRadius = 32
        var ringSpacing = 64
        var total = raw.length

        // Compute how many rings we need: each ring has a max capacity
        // based on angular spacing (wedgeRadius/ringRadius)
        var ringCount = 1
        var assigned = 0
        while (assigned < total) {
            var radius = R + (ringCount - 1) * ringSpacing
            var minAngle = 2 * Math.asin(wedgeRadius / Math.max(radius, 40))
            var maxPerRing = Math.max(2, Math.floor(sectorSpan / minAngle))
            assigned += maxPerRing
            if (assigned < total)
                ringCount++
        }

        assigned = 0
        for (var ring = 0; ring < ringCount && assigned < total; ++ring) {
            var radius = R + ring * ringSpacing
            var minAngle = 2 * Math.asin(wedgeRadius / Math.max(radius, 40))
            var maxPerRing = Math.max(2, Math.floor(sectorSpan / minAngle))
            var remaining = total - assigned
            var countThisRing = Math.min(remaining, maxPerRing);

            // center wedges within the sector; leave small gaps at ends
            var step = sectorSpan / (countThisRing + 1)

            for (var i = 0; i < countThisRing; ++i) {
                var actionIndex = assigned + i
                var a = raw[actionIndex]
                var angle = startAngle + (i + 1) * step
                result.push({
                    action: a,
                    angle: angle,
                    ringRadius: radius,
                    index: actionIndex
                })
            }
            assigned += countThisRing
        }

        layoutActions = result
    }

    /**
     * @brief Recursively find the deepest (front-most) visible child
     * at position (px, py) in scene coordinates mapped to `item`.
     */
    function deepestChildAt(item, px, py) {
        if (!item || !item.visible)
            return null

        var local = item.mapFromItem(null, px, py)

        if (local.x < 0 || local.y < 0 || local.x > item.width || local.y > item.height)
            return null

        // Iterate children back-to-front (highest z first)
        for (var i = item.children.length - 1; i >= 0; i--) {
            var child = item.children[i]
            var hit = deepestChildAt(child, px, py)
            if (hit)
                return hit
        }

        return item
    }

    // ---- action execution ------------------------------------------------

    function executeAction(actionObj) {
        hide()
        if (typeof actionObj.handler === "function") {
            actionObj.handler(actionObj.data || {})
        } else {
            console.log("RadialMenu: no handler for", actionObj.action, "– data:", JSON.stringify(actionObj.data || {}))
        }
    }
    function hide() {
        hideAnim.restart()
    }

    // ---- helpers ----------------------------------------------------------

    function sectionToColor(section) {
        switch (section) {
        case "persona":
            return Qt.rgba(0.3, 0.5, 0.8, 1)
        case "contact":
            return Qt.rgba(0.3, 0.7, 0.5, 1)
        case "group":
            return Qt.rgba(0.7, 0.5, 0.3, 1)
        case "event":
            return Qt.rgba(0.6, 0.3, 0.7, 1)
        case "message":
            return Qt.rgba(0.2, 0.6, 0.8, 1)
        case "background":
            return Qt.rgba(0.4, 0.4, 0.4, 1)
        case "timeline":
            return Qt.rgba(0.5, 0.5, 0.7, 1)
        default:
            return Qt.rgba(0.3, 0.3, 0.4, 1)
        }
    }

    // ---- public API -------------------------------------------------------

    /**
     * @brief Open the radial menu at scene coordinates (sx, sy).
     *
     * Hit-tests sceneRoot's subtree at the given point, walks the
     * parent chain collecting contextActions[], de-duplicates by
     * action id, and displays the merged list.
     */
    function showAtPoint(sx, sy) {
        centerX = sx
        centerY = sy
        persistent = true
        pressToActivate = false
        hoveredIndex = -1
        actions = collectActions(sx, sy)
        if (actions.length === 0)
            return
        computeLayout()
        visible = true
        showAnim.restart()
    }

    /**
     * @brief Legacy helper – show with an explicit action list.
     */
    function showWithActions(sx, sy, actionList) {
        centerX = sx
        centerY = sy
        persistent = true
        pressToActivate = false
        hoveredIndex = -1
        actions = actionList
        if (actions.length === 0)
            return
        computeLayout()
        visible = true
        showAnim.restart()
    }

    anchors.fill: parent
    visible: false
    z: 99

    // ---- visuals ----------------------------------------------------------

    // Dim background
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.25)
        opacity: visible ? 1 : 0

        MouseArea {
            function wedgeIndexAtScene(sx, sy) {
                var local = radialContainer.mapFromItem(null, sx, sy)
                for (var i = radialContainer.children.length - 1; i >= 0; --i) {
                    var c = radialContainer.children[i]
                    if (!c.visible || typeof c.index === "undefined")
                        continue
                    var lp = c.mapFromItem(radialContainer, local.x, local.y)
                    if (lp.x >= 0 && lp.y >= 0 && lp.x <= c.width && lp.y <= c.height)
                        return c.index
                }
                return -1
            }

            anchors.fill: parent
            hoverEnabled: true

            onClicked: function (mouse) {
                if (mouse.button === Qt.LeftButton && !pressToActivate) {
                    radialRoot.hide()
                }
            }
            onPressed: function (mouse) {
                // right-button press starts press-to-activate mode
                if (mouse.button === Qt.RightButton) {
                    pressToActivate = true
                    // determine which wedge is under the cursor, if any
                    var idx = wedgeIndexAtScene(mouse.x, mouse.y)
                    activeIndexOnPress = idx
                    hoveredIndex = idx
                }
            }
            onReleased: function (mouse) {
                if (mouse.button === Qt.RightButton && pressToActivate) {
                    // if the user moved to another wedge, use hoveredIndex;
                    // otherwise fall back to the one under the initial press
                    var idx = hoveredIndex >= 0 ? hoveredIndex : activeIndexOnPress
                    if (idx >= 0 && idx < layoutActions.length) {
                        executeAction(layoutActions[idx].action)
                    } else if (!persistent) {
                        radialRoot.hide()
                    }
                    pressToActivate = false
                    activeIndexOnPress = -1
                }
            }
        }
    }

    // Radial container
    Item {
        id: radialContainer

        height: menuRadius * 2
        width: menuRadius * 2
        x: centerX - menuRadius
        y: centerY - menuRadius

        // Center indicator
        Rectangle {
            anchors.centerIn: parent
            border.color: Qt.rgba(1, 1, 1, 0.2)
            border.width: 1
            color: Qt.rgba(0.15, 0.15, 0.2, 0.9)
            height: 40
            radius: 20
            width: 40
        }

        // Wedges
        Repeater {
            model: layoutActions

            delegate: Item {
                id: wedgeItem

                // angle and ringRadius are precomputed in layoutActions
                property real angle: modelData.angle
                required property int index
                property bool isHovered: radialRoot.hoveredIndex === index
                property real itemX: Math.cos(angle) * ringRadius
                property real itemY: Math.sin(angle) * ringRadius
                required property var modelData
                property real ringRadius: modelData.ringRadius

                // Section tint: colour the ring slightly per section
                property color sectionColor: sectionToColor(modelData.action.section || "")
                property int totalActions: radialRoot.actions.length

                height: 64
                width: 64
                x: radialContainer.width / 2 + itemX - 32
                y: radialContainer.height / 2 + itemY - 32

                Rectangle {
                    anchors.fill: parent
                    border.color: isHovered ? "#4da6ff" : Qt.rgba(1, 1, 1, 0.15)
                    border.width: isHovered ? 2 : 1
                    color: isHovered ? Qt.lighter(sectionColor, 1.4) : Qt.rgba(sectionColor.r * 0.4 + 0.1, sectionColor.g * 0.4 + 0.1, sectionColor.b * 0.4 + 0.1, 0.85)
                    radius: 32
                    scale: isHovered ? 1.15 : 1.0

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                    Behavior on scale {
                        NumberAnimation {
                            duration: 150
                            easing.type: Easing.OutBack
                        }
                    }
                }

                // Abbreviated icon
                Text {
                    anchors.centerIn: parent
                    color: isHovered ? "#ffffff" : "#c0c0c0"
                    font.bold: true
                    font.pixelSize: 32
                    text: modelData.action.icon || modelData.action.label
                }

                // Full label (visible on hover)
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.bottom
                    anchors.topMargin: 4
                    color: isHovered ? "#ffffff" : "#909090"
                    font.pixelSize: 10
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData.action.label || ""
                    visible: isHovered
                }

                // Section tag (tiny label on the outer edge)
                Text {
                    anchors.bottom: parent.top
                    anchors.bottomMargin: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: Qt.rgba(1, 1, 1, 0.35)
                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 7
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData.action.section || ""
                    visible: (modelData.action.section || "").length > 0
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: radialRoot.executeAction(modelData.action)
                    onEntered: radialRoot.hoveredIndex = index
                    onExited: if (radialRoot.hoveredIndex === index)
                        radialRoot.hoveredIndex = -1
                    onPressAndHold: {
                        // Save as default left-click action for this section
                        var section = modelData.action.section || ""
                        if (section.length > 0 && Backend.profileOpen) {
                            Backend.setParam("ui.defaultAction." + section, modelData.action.action || "")
                        }
                    }
                }
            }
        }
    }

    // ---- animations -------------------------------------------------------

    ParallelAnimation {
        id: showAnim

        NumberAnimation {
            duration: 250
            easing.type: Easing.OutBack
            from: 0.3
            property: "scale"
            target: radialContainer
            to: 1.0
        }
        NumberAnimation {
            duration: 200
            from: 0
            property: "opacity"
            target: radialContainer
            to: 1
        }
    }
    SequentialAnimation {
        id: hideAnim

        ParallelAnimation {
            NumberAnimation {
                duration: 150
                easing.type: Easing.InBack
                property: "scale"
                target: radialContainer
                to: 0.3
            }
            NumberAnimation {
                duration: 150
                property: "opacity"
                target: radialContainer
                to: 0
            }
        }
        ScriptAction {
            script: radialRoot.visible = false
        }
    }
}
