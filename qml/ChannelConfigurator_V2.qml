import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.Layouts

ColumnLayout {
    id: ch_settings
    spacing: 10

    property real ch_id: -1
    property real ch_current_value: 1569
    property real ch_min: -9999
    property real ch_max: 9999
    
    property string input_label: ""
    property bool checked: false
    signal scanForInputChanged(bool checked)

    signal applyConfig()
    signal clearConfig()

    property var selected_behaviour: 0
    property int value_offset: 0

    // Existing channel settings row
    RowLayout {
        Text {
            id: channel_id
            text: `Channel ${ch_settings.ch_id}`
            color: Universal.baseHighColor
            font.pointSize: 16
            Layout.minimumWidth: 150
        }

        Button {
            id: input
            Layout.minimumWidth: 120
            checkable: true
            checked: ch_settings.checked
            text: ch_settings.checked ? "Waiting for input..." : (ch_settings.input_label === "" ? "Set input" : ch_settings.input_label)

            onClicked: {
                ch_settings.checked = input.checked
                ch_settings.scanForInputChanged(ch_settings.checked)
            }
        }

        ComboBox {
            id: behaviour_selector
            Layout.minimumWidth: 150
            model: ["NONE", "RAW", "TAP", "HOLD", "RELEASE", "INCREMENT", "TOGGLE", "TOGGLE_SYMETRIC"]
            onCurrentIndexChanged: {
                ch_settings.selected_behaviour = behaviour_selector.currentIndex
            }
            
            // Initialize once
            Component.onCompleted: {
                behaviour_selector.currentIndex = ch_settings.selected_behaviour
            }

        }

        RowLayout {
            Button {
                text: "-"
                onClicked: {
                    if (ch_settings.value_offset - 100 > ch_settings.ch_min) {
                        ch_settings.value_offset -= 100
                    } else {
                        ch_settings.value_offset = ch_settings.ch_min
                    }
                }
            }

            TextInput {
                id: value_offset
                text: `${ch_settings.value_offset}`
                Layout.minimumWidth: 50
                font.pixelSize: 20
                color: Universal.baseHighColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                validator: IntValidator { bottom: ch_settings.ch_min; top: ch_settings.ch_max }

                onTextChanged: {
                    const val = parseInt(text)
                    if (!isNaN(val)) ch_settings.value_offset = val
                }
            }

            Button {
                text: "+"
                onClicked: {
                    if (ch_settings.value_offset + 100 < ch_settings.ch_max) {
                        ch_settings.value_offset += 100
                    } else {
                        ch_settings.value_offset = ch_settings.ch_max
                    }
                }
            }
        }

        Button {
            text: "Apply"
            onClicked: {
                if (ch_settings.selected_behaviour === 0) {
                    console.log("Please select a behaviour before applying the configuration.")
                    return                
                }
                if (input.text === "No config found" || input.text === "Set input") {
                    console.log("Please set an input before applying the configuration.")
                    return
                }
                if (ch_settings.ch_id === -1) {
                    console.log("Invalid channel ID.")
                    return
                }
                ch_settings.applyConfig()
            }
        }

        Button {
            text: "Clear"
            onClicked: {
                ch_settings.selected_behaviour = 0
                ch_settings.value_offset = 0
                ch_settings.input_label = ""
                input.checked = false
                ch_settings.clearConfig()
            }
        }

        Item { 
            id: spacer_width
            Layout.fillWidth: true
        }
    }

    // New progress bar section underneath
    Item {
        height: 40        

        // The ProgressBar
        ProgressBar {
            id: raw_progressBar
            anchors.left: parent.left
            anchors.leftMargin: channel_id.width + ch_settings.spacing/2
            width: ch_settings.width - channel_id.width - ch_settings.spacing
            height: 20
            from: ch_settings.ch_min
            to: ch_settings.ch_max
            value: ch_settings.ch_current_value

            background: Rectangle { 
                implicitHeight: raw_progressBar.height
                color: '#8b8b8b' 
            } 
            contentItem: Rectangle { 
                implicitHeight: raw_progressBar.height
                width: raw_progressBar.width * (raw_progressBar.value - raw_progressBar.from) / (raw_progressBar.to - raw_progressBar.from)
                color: '#555555' 
            }

            Component.onCompleted: { raw_progressBar.value = ch_settings.ch_current_value} // Need to overcome a bug where it does not update the value initially
        }

        // Overlay texts on top of the ProgressBar
        Text {
            anchors.left: raw_progressBar.left
            anchors.leftMargin: 4
            anchors.verticalCenter: raw_progressBar.verticalCenter
            text: ch_settings.ch_min
            font.pointSize: 12
            color: "white"
        }

        Text {
            anchors.right: raw_progressBar.right
            anchors.rightMargin: 4
            anchors.verticalCenter: raw_progressBar.verticalCenter
            text: ch_settings.ch_max
            font.pointSize: 12
            color: "white"
        }

        Text {
            id: currentValueText
            anchors.centerIn: raw_progressBar
            text: ch_settings.ch_current_value
            font.pointSize: 12
            font.bold: true
            color: "white"
        }
    }

    Item { 
            id: spacer_h
            Layout.fillHeight: true
        }
}