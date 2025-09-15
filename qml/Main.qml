import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.Layouts
import QtQuick.Dialogs

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

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            // Top row: Load & Save buttons
            RowLayout {
                spacing: 10
                width: parent.width

                // Load button
                FileDialog {
                    id: loadDialog
                    title: "Open Config File"
                    nameFilters: ["JSON files (*.json)"]
                    onAccepted: {
                        var localPath = fileUrl.replace("file://", "")
                        SdlController.loadConfig()
                    }
                }

                Button {
                    text: "Load Config"
                    Layout.preferredWidth: 150
                    onClicked: SdlController.loadConfig()
                }

                // Save button
                Button {
                    text: "Save Config"
                    Layout.preferredWidth: 150
                    onClicked: SdlController.saveConfig()
                }

            }

            // Scrollable container for channels
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

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
                            onScanForInputChanged: (checked) => root_channels.handleInputCheckedChanged(ch_id, checked)
                            onApplyConfig: {
                                SdlController.ApplyChannelSettings(ch_id, selected_behaviour, value_offset) 
                            }
                            onClearConfig: { 
                                SdlController.ClearChannelConfig(ch_id)
                            }
                        }
                    }

                    Connections {
                        target: SdlController
                        onConfigLoaded: {
                            for (var i = 0; i < channelRepeater.count; i++) {
                                var child = channelRepeater.itemAt(i)
                                if (child) {
                                    child.ch_current_value = SdlController.channelValues[i]
                                    child.input_label = SdlController.getChannelInputLabel(i)
                                    child.selected_behaviour = SdlController.getMode(i)
                                    child.value_offset = SdlController.getChannelOffset(i)
                                }
                            }
                        }
                    }

                }
            }
        }

        function handleInputCheckedChanged(ch_id, checked) {
            var child = channelRepeater.itemAt(ch_id)
            if (!child) return

            if (checked) {
                // Show "Waiting for input..." immediately
                child.input_label = ""  // clear old label

                // Start asynchronous scanning
                Qt.callLater(function() {
                    var inputText = SdlController.setInput(ch_id) // blocks here, but after QML updates
                    if (inputText && inputText.length > 0) {
                        child.input_label = inputText
                        // button.checked = false
                    }
                    child.checked = false
                })
            } else {
                // Stop scanning if needed
                SdlController.stopScanning()
            }
            
            root_channels.forceActiveFocus()
        }
    }
}