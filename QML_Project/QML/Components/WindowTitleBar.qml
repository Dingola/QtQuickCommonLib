import QtQuick
import QtQuick.Controls
import QtQuick.Window 2.15

Item {
    id: root
    width: parent ? parent.width : 0
    height: 36

    default property alias customContentData: customContentLayout.data

    property string title: ""
    property bool isMaximized: false
    property color backgroundColor: "transparent"
    property int icon_px: 16

    readonly property Item minimizeButtonItem: minimizeButton
    readonly property Item maximizeButtonItem: maximizeButton
    readonly property Item closeButtonItem: closeButton
    readonly property Item customTitleBarItem: customContentLayout

    signal minimizeRequested()
    signal maximizeRequested()
    signal restoreRequested()
    signal closeRequested()

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    Label {
        id: titleLabel
        text: root.title
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.verticalCenter: parent.verticalCenter
        color: "black"
        font.pixelSize: 12
        font.bold: true
        elide: Text.ElideRight
    }

    Item {
        id: customContentHost
        x: titleLabel.x + titleLabel.implicitWidth + 12
        width: Math.max(0, window_controls_row.x - x)
        height: parent.height
        clip: true

        Item {
            id: customContentLayout
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: childrenRect.width
            height: parent.height
        }
    }

    Row {
        id: window_controls_row
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: 0

        component TitleBarButton : Button {
            id: btn
            width: 46
            height: parent.height
            flat: true
            focusPolicy: Qt.NoFocus

            property url iconPath: ""
            property color hoverColor: "#e9e9e9"
            property color pressColor: "#ededed"

            background: Rectangle {
                anchors.fill: parent
                color: btn.down ? btn.pressColor : (btn.hovered ? btn.hoverColor : "transparent")
            }

            contentItem: Item {
                anchors.fill: parent

                Image {
                    source: btn.iconPath
                    anchors.centerIn: parent

                    width: root.icon_px
                    height: root.icon_px

                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: width * Screen.devicePixelRatio
                    sourceSize.height: height * Screen.devicePixelRatio
                    mipmap: false
                    smooth: true
                }
            }
        }

        TitleBarButton {
            id: minimizeButton
            iconPath: "qrc:/Resources/Icons/titlebar-minimize.svg"
            onClicked: root.minimizeRequested()
        }

        TitleBarButton {
            id: maximizeButton
            iconPath: root.isMaximized
                      ? "qrc:/Resources/Icons/titlebar-restore.svg"
                      : "qrc:/Resources/Icons/titlebar-maximize.svg"
            onClicked: {
                if (root.isMaximized) {
                    root.restoreRequested()
                } else {
                    root.maximizeRequested()
                }
            }
        }

        TitleBarButton {
            id: closeButton
            iconPath: "qrc:/Resources/Icons/titlebar-close.svg"
            hoverColor: "#e81123"
            pressColor: "#f1707a"
            onClicked: root.closeRequested()
        }
    }
}
