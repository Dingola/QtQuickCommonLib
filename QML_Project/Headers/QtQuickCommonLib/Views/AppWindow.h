#pragma once

#include <QEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <memory>

#include "QtQuickCommonLib/ApiMacro.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace QtQuickCommonLib
{

class AppWindowLinux;
class AppWindowWin;

/**
 * @class AppWindow
 * @brief Custom top-level window for QML with a fully customizable client-side title bar.
 *
 * This class implements a QQuickWindow that:
 *  - removes the native title bar and provides a mechanism for a custom QML title bar
 *  - preserves native window behaviors on Windows: aero snap, border resize,
 *    minimize/maximize/close, multi-monitor maximize, DPI-aware sizing
 *  - maps clicks on caption buttons to HTCLIENT so QML can handle them
 *  - supports Windows 11 features like Mica backdrop, rounded corners, and (best-effort) dark
 *    immersive borders/caption attributes
 *
 * Usage in QML:
 *  AppWindow {
 *      titleBarItem: myTitleBar // Bind your QML TitleBar item here
 *      useMica: true
 *  }
 *
 * @note Windows-only implementation details are guarded by #ifdef Q_OS_WIN.
 */
class QTQUICKCOMMONLIB_API AppWindow: public QQuickWindow
{
        Q_OBJECT
        QML_ELEMENT

        /**
         * @brief The QML item acting as the title bar.
         *
         * This property must be set to the QML Item representing the title bar.
         * It is used for hit-testing (WM_NCHITTEST) to distinguish between dragging the window
         * and interacting with controls within the title bar.
         */
        Q_PROPERTY(QQuickItem* titleBarItem READ get_title_bar_item WRITE set_title_bar_item NOTIFY
                       title_bar_item_changed FINAL)

        /**
         * @brief The QML item representing the minimize button hit-test region.
         *
         * If the cursor is over this item, WM_NCHITTEST returns HTCLIENT so QML receives clicks.
         */
        Q_PROPERTY(QQuickItem* minimizeButtonItem READ get_minimize_button_item WRITE
                       set_minimize_button_item NOTIFY minimize_button_item_changed FINAL)

        /**
         * @brief The QML item representing the maximize/restore button hit-test region.
         *
         * If the cursor is over this item, WM_NCHITTEST returns HTCLIENT so QML receives clicks.
         */
        Q_PROPERTY(QQuickItem* maximizeButtonItem READ get_maximize_button_item WRITE
                       set_maximize_button_item NOTIFY maximize_button_item_changed FINAL)

        /**
         * @brief The QML item representing the close button hit-test region.
         *
         * If the cursor is over this item, WM_NCHITTEST returns HTCLIENT so QML receives clicks.
         */
        Q_PROPERTY(QQuickItem* closeButtonItem READ get_close_button_item WRITE
                       set_close_button_item NOTIFY close_button_item_changed FINAL)

        /**
         * @brief The QML item representing an additional interactive title bar region.
         *
         * This property can be used for custom QML controls placed inside the title bar
         * (for example search fields, tool buttons, or custom composite items). If the
         * cursor is over this item or one of its child items, WM_NCHITTEST returns HTCLIENT
         * so QML receives the interaction instead of starting a window drag.
         */
        Q_PROPERTY(QQuickItem* customTitleBarItem READ get_custom_title_bar_item WRITE
                       set_custom_title_bar_item NOTIFY custom_title_bar_item_changed FINAL)

        /**
         * @brief User preference: enable Mica-like backdrop when supported (Windows 11+).
         */
        Q_PROPERTY(bool useMica READ get_use_mica WRITE set_use_mica NOTIFY use_mica_changed FINAL)

        /**
         * @brief User preference: enable rounded corners when supported (Windows 11+).
         */
        Q_PROPERTY(bool useRoundedCorners READ get_use_rounded_corners WRITE set_use_rounded_corners
                       NOTIFY use_rounded_corners_changed FINAL)

        /**
         * @brief Current maximized state derived from the native window state.
         *
         * This property exists to provide a stable maximized indicator for QML, especially for
         * frameless Windows windows where QML-facing state (e.g. visibility/windowState) may not
         * reflect the native state reliably.
         */
        Q_PROPERTY(bool isMaximizedNative READ get_is_maximized_native NOTIFY
                       is_maximized_native_changed FINAL)

    public:
        /**
         * @brief Constructs an AppWindow.
         *
         * @param parent Optional parent window.
         */
        explicit AppWindow(QWindow* parent = nullptr);

        /**
         * @brief Destroys the AppWindow.
         */
        ~AppWindow() override;

        /**
         * @brief Returns the QML item acting as the title bar.
         *
         * @return Pointer to the title bar item or nullptr.
         */
        [[nodiscard]] auto get_title_bar_item() const -> QQuickItem*;

        /**
         * @brief Sets the QML item acting as the title bar.
         *
         * @param item Pointer to the item.
         */
        auto set_title_bar_item(QQuickItem* item) -> void;

        /**
         * @brief Returns the QML item representing the minimize button hit-test region.
         *
         * The return value is used by the native hit-test implementation to determine whether
         * clicks should be handled by QML (HTCLIENT) or treated as caption drags.
         *
         * @return Pointer to the minimize button item or nullptr.
         */
        [[nodiscard]] auto get_minimize_button_item() const -> QQuickItem*;

        /**
         * @brief Sets the QML item representing the minimize button hit-test region.
         *
         * @param item Pointer to the item.
         */
        auto set_minimize_button_item(QQuickItem* item) -> void;

        /**
         * @brief Returns the QML item representing the maximize/restore button hit-test region.
         *
         * The return value is used by the native hit-test implementation to determine whether
         * clicks should be handled by QML (HTCLIENT) or treated as caption drags.
         *
         * @return Pointer to the maximize/restore button item or nullptr.
         */
        [[nodiscard]] auto get_maximize_button_item() const -> QQuickItem*;

        /**
         * @brief Sets the QML item representing the maximize/restore button hit-test region.
         *
         * @param item Pointer to the item.
         */
        auto set_maximize_button_item(QQuickItem* item) -> void;

        /**
         * @brief Returns the QML item representing the close button hit-test region.
         *
         * The return value is used by the native hit-test implementation to determine whether
         * clicks should be handled by QML (HTCLIENT) or treated as caption drags.
         *
         * @return Pointer to the close button item or nullptr.
         */
        [[nodiscard]] auto get_close_button_item() const -> QQuickItem*;

        /**
         * @brief Sets the QML item representing the close button hit-test region.
         *
         * @param item Pointer to the item.
         */
        auto set_close_button_item(QQuickItem* item) -> void;

        /**
         * @brief Returns the QML item representing an additional interactive title bar region.
         *
         * The return value is used by the native hit-test implementation to determine whether
         * clicks should be handled by QML (HTCLIENT) or treated as caption drags.
         *
         * @return Pointer to the custom interactive title bar item or nullptr.
         */
        [[nodiscard]] auto get_custom_title_bar_item() const -> QQuickItem*;

        /**
         * @brief Sets the QML item representing an additional interactive title bar region.
         *
         * @param item Pointer to the item.
         */
        auto set_custom_title_bar_item(QQuickItem* item) -> void;

        /**
         * @brief Query whether Mica-like backdrop is requested.
         *
         * @return bool True if Mica effect is requested, false otherwise.
         */
        [[nodiscard]] auto get_use_mica() const -> bool;

        /**
         * @brief Enable/disable Mica-like backdrop on Windows 11 at runtime.
         *
         * @param enabled True to enable Mica backdrop.
         */
        auto set_use_mica(bool enabled) -> void;

        /**
         * @brief Query whether rounded corners are requested.
         *
         * @return bool True if rounded corners are requested, false otherwise.
         */
        [[nodiscard]] auto get_use_rounded_corners() const -> bool;

        /**
         * @brief Enable/disable rounded corners on Windows 11 at runtime.
         *
         * @param enabled True to prefer rounded corners; false to request squared corners.
         */
        auto set_use_rounded_corners(bool enabled) -> void;

        /**
         * @brief Returns the current maximized state derived from the native window state.
         *
         * This property exists to provide a stable maximized indicator for QML, especially for
         * frameless Windows windows where QML-facing state (e.g. visibility/windowState) may not
         * reflect the native state reliably.
         *
         * @return bool True if the window is maximized natively.
         */
        [[nodiscard]] auto get_is_maximized_native() const -> bool;

        /**
         * @brief Start a system-managed window move operation.
         *
         * This is used by the Linux title bar implementation to delegate moving of the
         * frameless window back to the window manager/compositor.
         *
         * @return bool True if the system move operation was started successfully.
         */
        Q_INVOKABLE bool start_system_move();

        /**
         * @brief Start a system-managed window resize operation.
         *
         * This is used by the Linux frameless window implementation to delegate resize handling
         * back to the window manager/compositor.
         *
         * @param edges Window edges that should participate in the resize operation.
         * @return bool True if the system resize operation was started successfully.
         */
        Q_INVOKABLE bool start_system_resize(Qt::Edges edges);

    protected:
        /**
         * @brief Native event filter for Windows messages.
         *
         * Processes Windows messages required for the client-side frame (WM_NCCALCSIZE,
         * WM_NCHITTEST, WM_GETMINMAXINFO, WM_DPICHANGED, etc.).
         *
         * @param eventType Platform event type (unused on Windows).
         * @param message Pointer to the native MSG structure to process.
         * @param result Output pointer where a platform-specific result may be stored.
         * @return true if the event was handled; false otherwise.
         */
        bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

        /**
         * @brief Handle Qt events to keep isMaximizedNative in sync.
         *
         * This is required because frameless/native frame handling can desynchronize QML-observable
         * state from the actual native window state.
         *
         * @param event Event to process.
         * @return bool Result of QQuickWindow::event(event).
         */
        auto event(QEvent* event) -> bool override;

    private:
        /**
         * @brief Refresh cached isMaximizedNative state and emit change signal when needed.
         */
        auto refresh_is_maximized_native() -> void;

    signals:
        /**
         * @brief Emitted when the title bar item property changes.
         */
        void title_bar_item_changed();

        /**
         * @brief Emitted when the minimize button item property changes.
         */
        void minimize_button_item_changed();

        /**
         * @brief Emitted when the maximize button item property changes.
         */
        void maximize_button_item_changed();

        /**
         * @brief Emitted when the close button item property changes.
         */
        void close_button_item_changed();

        /**
         * @brief Emitted when the custom interactive title bar item property changes.
         */
        void custom_title_bar_item_changed();

        /**
         * @brief Emitted when the Mica preference changes.
         */
        void use_mica_changed();

        /**
         * @brief Emitted when the rounded corners preference changes.
         */
        void use_rounded_corners_changed();

        /**
         * @brief Emitted when the maximized state derived from the native window state changes.
         */
        void is_maximized_native_changed();

    private:
        friend class AppWindowLinux;
        friend class AppWindowWin;

        QQuickItem* m_title_bar_item = nullptr;
        QQuickItem* m_minimize_button_item = nullptr;
        QQuickItem* m_maximize_button_item = nullptr;
        QQuickItem* m_close_button_item = nullptr;
        QQuickItem* m_custom_title_bar_item = nullptr;

        bool m_use_mica = false;
        bool m_use_rounded_corners = true;

        /**
         * @brief Cached maximized state (native) to avoid emitting change signals redundantly.
         */
        bool m_is_maximized_native = false;

        std::unique_ptr<AppWindowLinux> m_app_window_linux;
        std::unique_ptr<AppWindowWin> m_app_window_win;
};

}  // namespace QtQuickCommonLib
