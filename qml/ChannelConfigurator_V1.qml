import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.Layouts

RowLayout {
    id: ch_settings
    height: 100
    spacing: 10
    Layout.alignment: Qt.AlignLeft

    property real ch_id: -1
    property real ch_current_value: 69
    property real ch_min: -100
    property real ch_max: 100

    enum Behaviours {
        Toggle,
        Toggle_sym,
        Tap,
        Raw_Axis
    }

    property var behaviour_types: ["Toggle", "Toggle (Symmetrical)", "Tap", "Raw Axis"];
    property var selected_behaviour: -1;
    property int target_value: 0;
    property string input_label: "" 
    signal inputCheckedChanged(bool checked)

    // Channel info + progress bar
    ColumnLayout {
        spacing: 5
        Layout.alignment: Qt.AlignLeft

        Text {
            id: channel_id
            text: `Channel ${ch_settings.ch_id}`
            color: Universal.baseHighColor
            font.pointSize: 16
        }

        Item {
            width: 150
            height: 40

            ProgressBar {
                id: raw_progressBar
                anchors.top: parent.top
                width: parent.width
                height: 20
                from: ch_settings.ch_min
                to: ch_settings.ch_max
                value: ch_settings.ch_current_value
            }

            Text {
                anchors.left: raw_progressBar.left
                anchors.top: raw_progressBar.bottom
                text: ch_settings.ch_min
                font.pointSize: 12
                color: Universal.baseMediumHighColor
            }

            Text {
                anchors.horizontalCenter: raw_progressBar.horizontalCenter
                anchors.top: raw_progressBar.bottom
                text: ch_settings.ch_current_value
                font.pointSize: 12
                font.weight: Font.Bold
                color: Universal.baseHighColor
            }

            Text {
                anchors.right: raw_progressBar.right
                anchors.top: raw_progressBar.bottom
                text: ch_settings.ch_max
                font.pointSize: 12
                color: Universal.baseMediumHighColor
            }
        }
    }

    // Input button
    Button {
        id: input
        text: checked ? "Waiting for input..." : ch_settings.input_label === "" ? "Set input" : ch_settings.input_label
        checkable: true
        onCheckedChanged: inputCheckedChanged(checked)
    }

    // Behaviour combo box
    ComboBox {
        id: comboBox
        displayText: ch_settings.selected_behaviour === -1 ? "Select Behaviour" : model[currentIndex]
        currentIndex: ch_settings.selected_behaviour
        model: ch_settings.behaviour_types
        onCurrentIndexChanged: ch_settings.selected_behaviour = currentIndex
    }

    // Target value controls
    RowLayout {
        spacing: 5

        Button {
            text: "-"
            onClicked: ch_settings.target_value -= 100
        }

        TextInput {
            id: target_value
            text: `${ch_settings.target_value}`
            width: 50
            font.pixelSize: 20
            color: Universal.baseHighColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            inputMethodHints: Qt.ImhDigitsOnly
            validator: IntValidator { bottom: ch_settings.ch_min; top: ch_settings.ch_max }

            onTextChanged: {
                const val = parseInt(text)
                if (!isNaN(val)) ch_settings.target_value = val
            }
        }

        Button {
            text: "+"
            onClicked: ch_settings.target_value += 100
        }
    }

    // Apply / Clear buttons
    Button { text: "Apply" }

    Button {
        text: qsTr("Clear")
        onClicked: {
            ch_settings.selected_behaviour = -1
            ch_settings.target_value = 0
            ch_settings.input_label = ""
            input.checked = false
        }
    }
}
