import QtQuick 2.4
import "Components"
import "js/account.js" as A
import "js/userinteraction.js" as U

Rectangle {
    id: account
    anchors.fill: parent
    clip: true
    color: "#ff777777"

    property Profile _master_profile

    property alias move_to: move_to
    property alias profile_component: profile_component

    function show(account_id) {
        A.initDB(account_id)

        var main_profile = A.getProfile(1)

        // Create main profile, if it doesn't exists
        if( main_profile.address === undefined ) {
            main_profile = A.emptyProfileData()
            main_profile.address = A.createAddress()
            main_profile.id = A.createProfile(main_profile)
        }

        account.visible = true
        _master_profile = A.createProfileObj(main_profile)
        _master_profile.master()

        updateSheet()

        line_of_life.setProfile(_master_profile)
    }
    function hide() {
        account.visible = false
    }

    function updateSheet() {
        console.log("Update sheet")

        visible_area.returnToBounds()
    }

    Component.onCompleted: {
        A.init(app, account, sheet, visible_area, show_event)
    }

    Flickable {
        id: visible_area
        anchors.fill: parent
        pressDelay: 300

        clip: true

        contentWidth: sheet.scaledWidth
        contentHeight: sheet.scaledHeight

        leftMargin: contentWidth > width ? - sheet.scaledX : width/2 - (sheet.scaledX + sheet.scaledWidth/2)
        rightMargin: - leftMargin
        topMargin: contentHeight > height ? - sheet.scaledY : height/2 - (sheet.scaledY + sheet.scaledHeight/2)
        bottomMargin: - topMargin

        MouseArea {
            id: mouse_area

            parent: visible_area
            anchors.fill: parent
            z: -1

            preventStealing: true

            onPressed: {
                console.log("Pressed Account")

                var point = A.convertViewPointToSheetPoint(mouse)
                drag.target = U.actionMenuShow(mouse, [
                                     { name: qsTr("New Profile"), color: "#aff", action: A.createNewProfileObj, property: point },
                                     { name: qsTr("Zoom +"), color: "#faf", action: function(p){ A.sheetScaleTo(+1, p) }, property: point },
                                     { name: qsTr("Zoom -"), color: "#ffa", action: function(p){ A.sheetScaleTo(-1, p) }, property: point }
                                 ])
            }
            onReleased: {
                console.log("Released Account")
                U.actionMenuHide()
                drag.target = null
            }

            onWheel: {
                console.log("Wheel Account")
                A.sheetScaleTo(wheel.angleDelta.y, A.convertViewPointToSheetPoint(wheel))
            }
        }

        Item {
            id: sheet

            property var _available_scales: [0.0625, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0]
            property int _target_scale: 4

            function targetScale(index) {
                _target_scale = index in _available_scales ? index : _target_scale
                sheet.scale = _available_scales[_target_scale]
                return _available_scales[_target_scale]
            }

            Behavior on scale {
                NumberAnimation { duration: 1000; easing.type: Easing.OutExpo }
            }

            property int scaledWidth: childrenRect.width * scale
            property int scaledHeight: childrenRect.height * scale
            property int scaledX: childrenRect.x * scale
            property int scaledY: childrenRect.y * scale
        }
    }

    SequentialAnimation {
        id: move_to
        property point target_point
        property int duration_x
        property int duration_y

        ParallelAnimation {
            PropertyAnimation { target: visible_area; property: "contentX"; to: move_to.target_point.x; duration: move_to.duration_x; easing.type: Easing.OutExpo }
            PropertyAnimation { target: visible_area; property: "contentY"; to: move_to.target_point.y; duration: move_to.duration_y; easing.type: Easing.OutExpo }
        }
        ScriptAction { script: visible_area.returnToBounds() }
    }

    ScrollBar {
        id: horizontal_scrollbar
        width: visible_area.width - 12 * screenScale; height: 12 * screenScale
        anchors.bottom: parent.bottom
        opacity: 0.2
        visible: sheet.scaledWidth > visible_area.width
        orientation: Qt.Horizontal
        position: visible_area.visibleArea.xPosition
        pageSize: visible_area.width / sheet.scaledWidth

        states: State {
            name: "visible"
            when: visible_area.moving
            PropertyChanges { target: horizontal_scrollbar; opacity: 1.0 }
        }
        transitions: Transition {
            NumberAnimation { property: "opacity"; duration: 1000; easing.type: Easing.OutCubic }
        }

        DropArea {
            anchors.fill: parent

            onEntered: console.log("entered");
            onExited: console.log("exited");
            onDropped: console.log("dropped");
        }
    }

    ScrollBar {
        id: vertical_scrollbar
        width: 12 * screenScale; height: visible_area.height - 12 * screenScale
        anchors.right: parent.right
        opacity: 0.2
        visible: sheet.scaledHeight > visible_area.height
        orientation: Qt.Vertical
        position: visible_area.visibleArea.yPosition
        pageSize: visible_area.height / sheet.scaledHeight

        states: State {
            name: "visible"
            when: visible_area.moving
            PropertyChanges { target: vertical_scrollbar; opacity: 1.0 }
        }
        transitions: Transition {
            NumberAnimation { property: "opacity"; duration: 1000; easing.type: Easing.OutCubic }
        }
    }

    LineOfLife {
        id: line_of_life
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: 50 * screenScale
            rightMargin: 50 * screenScale
            bottomMargin: 20 * screenScale
        }
        height: parent.height / 10
    }

    ShowEvent {
        id: show_event
    }

    Component {
        id: profile_component
        Profile {}
    }
}
