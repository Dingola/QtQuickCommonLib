import QtQuick
import QtQuick.Controls
import QtQuickCommonLib 1.0
import "Components"

AppWindow {
    id: root
    visible: true
    width: 1280
    height: 1024
    minimumWidth: 200
    minimumHeight: 200
    title: "Qt Quick App"
    useMica: false
    useRoundedCorners: false
    color: "#f3f3f3"

    property int linuxResizeBorderThickness: 8

    titleBarItem: appTitleBar
    minimizeButtonItem: appTitleBar.minimizeButtonItem
    maximizeButtonItem: appTitleBar.maximizeButtonItem
    closeButtonItem: appTitleBar.closeButtonItem
    customTitleBarItem: appTitleBar.customTitleBarItem

    WindowTitleBar {
        id: appTitleBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        title: root.title
        isMaximized: root.isMaximizedNative
        backgroundColor: "transparent"

        onMinimizeRequested: root.showMinimized()
        onMaximizeRequested: root.showMaximized()
        onRestoreRequested: root.showNormal()
        onCloseRequested: root.close()
        onDragRequested: root.start_system_move()
        onTitleBarDoubleClicked: {
            if (root.isMaximizedNative) {
                root.showNormal()
            } else {
                root.showMaximized()
            }
        }
    }

    Rectangle {
        anchors.top: appTitleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#ffffff"
    }

    Item {
        anchors.fill: parent
        z: 1000
        visible: Qt.platform.os === "linux" && !root.isMaximizedNative

        MouseArea {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeHorCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.LeftEdge)
                }
            }
        }

        MouseArea {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeHorCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.RightEdge)
                }
            }
        }

        MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeVerCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.TopEdge)
                }
            }
        }

        MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeVerCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.BottomEdge)
                }
            }
        }

        MouseArea {
            anchors.left: parent.left
            anchors.top: parent.top
            width: root.linuxResizeBorderThickness
            height: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeFDiagCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.LeftEdge | Qt.TopEdge)
                }
            }
        }

        MouseArea {
            anchors.right: parent.right
            anchors.top: parent.top
            width: root.linuxResizeBorderThickness
            height: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeBDiagCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.RightEdge | Qt.TopEdge)
                }
            }
        }

        MouseArea {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: root.linuxResizeBorderThickness
            height: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeBDiagCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.LeftEdge | Qt.BottomEdge)
                }
            }
        }

        MouseArea {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: root.linuxResizeBorderThickness
            height: root.linuxResizeBorderThickness
            cursorShape: Qt.SizeFDiagCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    root.start_system_resize(Qt.RightEdge | Qt.BottomEdge)
                }
            }
        }
    }
}
