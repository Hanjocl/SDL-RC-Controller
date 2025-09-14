import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    id: window
    title: qsTr("SDL RC Controller - Example")

    Universal.theme: Universal.Dark

    Rectangle {
        id: root_channels
        anchors.fill: parent
        color: "#2b2b2b"

        focus: true
        Keys.onPressed: function(event) {
            SdlController.injectKey(event.key, event.text)
        }

        function handleInputCheckedChanged(checked) {
            console.log("Checked state changed:", checked)
            if (checked) {
                // Get single input
            } else {
                // Stop scanning and start polling again if polling was active before
            }
        }

        // Scrollable container
        ScrollView {
            anchors.fill: parent

            ColumnLayout {
                id: channelsLayout
                spacing: 10

                Repeater {
                    id: channelRepeater
                    model: 16

                    ChannelConfigurator_V2 {
                        ch_id: model.index
                        ch_current_value: SdlController.channelValues[model.index]
                        ch_min: 1000
                        ch_max: 2000
                        onScanForInputChanged: (checked) => root_channels.handleInputCheckedChanged(checked)
                        onApplyConfig: { console.log("Apply config for channel", ch_id) }
                        onClearConfig: { console.log("Clear config for channel", ch_id) }
                    }
                }
            }
        }
    }
}
