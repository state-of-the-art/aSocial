import QtQuick 2.0
import "../js/userinteraction.js" as U

Rectangle {
    id: root
    visible: false
    opacity: 0.0

    width: 0
    height: width
    color: "#44ffffff"

    property string defaultText: qsTr("Do nothing")
    property int actions_width: 50 * screenScale
    property int actions_radius: actions_width / 2

    property int _circle_radius: 0
    property var _actions: []

    property var _sectors: 0

    function show(view_pos, actions) {
        _actions = actions
        _circle_radius = Math.max(Math.ceil(25/Math.sin(Math.PI/_actions.length)), actions_width)

        //  Testing visibility of circle extremum points
        var winsize = U.windowSize()
        var parts = [null, null, null, null]
        _sectors = false
        var points = []
        var parts_count = 0
        // Left
        if( view_pos.x - _circle_radius - actions_radius > 0 ) {
            parts[0] = Qt.point(-1, 0)
            _sectors += 1 // 0001
            parts_count++
        }
        // Top
        if( view_pos.y - _circle_radius - actions_radius > 0 ) {
            parts[1] = Qt.point(0, -1)
            _sectors += 2 // 0010
            parts_count++
        }
        // Right
        if( view_pos.x + _circle_radius + actions_radius < winsize.width ) {
            parts[2] = Qt.point(1, 0)
            _sectors += 4 // 0100
            parts_count++
        }
        // Bottom
        if( view_pos.y + _circle_radius + actions_radius < winsize.height ) {
            parts[3] = Qt.point(0, 1)
            _sectors += 8 // 1000
            parts_count++
        }

        var ind = parts.indexOf(null)
        if( ind > -1 && parts_count > 1 ) {
            // Recalculate circle radius due to reduction of available space on the path
            _circle_radius = Math.max(Math.ceil(25/Math.sin(Math.PI/(_actions.length*(4/(parts_count-1))))), actions_width)

            var i = ind
            do {
                i = (i+1)%4
                if( parts[i] !== null ) {
                    parts[i].x *= _circle_radius
                    parts[i].y *= _circle_radius
                    points.push(parts[i])
                }
            } while( i !== ind )
        }
        pathview.setPathPoints(points)

        root.width = _circle_radius * 2 + actions_width
        root.radius = root.width / 2
        root.x = view_pos.x - root.radius
        root.y = view_pos.y - root.radius

        dragitem.x = root.radius
        dragitem.y = root.radius

        action_name.text = root.defaultText

        root.visible = true
        root.focus = true

        pathview.model = _actions.length

        if( parts_count > 1 )
            pathview.offset = 0.5

        dragitem.dragActive = true
        return dragitem
    }

    function hide() {
        dragitem.dragActive = false
        root.visible = false
        pathview.model = 0
    }

    PathView {
        id: pathview
        model: 0
        interactive: false

        function setPathPoints(points) {
            if( points.length === 0 )
                pathview.path = path_circle
            else {
                path_arc.useLargeArc = points.length > 3
                path_arc.startPoint = points[0]
                path_arc.endPoint = points[points.length-1]
                pathview.path = path_arc
            }
        }

        delegate: Rectangle {
            width: actions_width
            height: width
            radius: actions_radius
            border.width: 1

            property var _a_data: _actions[index]

            color: _a_data.color

            function choose() {
                color = "#000"
                action_name.text = _a_data.name
            }

            function cleanChosen() {
                color = _a_data.color
                action_name.text = root.defaultText
            }

            DropArea {
                anchors.fill: parent

                onPositionChanged: {
                    if( parent.radius > 0 ) {
                        // Delegate is not squared - so, let's check that point in radius
                        if( Math.pow(drag.x - parent.radius, 2) + Math.pow(drag.y - parent.radius, 2) > Math.pow(parent.radius, 2) ) {
                            drag.accepted = false
                            cleanChosen()
                            return
                        }
                    }
                    drag.accepted = true
                    choose()
                }

                onExited: {
                    cleanChosen()
                }

                onDropped: {
                    if( parent.radius > 0 ) {
                        // Delegate is not squared - so, let's check that point in radius
                        if( Math.pow(drag.x - parent.radius, 2) + Math.pow(drag.y - parent.radius, 2) > Math.pow(parent.radius, 2) ) {
                            drop.accepted = false
                            return
                        }
                    }
                    drop.accept()
                    _a_data.action(_a_data.property)
                    cleanChosen()
                }
            }
        }

        path: path_circle

        Path {
            id: path_circle
            startX: root.radius
            startY: root.radius - _circle_radius

            PathArc {
                x: root.radius
                y: root.radius + _circle_radius
                radiusX: _circle_radius
                radiusY: _circle_radius
            }
            PathArc {
                x: root.radius
                y: root.radius - _circle_radius
                radiusX: _circle_radius
                radiusY: _circle_radius
            }
        }
        Path {
            id: path_arc

            property point startPoint
            property point endPoint
            property bool useLargeArc: false

            startX: root.radius + startPoint.x
            startY: root.radius + startPoint.y

            PathArc {
                x: root.radius + path_arc.endPoint.x
                y: root.radius + path_arc.endPoint.y
                radiusX: _circle_radius
                radiusY: _circle_radius
                useLargeArc: path_arc.useLargeArc
            }
        }
    }

    Item {
        id: dragitem
        visible: false

        property bool dragActive
        onDragActiveChanged: {
            if( dragActive ) {
                Drag.start();
            } else {
                Drag.drop();
            }
        }
    }

    Rectangle {
        id: action_name_bg
        anchors {
            horizontalCenter: {
                if( (_sectors & 1) !== 0 && (_sectors & 4) !== 0 )
                    parent.horizontalCenter
                else if( (_sectors & 1) !== 0 )
                    parent.left
                else
                    parent.right
            }
            verticalCenter: {
                if( (_sectors & 2) !== 0 && (_sectors & 8) !== 0 )
                    parent.verticalCenter
                else if( (_sectors & 2) !== 0 )
                    parent.top
                else
                    parent.bottom
            }
        }
        width: action_name.contentWidth + 10 * screenScale
        height: action_name.contentHeight + 5 * screenScale

        color: "#aaffffff"

        radius: 2 * screenScale

        Text {
            id: action_name
            anchors.centerIn: parent

            text: defaultText
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

