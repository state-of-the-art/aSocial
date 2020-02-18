import QtQuick 2.3
import QtQuick.Layouts 1.1
import "Components"

Rectangle {
    id: component
    anchors.fill: parent
    color: "#cc000000"
    visible: false

    signal done(string password)

    function show(description, create) {
        desc.text = description
        state = create ? 'create new password' : ''
        component.visible = true
        input.focus = true
    }

    function hideOk() {
        if( input.text == "" ) {
            desc.color = "#f00"
            desc.text = qsTr("Password can't be empty")
            return
        }
        if( state === 'create new password' ) {
            if( input_repeat.text == "" ) {
                desc.color = "#000"
                desc.text = qsTr("Please repeat password")
                input_repeat.focus = true
                return
            } else if( input.text != input_repeat.text ) {
                desc.color = "#f00"
                desc.text = qsTr("Passwords not match")
                input_repeat.text = ""
                input_repeat.focus = true
                return
            }
        }

        component.visible = false
        desc.text = "no description"
        done(input.text)
        input.text = ""
        input_repeat.text = ""
    }

    Rectangle {
        id: window
        color: "#fff"
        anchors.centerIn: parent
        border.color: "#000"
        border.width: 2 * screenScale
        radius: 5 * screenScale

        width: 300 * screenScale
        height: 100 * screenScale

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10 * screenScale
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                id: desc
                text: "no description"
            }

            Rectangle {
                color: "#33000000"
                border.color: "#000"
                border.width: 2 * screenScale
                height: 30 * screenScale
                anchors {
                    left: parent.left
                    right: parent.right
                }

                Text {
                    color: "#44333333"
                    anchors.fill: parent

                    clip: true
                    visible: input.text == ""
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    font {
                        family: "monospace"
                        pixelSize: 20 * screenScale
                    }

                    text: "Enter password"
                }

                TextInput {
                    id: input
                    color: "#333"
                    anchors.fill: parent

                    clip: true
                    horizontalAlignment: TextInput.AlignHCenter
                    verticalAlignment: TextInput.AlignVCenter
                    onAccepted: {
                        if( state === 'create new password' && input_repeat.text == "" )
                            input_repeat.focus = true
                        else
                            hideOk()
                    }
                    KeyNavigation.tab: input_repeat

                    font {
                        bold: true
                        family: "monospace"
                        pixelSize: 20 * screenScale
                    }

                    passwordCharacter: "*"
                    echoMode: TextInput.Password
                }
            }

            Rectangle {
                id: repeat_password
                color: "#33000000"
                border.color: input_repeat.text == input.text ? "#000" : "#f00"
                border.width: 2 * screenScale
                height: 30 * screenScale
                anchors.left: parent.left
                anchors.right: parent.right

                visible: false

                Text {
                    color: "#44333333"
                    anchors.fill: parent

                    clip: true
                    visible: input_repeat.text == ""
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    font {
                        family: "monospace"
                        pixelSize: 20 * screenScale
                    }

                    text: "Repeat password"
                }

                TextInput {
                    id: input_repeat
                    color: "#333"
                    anchors.fill: parent

                    clip: true
                    horizontalAlignment: TextInput.AlignHCenter
                    verticalAlignment: TextInput.AlignVCenter
                    onAccepted: hideOk()
                    KeyNavigation.tab: input

                    font {
                        bold: true
                        family: "monospace"
                        pixelSize: 20 * screenScale
                    }

                    passwordCharacter: "*"
                    echoMode: TextInput.Password
                }
            }

            Button {
                id: button_ok
                anchors.horizontalCenter: parent.horizontalCenter

                width: 80 * screenScale
                height: 30 * screenScale
                color: "#595"

                caption: qsTr("Ok")

                onClicked: hideOk()
            }
        }
    }

    states: [
        State {
            name: 'create new password'
            PropertyChanges { target: repeat_password; visible: true }
            PropertyChanges { target: window; height: 150 * screenScale }
        }
    ]
}
