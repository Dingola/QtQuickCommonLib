#pragma once

#include <QEvent>
#include <QQuickItem>
#include <QQuickWindow>

#include "QtQuickCommonLib/ApiMacro.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace QtQuickCommonLib
{

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

    private:  // private methods (Windows-only)
#ifdef Q_OS_WIN
        /**
         * @brief Return the native HWND for this window.
         *
         * Uses winId() and casts to HWND. Callers must ensure the native window exists
         * (create()) before using the returned handle.
         *
         * @return HWND native window handle, or nullptr if not created.
         */
        [[nodiscard]] auto native_window_handle() const -> HWND;

        /**
         * @brief Returns the system resize border thickness in pixels (DPI-aware).
         *
         * Uses SM_CXSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
         *
         * @return int Total border width in pixels for the current DPI.
         */
        [[nodiscard]] auto system_resize_border_width() const -> int;

        /**
         * @brief Returns the system resize border thickness in pixels (DPI-aware).
         *
         * Uses SM_CYSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
         *
         * @return int Total border height in pixels for the current DPI.
         */
        [[nodiscard]] auto system_resize_border_height() const -> int;

        /**
         * @brief Returns the system caption (titlebar) height in pixels (DPI-aware).
         *
         * Uses SM_CYCAPTION scaled for the window's DPI.
         *
         * @return int Caption height in pixels for the current DPI.
         */
        [[nodiscard]] auto system_caption_height() const -> int;

        /**
         * @brief Enable required native window styles for snap and resize behavior.
         */
        auto enable_native_window_styles() -> void;

        /**
         * @brief Extend DWM frame into client area to avoid white borders.
         *
         * When DWM composition is available, extends a 1px margin to avoid white border artifacts.
         * Otherwise applies a drop-shadow via class style as a best-effort fallback.
         */
        auto extend_frame_into_client_area() -> void;

        /**
         * @brief Enable Windows 11 specific DWM features if available.
         *
         * Applies rounded corners and optional Mica backdrop, gated by runtime availability and
         * user preferences set via set_use_mica() and set_use_rounded_corners().
         * Also attempts to enable immersive dark mode for borders/caption where supported.
         */
        auto enable_win11_features() -> void;

        /**
         * @brief Perform custom hit-testing for frameless window.
         *
         * @param msg Native MSG pointer.
         * @return LRESULT Hit-test code.
         */
        auto handle_nc_hit_test(MSG* msg) -> LRESULT;

        /**
         * @brief Handle WM_GETMINMAXINFO so maximize fits monitor work area.
         *
         * Also applies the current minimum tracking size derived from the QQuickWindow minimum
         * size properties.
         *
         * @param msg Native MSG pointer.
         */
        auto handle_get_min_max_info(MSG* msg) -> void;

        /**
         * @brief Test whether a global point is over a specific QML item.
         *
         * This is used by WM_NCHITTEST to reliably distinguish interactive controls
         * from draggable caption areas.
         *
         * @param item Item to test (may be nullptr).
         * @param global_x Global x in screen coordinates.
         * @param global_y Global y in screen coordinates.
         * @return true when the point is inside the item bounds; false otherwise.
         */
        [[nodiscard]] auto is_over_qml_item(const QQuickItem* item, int global_x,
                                            int global_y) const -> bool;

        /**
         * @brief Test whether a global point is over a specific QML item or one of its children.
         *
         * This is used by WM_NCHITTEST for interactive title bar containers whose child items
         * should remain clickable while whitespace outside those child items stays draggable.
         *
         * @param item Root item to test (may be nullptr).
         * @param global_x Global x in screen coordinates.
         * @param global_y Global y in screen coordinates.
         * @return true when the point is inside the item or one of its child items; false
         * otherwise.
         */
        [[nodiscard]] auto is_over_qml_item_or_child(const QQuickItem* item, int global_x,
                                                     int global_y) const -> bool;
#endif

        /**
         * @brief Refresh cached isMaximizedNative state and emit change signal when needed.
         */
        auto refresh_is_maximized_native() -> void;

        /**
         * @brief Re-apply maximized state after a minimize/restore cycle when Qt restored the
         *        frameless window to the normal state.
         *
         * This helper queues a maximize request only when the window was maximized before
         * minimizing and the restored Qt/native state is no longer maximized.
         */
        auto restore_maximized_state_if_needed() -> void;

    private:
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

        /**
         * @brief Cached state used to remember whether the window was maximized before minimizing.
         */
        bool m_was_maximized_before_minimize = false;

        /**
         * @brief Guard flag preventing multiple queued maximize restores for the same cycle.
         */
        bool m_restore_maximized_queued = false;
};

}  // namespace QtQuickCommonLib
