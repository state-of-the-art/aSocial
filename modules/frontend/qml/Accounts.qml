import QtQuick 2.3
import "Components"

Rectangle {
    id: component
    anchors.fill: parent
    clip: true
    color: "#ff777777"

    signal accountSelected(int account_id)

    function show() {
        component.visible = true
    }
    function hide() {
        component.visible = false
    }
    function update() {
        list.model.clear()
        list.model.append(app.getAccounts())
        list.model.append({id: -1, name: qsTr("New account"), description: qsTr("Click here to add a new account")})
        list.focus = true
    }

    Text {
        anchors.fill: parent
        anchors.margins: 2 * screenScale

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        fontSizeMode: Text.Fit
        minimumPixelSize: 10 * screenScale
        font.pixelSize: 72 * screenScale
        font.weight: Font.Bold
        color: "#ccffffff"

        text: qsTr("Choose your destiny")
    }

    ListView {
        id: list
        anchors.fill: parent
        anchors.margins: 20 * screenScale

        model: ListModel {}
        delegate: Rectangle {
            property variant modelData: model
            height: 50 * screenScale
            width: parent.width
            radius: 5 * screenScale
            color: "#88004400"
            border.width: 1 * screenScale

            function select() {
                if( id === -1 ) {
                    if( text_description.text === '' ) {
                        text_description.setEdit(true)
                        text_description.focus = true
                    }
                    if( text_name.text === '' ) {
                        text_name.setEdit(true)
                        text_name.focus = true
                    }

                    if( text_name.text !== '' && text_description.text !== '' ) {
                        var newid = app.createAccount({name: text_name.text, description: text_description.text},
                                                      app.passwordGetWait(qsTr("Please enter password for the new account"), true))
                        accountSelected(newid)
                        component.hide()
                    }
                } else {
                    accountSelected(id)
                    component.hide()
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: select()
                onEntered: list.currentIndex = index

                Column {
                    anchors.fill: parent
                    anchors.margins: 3 * screenScale

                    EditableInput {
                        id: text_name
                        font.weight: Font.Bold
                        text: id >= 0 ? name : ''
                        default_text: name
                        enabled: false

                        next_item: text_description
                    }
                    EditableInput {
                        id: text_description
                        text: id >= 0 ? description : ''
                        default_text: description
                        enabled: false

                        next_item: text_name
                    }
                }
            }
        }

        highlight: Rectangle {
            color: "#aa00ff00"
            radius: 5 * screenScale
        }

        Keys.onReturnPressed: currentItem.select()
    }
}
