var _levels = [
     { seconds: 60*60,        minwidth: 6 * screenScale, size: 0.30, format: "hh",   }, // 0 hour
     { seconds: 60*60*24,     minwidth: 4 * screenScale, size: 0.40, format: "dd",   }, // 1 day
     { seconds: 60*60*24*31,  minwidth: 10 * screenScale, size: 0.60, format: "MM",   }, // 2 month
     { seconds: 60*60*24*365, minwidth: 20 * screenScale, size: 0.80, format: "yyyy", }  // 3 year
]

var _visible_events = {}
var _birth_event = null
var _death_event = null

var current_detail_level = _levels.length - 1

function setAxisDetailLevel() {
    var level = null
    for( level in _levels ) {
        var delta = timeToPoint(lineoflife._visible_from + _levels[level].seconds)
        if( delta > _levels[level].minwidth )
            break
    }
    current_detail_level = parseInt(level) // Due to level is string right now
}

function createAxisMark(unixtime) {
    return axis_mark.createObject(axis, {})
}

function setMark(mark_obj, unixtime) {
    // Determine level of mark
    var level = wdate.getLevel(unixtime*1000)
    level = level < current_detail_level ? current_detail_level : level

    mark_obj.x = timeToPoint(unixtime)
    mark_obj.unixtime = unixtime
    mark_obj.format = _levels[level].format
    mark_obj.height = axis.height * _levels[level].size
    mark_obj.level = level
}

function createBirthDayMark(unixtime) {
    unixtime -= wdate.tzOffsetSec() // birth date is unixtime day without hours
    return event_mark.createObject(
                events, {
                    x: timeToPoint(unixtime),
                    unixtime: unixtime
                })
}

function createDeathDayMark(unixtime) {
    unixtime -= wdate.tzOffsetSec() // death date is unixtime day without hours
    return event_mark.createObject(
                events, {
                    x: timeToPoint(unixtime),
                    unixtime: unixtime
                })
}

function createEventMark(data) {
    return event_mark.createObject(
                events, {
                    x: timeToPoint(data.occur),
                    unixtime: data.occur,
                    color: "#0a0"
                })
}

function arrayNotIn(a, b) {
    return a.filter(function(val) {
        return b.indexOf(val) < 0
    })
}

function cleanEvents(toclean) {
    for( var i in toclean ) {
        console.debug("Destroying " + toclean[i])
        _visible_events[toclean[i]].destroy()
        delete _visible_events[toclean[i]]
    }
}

function updateAxis() {
    if( lineoflife.visible === false )
        return

    var marks = wdate.getAxisMarks(current_detail_level, lineoflife._visible_from*1000, lineoflife._visible_interval*1000)

    for( var i in marks ) {
        var obj = axis.children[i]

        if( obj === undefined )
            obj = createAxisMark()

        setMark(obj, marks[i])
        obj.init(axis.children[i-1])
    }

    // Cleaning not required axis marks
    for( i = axis.children.length; i > marks.length ; i-- )
        axis.children[i-1].destroy()


    // Create profile events marks
    // TODO: required optimization
    var events_ids = A.findEvents(_visible_from, _visible_from + _visible_interval, A.getTypeId("fact"))

    // Clean & update all events not in found events
    var events_exists = Object.keys(_visible_events).map(function(val) { return parseInt(val) })
    cleanEvents(arrayNotIn(events_exists, events_ids))
    for( var e in _visible_events )
        _visible_events[e].updatePos()

    // Get all not existed events from the db
    events_exists = Object.keys(_visible_events).map(function(val) { return parseInt(val) })
    var new_ids = arrayNotIn(events_ids, events_exists)
    if( new_ids.length > 0 ) {
        var new_events = A.getEvents(new_ids)
        for( var n in new_events )
            _visible_events[new_events[n].id] = createEventMark(new_events[n])
    }


    // Creating birth/death marks
    if( _profile_data.birth_date !== null ) {
        if( _birth_event !== null )
            _birth_event.updatePos()
        else
            _birth_event = createBirthDayMark(_profile_data.birth_date)
    } else if( _death_event !== null ) {
        _birth_event.destroy()
        _birth_event = null
    }
    if( _profile_data.death_date !== null ) {
        if( _death_event !== null )
            _death_event.updatePos()
        else
            _death_event = createBirthDayMark(_profile_data.death_date)
    } else if( _death_event !== null ) {
        _death_event.destroy()
        _death_event = null
    }
}

function selectEvent(id) {
    if( _visible_events.hasOwnProperty(id) )
        _visible_events[id].focus = true
}


function pointToTime(point) {
    return Math.ceil((1.0 * point / axis_mouse_area.width) * lineoflife._visible_interval + lineoflife._visible_from)
}

function timeToPoint(unixtime) {
    return Math.ceil((1.0 * (unixtime - lineoflife._visible_from) / lineoflife._visible_interval) * axis_mouse_area.width)
}
