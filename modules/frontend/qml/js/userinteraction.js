/**
 * UserInteraction library
 * Provides singleton user action functions
**/
.pragma library

var app = null
var _u = null

function init(app_obj, user_interaction_obj) {
    console.log("Init user interaction library")
    app = app_obj
    _u = user_interaction_obj
}

/**
 * Window size
**/
function windowSize() {
    return {
        width: _u.width,
        height: _u.height
    }
}

/**
 * Action Menu
**/

function actionMenuShow(view_pos, actions) {
    return _u.action_menu.show(view_pos, actions)
}

function actionMenuHide() {
    return _u.action_menu.hide()
}


/**
 * Action Delayed
**/

function actionDelayedStart(view_pos, slot) {
    actionDelayedStop()
    return _u.action_delayed.start(view_pos, slot)
}

function actionDelayedStop() {
    return _u.action_delayed.stop()
}
