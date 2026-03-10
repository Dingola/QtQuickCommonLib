#pragma once

#include <QByteArray>
#include <QEvent>
#include <QQuickItem>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace QtQuickCommonLib
{

class AppWindow;

/**
 * @class AppWindowWin
 * @brief Windows-specific helper for AppWindow behavior.
 *
 * This helper encapsulates the Windows native window logic so AppWindow can keep the public API
 * stable while the platform implementation lives in a dedicated translation unit.
 */
class AppWindowWin
{
    public:
        /**
         * @brief Construct the Windows helper for a specific AppWindow instance.
         *
         * @param app_window Owning AppWindow instance.
         */
        explicit AppWindowWin(AppWindow* app_window);

        /**
         * @brief Apply the initial Windows-specific native window configuration.
         */
        auto initialize() -> void;

        /**
         * @brief Re-apply the Windows 11 specific window features for the owning AppWindow.
         */
        auto apply_win11_features() -> void;

        /**
         * @brief Returns the current maximized state derived from the native window state.
         *
         * @return bool True if the window is maximized natively.
         */
        [[nodiscard]] auto get_is_maximized_native() const -> bool;

        /**
         * @brief Start a system-managed move operation for the owning window.
         *
         * @return bool True if the operation was started successfully.
         */
        auto start_system_move() -> bool;

        /**
         * @brief Start a system-managed resize operation for the owning window.
         *
         * @param edges Window edges that should participate in the resize operation.
         * @return bool True if the operation was started successfully.
         */
        auto start_system_resize(Qt::Edges edges) -> bool;

        /**
         * @brief Handle Qt window-state changes required by the Windows frame implementation.
         *
         * @param old_window_state Previous window state from QWindowStateChangeEvent.
         */
        auto handle_window_state_change(Qt::WindowStates old_window_state) -> void;

        /**
         * @brief Native event handler for Windows messages used by the custom frame implementation.
         *
         * @param eventType Platform event type (unused on Windows).
         * @param message Pointer to a native MSG structure (MSG*).
         * @param result Output pointer where a platform-specific result (LRESULT) may be stored.
         * @return bool true when the message was handled by this function, false when the message
         *              should be forwarded to the base class implementation.
         */
        auto native_event(const QByteArray& eventType, void* message, qintptr* result) -> bool;

    private:
        /**
         * @brief Re-apply maximized state after a minimize/restore cycle when Qt restored the
         *        frameless window to the normal state.
         *
         * This helper queues a maximize request only when the window was maximized before
         * minimizing and the restored Qt/native state is no longer maximized.
         */
        auto restore_maximized_state_if_needed() -> void;

#ifdef Q_OS_WIN
        /**
         * @brief Return the native HWND for this window.
         *
         * @return HWND native window handle.
         */
        auto native_window_handle() const -> HWND;

        /**
         * @brief Returns the system resize border thickness in pixels (DPI-aware).
         *
         * Uses SM_CXSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
         *
         * @return int Total border width in pixels for the current DPI.
         */
        auto system_resize_border_width() const -> int;

        /**
         * @brief Returns the system resize border thickness in pixels (DPI-aware).
         *
         * Uses SM_CYSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
         *
         * @return int Total border height in pixels for the current DPI.
         */
        auto system_resize_border_height() const -> int;

        /**
         * @brief Returns the system caption (titlebar) height in pixels (DPI-aware).
         *
         * Uses SM_CYCAPTION scaled for the window's DPI.
         *
         * @return int Caption height in pixels for the current DPI.
         */
        auto system_caption_height() const -> int;

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
         * Applies rounded corners and optional Mica backdrop. Also attempts to enable immersive
         * dark mode for borders/caption where supported (best-effort).
         */
        auto enable_win11_features() -> void;

        /**
         * @brief Test whether a global point is over a specific QML item.
         *
         * The check is performed in the item's local coordinates.
         *
         * @param item Item to test (may be nullptr).
         * @param global_x Global x in screen coordinates.
         * @param global_y Global y in screen coordinates.
         * @return true when the point is inside the item bounds; false otherwise.
         */
        auto is_over_qml_item(const QQuickItem* item, int global_x, int global_y) const -> bool;

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
        auto is_over_qml_item_or_child(const QQuickItem* item, int global_x,
                                       int global_y) const -> bool;

        /**
         * @brief Perform custom hit-testing for frameless window.
         *
         * Returns HT* values for caption dragging, client controls and resize edges.
         *
         * @param msg Native MSG pointer containing screen coordinates in lParam.
         * @return LRESULT Hit-test code (HT*), e.g. HTCAPTION, HTCLIENT, HTLEFT, etc.
         */
        auto handle_nc_hit_test(MSG* msg) -> LRESULT;

        /**
         * @brief Handle WM_GETMINMAXINFO so maximize fits monitor work area (taskbar-aware).
         *
         * Sets ptMaxPosition and ptMaxSize based on the monitor work area. Also sets minimum
         * track size based on the QQuickWindow minimum size.
         *
         * @param msg Native MSG pointer containing LPMINMAXINFO in lParam.
         */
        auto handle_get_min_max_info(MSG* msg) -> void;
#endif

        AppWindow* m_app_window = nullptr;
        bool m_was_maximized_before_minimize = false;
        bool m_restore_maximized_queued = false;
};

}  // namespace QtQuickCommonLib