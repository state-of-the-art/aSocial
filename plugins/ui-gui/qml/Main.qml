// Copyright (C) 2026  aSocial Developers
// SPDX-License-Identifier: GPL-3.0-or-later

// Author: Rabit (@rabits)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "components"

/**
 * Root application window. Manages global state transitions:
 *   1. Init screen  – no VFS container exists
 *   2. Login screen – container exists but no profile open
 *   3. Main screen  – profile is open, show persona graph + timeline
 *
 * Touch-ready: all hit areas >= 48 dp; PinchHandler on the graph view.
 *
 * The RadialMenu is context-aware: each visual element in the scene
 * declares a `contextActions` property.  When the user right-clicks or
 * long-presses, the menu hit-tests the item tree at that point and
 * collects actions from every ancestor (topmost = most specific,
 * bottom = background).
 */
ApplicationWindow {
    id: root

    // State machine: which screen to show
    property string appState: {
        if (!Backend.containerInitialized)
            return "init"
        if (!Backend.profileOpen)
            return "login"
        return "main"
    }

    function updateScreenForState() {
        switch (appState) {
        case "init":
            screenStack.replace(null, initComponent)
            break
        case "login":
            screenStack.replace(null, loginComponent)
            break
        case "main":
            screenStack.replace(null, mainComponent)
            break
        }
    }

    color: "transparent"
    height: 800
    minimumHeight: 640
    minimumWidth: 360
    title: qsTr("aSocial – Distributed Private Social Network")
    visible: true
    width: 1280

    Component.onCompleted: updateScreenForState()

    // React to state changes
    onAppStateChanged: updateScreenForState()

    // Animated day/night background covers the whole window
    DayNightBackground {
        anchors.fill: parent
        z: -1
    }

    // Toast / notification overlay
    ToastOverlay {
        id: toastOverlay

        anchors.fill: parent
        z: 1000
    }
    Connections {
        function onErrorOccurred(message) {
            toastOverlay.showError(message)
        }
        function onInfoMessage(message) {
            toastOverlay.showInfo(message)
        }

        target: Backend
    }

    // Hamburger menu (slide from left)
    HamburgerMenu {
        id: hamburgerMenu

        z: 100
    }

    // Radial context menu (appears on right-click / long-press)
    RadialMenu {
        id: radialMenu

        z: 99
        // sceneRoot is wired when the main screen becomes active (see below)
    }

    // Menu toggle button (always visible, top-left corner)
    RoundButton {
        id: menuButton

        anchors.left: parent.left
        anchors.margins: 12
        anchors.top: parent.top
        //icon.source: ""
        flat: true
        height: 34
        width: 34
        z: 50

        background: Rectangle {
            color: menuButton.hovered ? Qt.rgba(1, 1, 1, 0.15) : "transparent"
            radius: 24
        }
        contentItem: Column {
            anchors.centerIn: parent
            spacing: 4
            topPadding: 2

            Repeater {
                model: 3

                Rectangle {
                    color: "#e0e0e0"
                    height: 3
                    radius: 1.5
                    width: 22
                }
            }
        }

        onClicked: hamburgerMenu.toggle()

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }

    // Screen stack
    StackView {
        id: screenStack

        anchors.fill: parent
        initialItem: loadingComponent

        Component {
            id: loadingComponent

            LoadingScreen {
            }
        }
        Component {
            id: initComponent

            InitScreen {
            }
        }
        Component {
            id: loginComponent

            LoginScreen {
            }
        }
        Component {
            id: mainComponent

            MainScreen {
            }
        }
    }

    // When the StackView finishes a transition, wire the RadialMenu's
    // sceneRoot to the full current screen so hit-testing includes all
    // content (graph canvas, TimelineBar, etc.).
    Connections {
        function onCurrentItemChanged() {
            radialMenu.sceneRoot = screenStack.currentItem || null
        }

        target: screenStack
    }

    // Right-click: open radial menu at the pointer position.
    // The menu itself discovers which contextActions are available
    // by hit-testing the item tree under that point.
    TapHandler {
        acceptedButtons: Qt.RightButton
        gesturePolicy: TapHandler.ReleaseWithinBounds

        onTapped: function (eventPoint) {
            if (radialMenu.sceneRoot)
                radialMenu.showAtPoint(eventPoint.position.x, eventPoint.position.y)
        }
    }

    // Long-press (touch): same behaviour as right-click
    TapHandler {
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.ReleaseWithinBounds

        onLongPressed: function () {
            if (radialMenu.sceneRoot) {
                var pos = point.position
                radialMenu.showAtPoint(pos.x, pos.y)
            }
        }
    }
}
