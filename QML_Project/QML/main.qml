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
    }

    Rectangle {
        anchors.top: appTitleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#ffffff"
    }
}
