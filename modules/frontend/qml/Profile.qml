import QtQuick 2.4
import "Components"
import "js/account.js" as A
import "js/profile.js" as P
import "js/userinteraction.js" as U

Item {
    id: root
    objectName: "Profile"

    width: 200
    height: 200

    scale: 0.01

    property bool is_master: false
    property bool is_edit: false

    property var obj_data

    function createConnection(profile_id, pos) {
        console.log("Create connection for #" + obj_data.id)
        if( ! obj_data.data.connections )
            obj_data.data.connections = []

        var connection_key = obj_data.data.connections.push({"profile_id": profile_id, "pos": pos}) - 1
        A.updateProfileData(obj_data)

        return processConnection(obj_data.data.connections[connection_key])
    }

    function processConnection(connection) {
        var profile_data = A.getProfile(connection.profile_id)
        profile_data.pos = connection.pos
        return A.createProfileObj(profile_data)
    }

    function master() {
        console.log("Set master profile")
        root.is_master = true

        root.x = 0-300/2
        root.y = 0-400/2

        if( obj_data.data.connections ) {
            for( var c in obj_data.data.connections )
                root.processConnection(obj_data.data.connections[c])
        }
    }

    function saveObjData() {
        P.updateObjData()
        A.updateProfileData(obj_data)
    }

    function editSwitch() {
        if( root.is_edit )
            root.saveObjData()

        root.is_edit = ! root.is_edit
    }

    function createNewMessage() {
        console.log("New Message for: " + obj_data.id)
        A.showEvent({ occur: wdate.currentUnixtime(),
                      link: null,
                      type: "message",
                      owner: A.masterProfile().obj_data.id,
                      recipient: obj_data.id,
                      data: {text: ""},
                      overlay: {}
                    }, A.saveEvent)
    }

    Component.onCompleted: {
        // Set position of profile
        if( obj_data.pos ) {
            root.x = obj_data.pos.x - root.width/2
            root.y = obj_data.pos.y - root.height/2
        }
        born_animation.start()
    }

    SequentialAnimation {
        id: born_animation
        PropertyAnimation { target: root; property: "scale"; to: 1.0; duration: 500; easing.type: Easing.OutBack }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        antialiasing: true
        smooth: true

        border.width: 1
        border.color: (obj_data.address === "") ? "#fff" : "#000"

        radius: background.width/2
        color: (obj_data.address === "") ? "#33ffffff" : "#dcffffff"

        Rectangle {
            id: background_highlight
            width: parent.width
            height: parent.height
            visible: false
            opacity: 0.2

            radius: parent.radius

            color: "#0f0"
        }
    }

    MouseArea {
        id: mouse_area
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true

        onClicked: {
            console.log("Clicked Profile #" + obj_data.id)
            A.sheetItemTop(root)
            line_of_life.setProfile(root)
        }

        onPressed: {
            console.log("PrePressed Profile #" + obj_data.id)
            if( background.radius > 0 ) {
                // Background is not squared - so, let's check that point in radius
                if( Math.pow(mouse.x - background.radius, 2) + Math.pow(mouse.y - background.radius, 2) > Math.pow(background.radius, 2) ) {
                    mouse.accepted = false
                    return
                }
            }

            console.log("Pressed Profile #" + obj_data.id)
            console.log(JSON.stringify(obj_data))

            var point = Qt.point(mouse.x + root.x, mouse.y + root.y)
            var actions = [
                        { name: root.is_edit ? qsTr("Save") : qsTr("Edit"), color: "#faa", action: root.editSwitch },
                        { name: qsTr("New Message"), color: "#aaa", action: root.createNewMessage },
                        { name: qsTr("Zoom +"), color: "#faf", action: function(p){ A.sheetScaleTo(+1, p) }, property: point },
                        { name: qsTr("Zoom -"), color: "#ffa", action: function(p){ A.sheetScaleTo(-1, p) }, property: point }
                    ]
            if( ! root.is_master )
                actions.unshift({ name: qsTr("Move"), color: "#aff", action: function(){ console.log("TODO move profile action") } })

            drag.target = U.actionMenuShow(A.convertSheetPointToViewPoint(point), actions)

            background_highlight.visible = true
        }

        onReleased: {
            console.log("Released Profile #" + obj_data.id)

            U.actionMenuHide()
            background_highlight.visible = false

            // TODO: Save profile position
            /*if( root.state === "" || root.state === "edit_" ) {
                root.x += background_highlight.x
                root.y += background_highlight.y
                background_highlight.x = background_highlight.y = 0
                obj_data.pos.x = root.x + root.width/2
                obj_data.pos.y = root.y + root.height/2
                A.masterProfile().saveObjData()
            }*/
        }

        onPositionChanged: {
            // TODO: Drag profile
            /*if( mouse.buttons & Qt.LeftButton == 1 ) {
                if( root.state === "" || root.state === "edit_" ) {
                    background_highlight.x = mouse.x - _grab_point.x
                    background_highlight.y = mouse.y - _grab_point.y
                }
            }*/
        }

        onWheel: {
            console.log("Wheel Profile #" + obj_data.id)
            A.sheetScaleTo(wheel.angleDelta.y, Qt.point(root.x + wheel.x, root.y + wheel.y))
        }

        Rectangle {
            id: avatar_background
            antialiasing: true
            smooth: true
            anchors {
                top: parent.top
                topMargin: 50
                horizontalCenter: parent.horizontalCenter
            }

            width: 100
            height: 100
            border.width: 1

            color: "#ccc"

            Image {
                id: data_avatar_url
                anchors.fill: parent

                fillMode: Image.PreserveAspectFit

                source: obj_data.data.avatar_url_eq
            }

            Text {
                anchors.fill: parent
                anchors.margins: 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                fontSizeMode: Text.Fit
                minimumPixelSize: 10
                font.pixelSize: 72
                font.weight: Font.Bold
                color: "#22000000"

                visible: data_avatar_url.source == ""

                text: qsTr("No Avatar")
            }
        }

        EditableInput {
            id: data_name
            anchors {
                bottom: parent.bottom
                bottomMargin: 50
                horizontalCenter: parent.horizontalCenter
            }
            maxWidth: root.width
            horizontalAlignment: Text.AlignHCenter

            font.weight: Font.Bold
            font.pixelSize: 16
            style: Text.Outline
            styleColor: "#fff"

            default_text: qsTr("Name")
            enabled: false
            next_item: data_birth_date

            text: obj_data.data.name
        }

        EditableInput {
            id: data_birth_date
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: avatar_background.bottom

            default_text: qsTr("Birth Date")
            enabled: false
            next_item: data_death_date

            inputMask: "99/99/9999"

            text: obj_data.data.birth_date !== null  ? wdate.format(obj_data.data.birth_date*1000, "dd/MM/yyyy") : ""
        }

        EditableInput {
            id: data_death_date
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: data_birth_date.bottom

            default_text: qsTr("Death Date")
            enabled: false
            next_item: data_name

            inputMask: "99/99/9999"

            text: obj_data.data.death_date !== null ? wdate.format(obj_data.data.death_date*1000, "dd/MM/yyyy") : ""
        }
    }

    states: [
        State {
            name: "master"
            when: root.is_master && (! root.is_edit)
            PropertyChanges { target: background; radius: 0 }
            PropertyChanges { target: root; width: 300; height: 400 }
            PropertyChanges { target: avatar_background; width: 200; height: 300; anchors.topMargin: 20 }
            PropertyChanges { target: data_avatar_url; source: obj_data.data.avatar_url }
            PropertyChanges { target: data_name; font.pixelSize: 20 }
            PropertyChanges { target: data_birth_date; anchors.topMargin: 40; visible: false }
        },
        State {
            name: "edit_"
            when: (! root.is_master) && root.is_edit
            PropertyChanges { target: data_name; enabled: true; editable: true }
            PropertyChanges { target: data_birth_date; enabled: true; editable: true }
            PropertyChanges { target: data_death_date; enabled: true; editable: true }
            PropertyChanges { target: background; color: "#0f0" }
        },
        State {
            name: "edit_master"
            extend: "master"
            when: root.is_master && root.is_edit
            PropertyChanges { target: data_name; enabled: true; editable: true }
            PropertyChanges { target: data_birth_date; enabled: true; editable: true; visible: true }
            PropertyChanges { target: data_death_date; enabled: true; editable: true; visible: true }
            PropertyChanges { target: background; color: "#0f0" }
        }
    ]
    transitions: [
        Transition {
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { property: 'radius'; duration: 500; easing.type: Easing.OutCubic }
                    NumberAnimation { properties: 'width,height'; duration: 500; easing.type: Easing.OutCubic }
                }
            }
        }
    ]
}

