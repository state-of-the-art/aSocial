import QtQuick 2.4
import QtGraphicalEffects 1.0
import co.asocial 1.0 // WDate enums
import "LineOfLife"
import "../js/lineoflife.js" as L
import "../js/account.js" as A
import "../js/userinteraction.js" as U

Rectangle {
    id: lineoflife

    property var _profile
    property var _profile_data

    property var _visible_from
    property var _visible_interval

    visible: false

    color: "#77ffffff"
    radius: 5
    border.width: 1

    clip: true

    function setProfile(profile_obj) {
        _profile = profile_obj
        _profile_data = profile_obj.obj_data.data

        lineoflife.visible = true

        resetView()
    }

    function resetView() {
        _visible_from = _profile_data.birth_date !== null ? _profile_data.birth_date + wdate.tzOffsetSec() : 0
        _visible_interval = _profile_data.death_date !== null ? _profile_data.death_date + wdate.tzOffsetSec() - _visible_from : wdate.currentUnixtime() - _visible_from

        // Make small prepend & append to see the full picture
        var pend = Math.ceil(_visible_interval/20)
        _visible_from -= pend
        _visible_interval += pend*2

        L.updateAxis()
    }

    function newEvent(occur) {
        console.log("LineOfLife: Creating new event " + occur)

        //L.updateAxis()

        A.showEvent({ occur: occur,
                      link: null,
                      type: "fact",
                      owner: _profile.id,
                      recipient: null,
                      data: {text: ""},
                      overlay: {}
                    }, A.saveEvent)
    }

    onWidthChanged: {
        L.setAxisDetailLevel()
        L.updateAxis()
    }

    on_Visible_intervalChanged: {
        L.setAxisDetailLevel()
    }

    Item {
        id: content
        anchors.fill: parent

        function zoomIn(unixtime) {
            if( L.current_detail_level === WDate.HOUR )
                return

            // Zoom half of current visible
            if( unixtime !== undefined )
                _visible_from = unixtime - Math.ceil(_visible_interval / 4)
            else
                _visible_from += Math.ceil(_visible_interval / 2)

            _visible_interval = Math.ceil(_visible_interval / 2)

            L.updateAxis()
            axis_cursor.setTo(unixtime)
        }

        function zoomOut(unixtime) {
            if( L.current_detail_level === WDate.YEAR && axis.children.length >= 100 )
                return

            // Zoom out twice of current visible
            if( unixtime !== undefined )
                _visible_from = unixtime - _visible_interval
            else
                _visible_from -= Math.ceil(_visible_interval / 2)
            _visible_interval *= 2

            L.updateAxis()
        }

        function centerTo(unixtime) {
            _visible_from = unixtime - Math.ceil(_visible_interval / 2)

            L.updateAxis()
        }

        Item {
            id: axis
            anchors.fill: parent

            Component {
                id: axis_mark
                AxisMark {}
            }
        }

        Item {
            id: events
            anchors.fill: parent

            Component {
                id: event_mark
                EventMark {}
            }
        }

        Flickable {
            id: axis_flickable
            anchors.fill: parent
            pressDelay: 300
            clip: true

            contentWidth: 0
            contentHeight: 0
            contentX: 0

            flickableDirection: Flickable.HorizontalFlick
            rightMargin: 5000
            leftMargin: 5000

            property var _prev_from

            onMovementStarted: {
                _prev_from = _visible_from
            }

            onMovementEnded: {
                rightMargin = 5000
                leftMargin = 5000
                contentX = 0
            }

            onContentXChanged: {
                if( moving ) {
                    _visible_from = _prev_from + L.pointToTime(contentX) - _visible_from
                    L.updateAxis()

                    if( contentX < 2000 - leftMargin  )
                        leftMargin += 5000
                    else if( contentX > rightMargin - 2000 )
                        rightMargin += 5000
                }
            }

            MouseArea {
                id: axis_mouse_area

                parent: axis_flickable
                anchors.fill: parent
                z: -1

                preventStealing: true

                hoverEnabled: true
                drag.threshold: 0

                onPressed: {
                    console.log("LineOfLife pressed")
                    axis_cursor.setTo(L.pointToTime(mouse.x))

                    var point = Qt.point(mouse.x + lineoflife.x, mouse.y + lineoflife.y)
                    var actions = [
                                { name: qsTr("New Event"), color: "#faa", action: lineoflife.newEvent, property: axis_cursor.unixtime },
                                { name: qsTr("Zoom +"), color: "#faf", action: content.zoomIn, property: axis_cursor.unixtime },
                                { name: qsTr("Zoom -"), color: "#ffa", action: content.zoomOut }
                            ]

                    // Get first 10 events and show in menu
                    var near_events = A.findEvents(L.pointToTime(mouse.x-2), L.pointToTime(mouse.x+2), A.getTypeId("fact"), -1, -1, 10)
                    if( near_events.length > 0 ) {
                        near_events = A.getEvents(near_events)
                        for( var i in near_events ) {
                            var e = near_events[i]
                            actions.push({ name: e.occur, color: "#aaa", action: function(id) { A.showEvent(A.getEvents([id])[0], A.saveEvent) }, property: e.id })
                            L.selectEvent(e.id)
                        }
                    }

                    drag.target = U.actionMenuShow(point, actions)
                }

                onReleased: {
                    console.log("LineOfLife released")

                    U.actionMenuHide()
                    drag.target = null
                }

                onPositionChanged: {
                    axis_cursor.setTo(L.pointToTime(mouse.x))
                }

                onWheel: {
                    console.log("LineOfLife wheel")

                    var time = axis_cursor.unixtime

                    if( wheel.angleDelta.y > 0 )
                        content.zoomIn(time)
                    else if( wheel.angleDelta.y < 0 )
                        content.zoomOut()
                    else
                        content.pos(time)
                }
            }
        }

        AxisCursor {
            id: axis_cursor
            x: L.timeToPoint(unixtime)
            anchors.bottom: parent.bottom
            height: axis.height

            visible: axis_mouse_area.containsMouse
        }
    }
}
