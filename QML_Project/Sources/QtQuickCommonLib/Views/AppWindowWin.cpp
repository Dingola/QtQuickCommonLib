#include "QtQuickCommonLib/Views/AppWindowWin.h"

#include <QMetaObject>

#include "QtQuickCommonLib/Views/AppWindow.h"

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

namespace
{
// ---- Cached WinAPI entry points and constants (runtime-gated) ----------------

/**
 * @brief Structure holding function pointers to DWM and related WinAPI functions.
 */
struct dwm_functions {
        using p_dwm_is_composition_enabled = HRESULT(WINAPI*)(BOOL*);
        using p_dwm_extend_frame_into_client_area = HRESULT(WINAPI*)(HWND, const MARGINS*);
        using p_dwm_set_window_attribute = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
        using p_get_dpi_for_window = UINT(WINAPI*)(HWND);
        using p_get_system_metrics_for_dpi = int(WINAPI*)(int, UINT);

        p_dwm_is_composition_enabled dwm_is_composition_enabled = nullptr;
        p_dwm_extend_frame_into_client_area dwm_extend_frame_into_client_area = nullptr;
        p_dwm_set_window_attribute dwm_set_window_attribute = nullptr;
        p_get_dpi_for_window get_dpi_for_window = nullptr;
        p_get_system_metrics_for_dpi get_system_metrics_for_dpi = nullptr;

        bool initialized = false;
};

/**
 * @brief Get cached DWM function pointers, resolving them on first call.
 *
 * @return dwm_functions& Reference to the static structure with function pointers.
 */
[[nodiscard]] static auto get_dwm_functions() -> dwm_functions&
{
    static dwm_functions f;

    if (!f.initialized)
    {
        HMODULE h_dwm = GetModuleHandleW(L"dwmapi");
        HMODULE h_user32 = GetModuleHandleW(L"user32");

        if (h_dwm)
        {
            f.dwm_is_composition_enabled =
                reinterpret_cast<dwm_functions::p_dwm_is_composition_enabled>(
                    GetProcAddress(h_dwm, "DwmIsCompositionEnabled"));
            f.dwm_extend_frame_into_client_area =
                reinterpret_cast<dwm_functions::p_dwm_extend_frame_into_client_area>(
                    GetProcAddress(h_dwm, "DwmExtendFrameIntoClientArea"));
            f.dwm_set_window_attribute =
                reinterpret_cast<dwm_functions::p_dwm_set_window_attribute>(
                    GetProcAddress(h_dwm, "DwmSetWindowAttribute"));
        }

        if (h_user32)
        {
            f.get_dpi_for_window = reinterpret_cast<dwm_functions::p_get_dpi_for_window>(
                GetProcAddress(h_user32, "GetDpiForWindow"));
            f.get_system_metrics_for_dpi =
                reinterpret_cast<dwm_functions::p_get_system_metrics_for_dpi>(
                    GetProcAddress(h_user32, "GetSystemMetricsForDpi"));
        }

        f.initialized = true;
    }

    return f;
}

// Attribute IDs: use unique names to avoid collisions with SDK's enum identifiers.
constexpr DWORD kDWMWA_WINDOW_CORNER_PREFERENCE = 33u;
constexpr DWORD kDWMWA_BORDER_COLOR = 34u;
constexpr DWORD kDWMWA_CAPTION_COLOR = 35u;
constexpr DWORD kDWMWA_SYSTEMBACKDROP_TYPE = 38u;
// Documented on Win10+ but not always present in headers
constexpr DWORD kDWMWA_USE_IMMERSIVE_DARK_MODE = 20u;
// Legacy Mica attribute used on early Windows 11 builds
constexpr DWORD kDWMWA_MICA_EFFECT = 1029u;

// Corner preference values
enum class dwm_window_corner_pref : DWORD
{
    default_pref = 0,
    do_not_round = 1,
    round = 2,
    round_small = 3
};

// System backdrop types (Windows 11)
enum class dwm_system_backdrop_type : DWORD
{
    auto_type = 0,
    none = 1,
    main_window = 2,
    transient_window = 3,
    tabbed_window = 4
};

/**
 * @brief Resolve GetDpiForWindow at runtime and return DPI for the given HWND.
 *
 * Keeps runtime resolution of GetDpiForWindow in one place; falls back to 96 DPI.
 *
 * @param hwnd Window handle to query DPI for.
 * @return UINT DPI value in dots per inch.
 */
[[nodiscard]] static auto get_window_dpi(HWND hwnd) -> UINT
{
    const auto& f = get_dwm_functions();
    UINT dpi_value = 96u;

    if (f.get_dpi_for_window)
    {
        dpi_value = f.get_dpi_for_window(hwnd);
    }

    return dpi_value;
}

/**
 * @brief Get a system metric in a DPI-aware way when supported.
 *
 * Calls GetSystemMetricsForDpi when available; otherwise falls back to GetSystemMetrics.
 *
 * @param index System metric index (SM_*).
 * @param dpi Dpi to query for.
 * @return int Metric value for the given DPI.
 */
[[nodiscard]] static auto get_system_metrics_dpi_aware(int index, UINT dpi) -> int
{
    const auto& f = get_dwm_functions();
    int value = 0;

    if (f.get_system_metrics_for_dpi)
    {
        value = f.get_system_metrics_for_dpi(index, dpi);
    }
    else
    {
        value = GetSystemMetrics(index);
    }

    return value;
}

}  // namespace
#endif  // Q_OS_WIN

namespace QtQuickCommonLib
{

/**
 * @brief Construct the Windows helper for a specific AppWindow instance.
 *
 * @param app_window Owning AppWindow instance.
 */
AppWindowWin::AppWindowWin(AppWindow* app_window): m_app_window(app_window) {}

/**
 * @brief Apply the initial Windows-specific native window configuration.
 */
auto AppWindowWin::initialize() -> void
{
#ifdef Q_OS_WIN
    enable_native_window_styles();
    extend_frame_into_client_area();
    enable_win11_features();
#endif
}

/**
 * @brief Re-apply the Windows 11 specific window features for the owning AppWindow.
 */
auto AppWindowWin::apply_win11_features() -> void
{
#ifdef Q_OS_WIN
    enable_win11_features();
#endif
}

/**
 * @brief Returns the current maximized state derived from the native window state.
 *
 * This property exists to provide a stable maximized indicator for QML, especially for frameless
 * Windows windows where QML-facing state (e.g. visibility/windowState) may not reflect the native
 * state reliably.
 * @return bool True if the window is maximized natively.
 */
auto AppWindowWin::get_is_maximized_native() const -> bool
{
    bool result = false;

#ifdef Q_OS_WIN
    HWND hwnd = native_window_handle();

    if (IsWindow(hwnd))
    {
        result = (IsZoomed(hwnd) != FALSE);
    }
#endif

    return result;
}

/**
 * @brief Start a system-managed move operation for the owning window.
 *
 * @return bool True if the operation was started successfully.
 */
auto AppWindowWin::start_system_move() -> bool
{
    bool result = false;

#ifdef Q_OS_WIN
    if (m_app_window != nullptr)
    {
        result = m_app_window->startSystemMove();
    }
#endif

    return result;
}

/**
 * @brief Start a system-managed resize operation for the owning window.
 *
 * @param edges Window edges that should participate in the resize operation.
 * @return bool True if the operation was started successfully.
 */
auto AppWindowWin::start_system_resize(Qt::Edges edges) -> bool
{
    bool result = false;

#ifdef Q_OS_WIN
    if (m_app_window != nullptr)
    {
        result = m_app_window->startSystemResize(edges);
    }
#else
    Q_UNUSED(edges);
#endif

    return result;
}

/**
 * @brief Re-apply maximized state after a minimize/restore cycle when Qt restored the
 *        frameless window to the normal state.
 *
 * This is a Windows-specific workaround for QQuickWindow state desynchronization when a
 * maximized frameless window is minimized to the taskbar and later restored.
 */
auto AppWindowWin::restore_maximized_state_if_needed() -> void
{
    bool should_queue_restore = false;

#ifdef Q_OS_WIN
    const Qt::WindowStates current_state = m_app_window->windowState();
    const bool is_minimized = (current_state & Qt::WindowMinimized) != 0;
    const bool is_maximized = (current_state & Qt::WindowMaximized) != 0;
    const bool native_is_maximized = get_is_maximized_native();

    if (m_was_maximized_before_minimize)
    {
        if (is_minimized)
        {
            m_restore_maximized_queued = false;
        }
        else if (is_maximized || native_is_maximized)
        {
            m_was_maximized_before_minimize = false;
            m_restore_maximized_queued = false;
        }
        else if (!m_restore_maximized_queued)
        {
            m_restore_maximized_queued = true;
            should_queue_restore = true;
        }
    }
    else
    {
        m_restore_maximized_queued = false;
    }

    if (should_queue_restore)
    {
        QMetaObject::invokeMethod(
            m_app_window,
            [this]() {
                if (m_was_maximized_before_minimize)
                {
                    m_app_window->showMaximized();
                }

                m_restore_maximized_queued = false;
                m_app_window->refresh_is_maximized_native();

                if (get_is_maximized_native())
                {
                    m_was_maximized_before_minimize = false;
                }
            },
            Qt::QueuedConnection);
    }
#endif

    m_app_window->refresh_is_maximized_native();
}

/**
 * @brief Handle Qt window-state changes required by the Windows frame implementation.
 *
 * @param old_window_state Previous window state from QWindowStateChangeEvent.
 */
auto AppWindowWin::handle_window_state_change(Qt::WindowStates old_window_state) -> void
{
#ifdef Q_OS_WIN
    const Qt::WindowStates current_window_state = m_app_window->windowState();
    const bool old_was_minimized = (old_window_state & Qt::WindowMinimized) != 0;
    const bool old_was_maximized = (old_window_state & Qt::WindowMaximized) != 0;
    const bool current_is_minimized = (current_window_state & Qt::WindowMinimized) != 0;
    const bool current_is_maximized = (current_window_state & Qt::WindowMaximized) != 0;

    if (current_is_minimized)
    {
        m_was_maximized_before_minimize = old_was_maximized;
        m_restore_maximized_queued = false;
    }
    else if (old_was_minimized)
    {
        if (m_was_maximized_before_minimize && !current_is_maximized)
        {
            restore_maximized_state_if_needed();
        }
        else
        {
            m_was_maximized_before_minimize = false;
            m_restore_maximized_queued = false;
        }
    }
#endif
}

/**
 * @brief Native event handler for Windows messages used by the custom frame implementation.
 *
 * Handles WM_NCCALCSIZE, WM_NCHITTEST, WM_GETMINMAXINFO, WM_DPICHANGED and a few others.
 *
 * Uses a single return at the end and avoids early returns to conform to project style.
 *
 * @param eventType Platform event type (unused on Windows).
 * @param message Pointer to a native MSG structure (MSG*).
 * @param result Output pointer where a platform-specific result (LRESULT) may be stored.
 * @return bool true when the message was handled by this function, false when the message should
 *              be forwarded to the base class implementation.
 */
auto AppWindowWin::native_event(const QByteArray& eventType, void* message, qintptr* result) -> bool
{
    bool final_result = false;

#ifdef Q_OS_WIN
    Q_UNUSED(eventType);

    MSG* msg = static_cast<MSG*>(message);

    bool handled = false;
    LRESULT out_result = 0;

    if (msg->message == WM_NCCALCSIZE)
    {
        const bool platform_ready = (m_app_window->handle() != nullptr);

        if (platform_ready && msg->wParam == TRUE)
        {
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);

            if (IsZoomed(native_window_handle()))
            {
                const int border_x = system_resize_border_width();
                const int border_y = system_resize_border_height();

                params->rgrc[0].top += border_y;

                params->rgrc[0].left += border_x;
                params->rgrc[0].right -= border_x;
            }
        }

        out_result = 0;
        handled = true;
    }
    else if (msg->message == WM_NCHITTEST)
    {
        out_result = handle_nc_hit_test(msg);
        handled = true;
    }
    else if (msg->message == WM_GETMINMAXINFO)
    {
        handle_get_min_max_info(msg);
        handled = false;  // allow DefWindowProc to process it as well
    }
    else if (msg->message == WM_SYSCOMMAND)
    {
        const WPARAM command = (msg->wParam & 0xFFF0u);

        if (command == SC_MINIMIZE)
        {
            const bool native_is_maximized = get_is_maximized_native();
            m_was_maximized_before_minimize = native_is_maximized;
            m_restore_maximized_queued = false;

            if (native_is_maximized)
            {
                WINDOWPLACEMENT placement{};
                placement.length = sizeof(placement);

                if (GetWindowPlacement(native_window_handle(), &placement))
                {
                    placement.flags |= WPF_RESTORETOMAXIMIZED;
                    SetWindowPlacement(native_window_handle(), &placement);
                }
            }
        }

        handled = false;
    }
    else if (msg->message == WM_SIZE || msg->message == WM_WINDOWPOSCHANGED)
    {
        if (msg->message == WM_SIZE)
        {
            if (msg->wParam == SIZE_MINIMIZED)
            {
                m_was_maximized_before_minimize =
                    m_was_maximized_before_minimize || m_app_window->m_is_maximized_native;
                m_restore_maximized_queued = false;
            }
            else if (msg->wParam == SIZE_MAXIMIZED)
            {
                m_was_maximized_before_minimize = false;
                m_restore_maximized_queued = false;
            }
            else if (msg->wParam == SIZE_RESTORED)
            {
                restore_maximized_state_if_needed();
            }
        }

        m_app_window->refresh_is_maximized_native();
        handled = false;
    }
    else if (msg->message == WM_DWMCOMPOSITIONCHANGED || msg->message == WM_DPICHANGED)
    {
        if (msg->message == WM_DPICHANGED)
        {
            // On DPI changes Windows suggests a new window rect; apply it so scaling/edges match.
            RECT* const prc_new_window = reinterpret_cast<RECT*>(msg->lParam);

            if (prc_new_window != nullptr)
            {
                SetWindowPos(native_window_handle(), nullptr, prc_new_window->left,
                             prc_new_window->top, prc_new_window->right - prc_new_window->left,
                             prc_new_window->bottom - prc_new_window->top,
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }

        // Also reapply frame margins and Win11 attributes
        extend_frame_into_client_area();
        enable_win11_features();
        m_app_window->refresh_is_maximized_native();
        handled = false;
    }
    else if (msg->message == WM_NCACTIVATE)
    {
        // Prevent Windows from drawing the standard non-client area when activation changes.
        out_result = 1;
        handled = true;
    }
    else if (msg->message == WM_ERASEBKGND)
    {
        // Let Qt handle painting; claim the message handled to reduce flicker.
        out_result = 1;
        handled = true;
    }

    if (handled)
    {
        if (result != nullptr)
        {
            *result = out_result;
        }
        final_result = true;
    }
    else
    {
        final_result = m_app_window->QQuickWindow::nativeEvent(eventType, message, result);
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
#endif

    return final_result;
}

#ifdef Q_OS_WIN

/**
 * @brief Return the native HWND for this window.
 *
 * @return HWND native window handle.
 */
auto AppWindowWin::native_window_handle() const -> HWND
{
    HWND result = reinterpret_cast<HWND>(m_app_window->winId());
    return result;
}

/**
 * @brief Returns the system resize border thickness in pixels (DPI-aware).
 *
 * Uses SM_CXSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
 *
 * @return int Total border width in pixels for the current DPI.
 */
auto AppWindowWin::system_resize_border_width() const -> int
{
    HWND hwnd = native_window_handle();
    const UINT dpi = get_window_dpi(hwnd);

    const int cx_size_frame = get_system_metrics_dpi_aware(SM_CXSIZEFRAME, dpi);
    const int cx_padded_border = get_system_metrics_dpi_aware(SM_CXPADDEDBORDER, dpi);

    const int result = cx_size_frame + cx_padded_border;
    return result;
}

/**
 * @brief Returns the system resize border thickness in pixels (DPI-aware).
 *
 * Uses SM_CYSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
 *
 * @return int Total border height in pixels for the current DPI.
 */
auto AppWindowWin::system_resize_border_height() const -> int
{
    HWND hwnd = native_window_handle();
    const UINT dpi = get_window_dpi(hwnd);

    const int cy_size_frame = get_system_metrics_dpi_aware(SM_CYSIZEFRAME, dpi);
    const int cx_padded_border = get_system_metrics_dpi_aware(SM_CXPADDEDBORDER, dpi);

    const int result = cy_size_frame + cx_padded_border;
    return result;
}

/**
 * @brief Returns the system caption (titlebar) height in pixels (DPI-aware).
 *
 * Uses SM_CYCAPTION scaled for the window's DPI.
 *
 * @return int Caption height in pixels for the current DPI.
 */
auto AppWindowWin::system_caption_height() const -> int
{
    HWND hwnd = native_window_handle();
    const UINT dpi = get_window_dpi(hwnd);

    const int result = get_system_metrics_dpi_aware(SM_CYCAPTION, dpi);
    return result;
}

/**
 * @brief Enable required native window styles for snap and resize behavior.
 */
auto AppWindowWin::enable_native_window_styles() -> void
{
    HWND hwnd = native_window_handle();

    if (IsWindow(hwnd))
    {
        LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);

        style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION;
        style &= ~WS_POPUP;

        SetWindowLongPtrW(hwnd, GWL_STYLE, style);

        // Ensure extended styles include a standard edge look (consistent fallback shadow).
        LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        ex |= WS_EX_WINDOWEDGE;
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, ex);

        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

/**
 * @brief Extend DWM frame into client area to avoid white borders.
 *
 * When DWM composition is available, extends a 1px margin to avoid white border artifacts.
 * Otherwise applies a drop-shadow via class style as a best-effort fallback.
 */
auto AppWindowWin::extend_frame_into_client_area() -> void
{
    HWND hwnd = native_window_handle();

    if (IsWindow(hwnd))
    {
        const auto& f = get_dwm_functions();

        // Check whether DWM (Desktop Window Manager) composition is enabled.
        BOOL composition_enabled = FALSE;

        if (f.dwm_is_composition_enabled)
        {
            f.dwm_is_composition_enabled(&composition_enabled);
        }

        if (composition_enabled && f.dwm_extend_frame_into_client_area)
        {
            // Extend a tiny bit into the client area (1px on all sides) to remove white border
            // artifacts.
            MARGINS margins = {1, 1, 1, 1};
            f.dwm_extend_frame_into_client_area(hwnd, &margins);
        }
        else
        {
            // Fallback for systems without DWM (classic theme, remote desktop, etc.)
            // Enable a basic drop shadow to mimic Aero appearance.
            const LONG_PTR class_style = GetClassLongPtrW(hwnd, GCL_STYLE);
            SetClassLongPtrW(hwnd, GCL_STYLE, class_style | CS_DROPSHADOW);

            // Force a style refresh so the shadow is applied immediately.
            SetWindowPos(
                hwnd, nullptr, 0, 0, 0, 0,
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
        }
    }
}

/**
 * @brief Enable Windows 11 specific DWM features if available.
 *
 * Applies rounded corners and optional Mica backdrop. Also attempts to enable immersive dark mode
 * for borders/caption where supported (best-effort).
 */
auto AppWindowWin::enable_win11_features() -> void
{
    HWND hwnd = native_window_handle();

    if (IsWindow(hwnd))
    {
        const auto& f = get_dwm_functions();

        // Corner preference (Windows 11)
        if (f.dwm_set_window_attribute)
        {
            DWORD corner_pref = m_app_window->m_use_rounded_corners
                                    ? static_cast<DWORD>(dwm_window_corner_pref::round)
                                    : static_cast<DWORD>(dwm_window_corner_pref::do_not_round);

            f.dwm_set_window_attribute(hwnd, kDWMWA_WINDOW_CORNER_PREFERENCE, &corner_pref,
                                       sizeof(corner_pref));
        }

        // Try immersive dark mode (Win10+ attribute, not always available)
        if (f.dwm_set_window_attribute)
        {
            BOOL enable_dark = TRUE;

            HRESULT hr = f.dwm_set_window_attribute(hwnd, kDWMWA_USE_IMMERSIVE_DARK_MODE,
                                                    &enable_dark, sizeof(enable_dark));

            if (FAILED(hr))
            {
                // Fallback to a darker border/caption color when supported
                COLORREF border_color = RGB(30, 30, 30);
                f.dwm_set_window_attribute(hwnd, kDWMWA_BORDER_COLOR, &border_color,
                                           sizeof(border_color));

                COLORREF caption_color = RGB(30, 30, 30);
                f.dwm_set_window_attribute(hwnd, kDWMWA_CAPTION_COLOR, &caption_color,
                                           sizeof(caption_color));
            }
        }

        // Mica/system backdrop (Windows 11)
        if (f.dwm_set_window_attribute)
        {
            if (m_app_window->m_use_mica)
            {
                // Prefer the modern backdrop attribute
                DWORD backdrop = static_cast<DWORD>(dwm_system_backdrop_type::main_window);
                HRESULT hr = f.dwm_set_window_attribute(hwnd, kDWMWA_SYSTEMBACKDROP_TYPE, &backdrop,
                                                        sizeof(backdrop));

                if (FAILED(hr))
                {
                    // Fallback to legacy Mica attribute if present
                    BOOL enable_mica = TRUE;
                    f.dwm_set_window_attribute(hwnd, kDWMWA_MICA_EFFECT, &enable_mica,
                                               sizeof(enable_mica));
                }
            }
            else
            {
                // Disable Mica/backdrop when user preference is off
                DWORD backdrop_none = static_cast<DWORD>(dwm_system_backdrop_type::none);
                HRESULT hr = f.dwm_set_window_attribute(hwnd, kDWMWA_SYSTEMBACKDROP_TYPE,
                                                        &backdrop_none, sizeof(backdrop_none));

                if (FAILED(hr))
                {
                    BOOL disable_mica = FALSE;
                    f.dwm_set_window_attribute(hwnd, kDWMWA_MICA_EFFECT, &disable_mica,
                                               sizeof(disable_mica));
                }
            }
        }
    }
}

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
auto AppWindowWin::is_over_qml_item(const QQuickItem* item, int global_x,
                                    int global_y) const -> bool
{
    bool is_over = false;

    if (item != nullptr)
    {
        const QPointF scene_pos = m_app_window->mapFromGlobal(QPointF(global_x, global_y));
        const QPointF local_pos = item->mapFromScene(scene_pos);
        is_over = item->contains(local_pos);
    }

    return is_over;
}

/**
 * @brief Test whether a global point is over a specific QML item or one of its children.
 *
 * This is used by WM_NCHITTEST for interactive title bar containers whose child items
 * should remain clickable while whitespace outside those child items stays draggable.
 *
 * @param item Root item to test (may be nullptr).
 * @param global_x Global x in screen coordinates.
 * @param global_y Global y in screen coordinates.
 * @return true when the point is inside the item or one of its child items; false otherwise.
 */
auto AppWindowWin::is_over_qml_item_or_child(const QQuickItem* item, int global_x,
                                             int global_y) const -> bool
{
    bool is_over = false;

    if (item != nullptr)
    {
        if (is_over_qml_item(item, global_x, global_y))
        {
            is_over = true;
        }
        else
        {
            const QList<QQuickItem*> child_items = item->childItems();

            for (qsizetype i = 0; i < child_items.size() && !is_over; ++i)
            {
                const QQuickItem* child_item = child_items.at(i);

                if (child_item != nullptr && child_item->isVisible())
                {
                    is_over = is_over_qml_item_or_child(child_item, global_x, global_y);
                }
            }
        }
    }

    return is_over;
}

/**
 * @brief Perform custom hit-testing for frameless window.
 *
 * Returns HT* values for caption dragging, client controls and resize edges.
 *
 * Notes:
 *  - To allow resizing near the edges while using a custom title bar, points that fall
 *    inside the system resize border are not treated as caption.
 *  - When the window is maximized, resize zones are ignored so that clicks at the very
 *    outermost pixels of the titlebar behave like a restore+drag (native behavior).
 *
 * @param msg Native MSG pointer containing screen coordinates in lParam.
 * @return LRESULT Hit-test code (HT*), e.g. HTCAPTION, HTCLIENT, HTLEFT, etc.
 */
auto AppWindowWin::handle_nc_hit_test(MSG* msg) -> LRESULT
{
    const int x = GET_X_LPARAM(msg->lParam);
    const int y = GET_Y_LPARAM(msg->lParam);

    RECT wr{};
    GetWindowRect(native_window_handle(), &wr);

    const int left = wr.left;
    const int top = wr.top;
    const int right = wr.right;
    const int bottom = wr.bottom;

    const int cx = x - left;
    const int cy = y - top;
    const int w = right - left;
    const int h = bottom - top;

    // Border thickness (DPI aware)
    const int border = system_resize_border_width();
    const int caption = (m_app_window->m_title_bar_item != nullptr)
                            ? static_cast<int>(m_app_window->m_title_bar_item->height())
                            : system_caption_height();

    // Compute edge booleans early so titlebar logic can respect resize zones.
    bool on_left = cx < border;
    bool on_right = cx >= w - border;
    bool on_top = cy < border;
    bool on_bottom = cy >= h - border;

    // If the window is maximized, ignore the resize zones so clicks at the outermost
    // pixels of the titlebar behave like titlebar drags (restore + move) instead of resize.
    if (IsZoomed(native_window_handle()))
    {
        on_left = false;
        on_right = false;
        on_top = false;
        on_bottom = false;
    }

    LRESULT hit = HTCLIENT;
    bool over_interactive_item = false;

    // First check titlebar buttons and custom interactive items: if the global point is over
    // one of them, treat it as client.
    const bool over_button = is_over_qml_item(m_app_window->m_minimize_button_item, x, y) ||
                             is_over_qml_item(m_app_window->m_maximize_button_item, x, y) ||
                             is_over_qml_item(m_app_window->m_close_button_item, x, y);

    const bool over_custom_item =
        is_over_qml_item_or_child(m_app_window->m_custom_title_bar_item, x, y);

    if (over_button || over_custom_item)
    {
        over_interactive_item = true;
        hit = HTCLIENT;
    }

    // If inside the titlebar area (but not interactive items) -> allow dragging
    // Only set HTCAPTION when not over an interactive item.
    if (m_app_window->m_title_bar_item != nullptr && !over_interactive_item)
    {
        if (is_over_qml_item(m_app_window->m_title_bar_item, x, y))
        {
            // If the point lies inside the system resize border, allow the resize handling
            // below to run by not returning HTCAPTION here.
            if (!(on_top || on_left || on_right || on_bottom))
            {
                hit = HTCAPTION;
            }
            // else: fall through so edge/corner checks below can set resize values
        }
    }
    else if (m_app_window->m_title_bar_item == nullptr && !over_interactive_item)
    {
        // Fallback: allow top area as caption when no custom titlebar is present
        if (cy >= 0 && cy < caption)
        {
            hit = HTCAPTION;
        }
    }

    // Now handle resize zones (corners + edges) if caption wasn't set
    if (hit != HTCAPTION)
    {
        if (on_left && on_top)
        {
            hit = HTTOPLEFT;
        }
        else if (on_right && on_top)
        {
            hit = HTTOPRIGHT;
        }
        else if (on_left && on_bottom)
        {
            hit = HTBOTTOMLEFT;
        }
        else if (on_right && on_bottom)
        {
            hit = HTBOTTOMRIGHT;
        }
        else if (on_top)
        {
            hit = HTTOP;
        }
        else if (on_bottom)
        {
            hit = HTBOTTOM;
        }
        else if (on_left)
        {
            hit = HTLEFT;
        }
        else if (on_right)
        {
            hit = HTRIGHT;
        }
        else
        {
            hit = HTCLIENT;
        }
    }

    return hit;
}

/**
 * @brief Handle WM_GETMINMAXINFO so maximize fits monitor work area (taskbar-aware).
 *
 * Sets ptMaxPosition and ptMaxSize based on the monitor work area. Also sets minimum track
 * size based on the QQuickWindow minimum size.
 *
 * @param msg Native MSG pointer containing LPMINMAXINFO in lParam.
 */
auto AppWindowWin::handle_get_min_max_info(MSG* msg) -> void
{
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);

    HMONITOR mon = MonitorFromWindow(native_window_handle(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};

    if (GetMonitorInfoW(mon, &mi))
    {
        mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
        mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
        mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
        mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;

        const int min_width = static_cast<int>(m_app_window->minimumWidth());
        const int min_height = static_cast<int>(m_app_window->minimumHeight());

        mmi->ptMinTrackSize.x = (min_width > 0) ? min_width : 0;
        mmi->ptMinTrackSize.y = (min_height > 0) ? min_height : 0;
    }
}

#endif  // Q_OS_WIN

}  // namespace QtQuickCommonLib