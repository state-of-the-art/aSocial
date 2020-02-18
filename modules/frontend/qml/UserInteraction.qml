import QtQuick 2.4
import "Components"
import "js/userinteraction.js" as U

Item {
    id: root
    anchors.fill: parent

    property alias action_menu: action_menu
    property alias action_delayed: action_delayed

    ActionMenu {
        id: action_menu
    }

    ActionWait {
        id: action_delayed
    }

    Component.onCompleted: {
        U.init(app, root)
    }
}
