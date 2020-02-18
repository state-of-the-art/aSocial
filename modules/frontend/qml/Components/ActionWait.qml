import QtQuick 2.4

Rectangle {
    id: root
    visible: false
    opacity: 0.0

    property point _view_pos
    property var _current_slot: null

    signal run(point view_pos)

    function start(view_pos, slot) {
        if( _current_slot === null ) {
            _view_pos = Qt.point(view_pos.x, view_pos.y)
            _current_slot = slot

            root.run.connect(_current_slot)
            root.x = view_pos.x - root.radius
            root.y = view_pos.y - root.radius
            root.visible = true

            arrow_animation.start()
        } else
            return false

        return true
    }

    function stop() {
        if( _current_slot !== null ) {
            root.run.disconnect(_current_slot)
            _current_slot = null
            arrow_animation.stop()
            root.visible = false
        } else
            return false

        return true
    }

    width: 50 * screenScale
    height: width
    radius: width/2
    border.width: 1

    Rectangle {
        id: arrow
        anchors.centerIn: parent
        width: 2 * screenScale
        height: parent.radius
        color: "#000"
        transform: Translate { y: -arrow.height/2 }
        transformOrigin: Item.Bottom

        SequentialAnimation on rotation {
            id: arrow_animation
            running: false
            RotationAnimation { from: 0; to: 360; duration: 1000 }
            ScriptAction { script: root.run(root._view_pos) }
            ScriptAction { script: root.stop() }
        }
    }

    states: [
        State {
            name: "visible"
            when: visible
            PropertyChanges { target: root; opacity: 1.0 }
        }
    ]
    transitions: Transition {
        NumberAnimation { property: "opacity"; duration: 500; from: 0.0; to: 1.0; easing.type: Easing.InQuart }
    }
}

