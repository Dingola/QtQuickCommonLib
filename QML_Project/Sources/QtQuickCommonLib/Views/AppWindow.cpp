#include "QtQuickCommonLib/Views/AppWindow.h"

#include <QEvent>
#include <QQuickItem>

#include "QtQuickCommonLib/Views/AppWindowLinux.h"
#include "QtQuickCommonLib/Views/AppWindowWin.h"

namespace QtQuickCommonLib
{

/**
 * @brief Constructs an AppWindow.
 *
 * The AppWindow is a top-level native window with a client-side title bar logic.
 * It removes the standard OS frame but keeps native behaviors (Snap, Resize, etc.).
 *
 * @param parent Optional parent window.
 */
AppWindow::AppWindow(QWindow* parent): QQuickWindow(parent)
{
    // Remove standard frame but keep window flag.
    // We reinstate the behavior manually via WinAPI later.
    setFlags(Qt::Window | Qt::FramelessWindowHint);

#ifdef Q_OS_LINUX
    m_app_window_linux = std::make_unique<AppWindowLinux>(this);
#endif

#ifdef Q_OS_WIN
    m_app_window_win = std::make_unique<AppWindowWin>(this);

    // Ensure native window is created immediately to apply styles.
    if (!winId())
    {
        create();
    }

    m_app_window_win->initialize();
#endif

    refresh_is_maximized_native();
}

/**
 * @brief Destroys the AppWindow.
 */
AppWindow::~AppWindow() = default;

/**
 * @brief Returns the QML item acting as the title bar.
 * @return Pointer to the title bar item or nullptr.
 */
auto AppWindow::get_title_bar_item() const -> QQuickItem*
{
    QQuickItem* result = m_title_bar_item;
    return result;
}

/**
 * @brief Sets the QML item acting as the title bar.
 * @param item Pointer to the item.
 */
auto AppWindow::set_title_bar_item(QQuickItem* item) -> void
{
    if (m_title_bar_item != item)
    {
        m_title_bar_item = item;
        emit title_bar_item_changed();
    }
}

/**
 * @brief Returns the QML item representing the minimize button hit-test region.
 * @return Pointer to the minimize button item or nullptr.
 */
auto AppWindow::get_minimize_button_item() const -> QQuickItem*
{
    QQuickItem* result = m_minimize_button_item;
    return result;
}

/**
 * @brief Sets the QML item representing the minimize button hit-test region.
 * @param item Pointer to the item.
 */
auto AppWindow::set_minimize_button_item(QQuickItem* item) -> void
{
    if (m_minimize_button_item != item)
    {
        m_minimize_button_item = item;
        emit minimize_button_item_changed();
    }
}

/**
 * @brief Returns the QML item representing the maximize/restore button hit-test region.
 * @return Pointer to the maximize/restore button item or nullptr.
 */
auto AppWindow::get_maximize_button_item() const -> QQuickItem*
{
    QQuickItem* result = m_maximize_button_item;
    return result;
}

/**
 * @brief Sets the QML item representing the maximize/restore button hit-test region.
 * @param item Pointer to the item.
 */
auto AppWindow::set_maximize_button_item(QQuickItem* item) -> void
{
    if (m_maximize_button_item != item)
    {
        m_maximize_button_item = item;
        emit maximize_button_item_changed();
    }
}

/**
 * @brief Returns the QML item representing the close button hit-test region.
 * @return Pointer to the close button item or nullptr.
 */
auto AppWindow::get_close_button_item() const -> QQuickItem*
{
    QQuickItem* result = m_close_button_item;
    return result;
}

/**
 * @brief Sets the QML item representing the close button hit-test region.
 * @param item Pointer to the item.
 */
auto AppWindow::set_close_button_item(QQuickItem* item) -> void
{
    if (m_close_button_item != item)
    {
        m_close_button_item = item;
        emit close_button_item_changed();
    }
}

/**
 * @brief Returns the QML item representing an additional interactive title bar region.
 * @return Pointer to the custom interactive title bar item or nullptr.
 */
auto AppWindow::get_custom_title_bar_item() const -> QQuickItem*
{
    QQuickItem* result = m_custom_title_bar_item;
    return result;
}

/**
 * @brief Sets the QML item representing an additional interactive title bar region.
 * @param item Pointer to the item.
 */
auto AppWindow::set_custom_title_bar_item(QQuickItem* item) -> void
{
    if (m_custom_title_bar_item != item)
    {
        m_custom_title_bar_item = item;
        emit custom_title_bar_item_changed();
    }
}

/**
 * @brief Query whether Mica-like backdrop is requested.
 * @return bool True if Mica effect is requested.
 */
auto AppWindow::get_use_mica() const -> bool
{
    bool result = m_use_mica;
    return result;
}

/**
 * @brief Enable/disable Mica-like backdrop on Windows 11 at runtime.
 *
 * @param enabled True to enable Mica backdrop.
 */
auto AppWindow::set_use_mica(bool enabled) -> void
{
    if (m_use_mica != enabled)
    {
        m_use_mica = enabled;
#ifdef Q_OS_WIN
        if (m_app_window_win != nullptr)
        {
            m_app_window_win->apply_win11_features();
        }
#endif
        emit use_mica_changed();
    }
}

/**
 * @brief Query whether rounded corners are requested.
 * @return bool True if rounded corners are requested.
 */
auto AppWindow::get_use_rounded_corners() const -> bool
{
    bool result = m_use_rounded_corners;
    return result;
}

/**
 * @brief Enable/disable rounded corners on Windows 11 at runtime.
 *
 * @param enabled True to prefer rounded corners.
 */
auto AppWindow::set_use_rounded_corners(bool enabled) -> void
{
    if (m_use_rounded_corners != enabled)
    {
        m_use_rounded_corners = enabled;
#ifdef Q_OS_WIN
        if (m_app_window_win != nullptr)
        {
            m_app_window_win->apply_win11_features();
        }
#endif
        emit use_rounded_corners_changed();
    }
}

/**
 * @brief Returns the current maximized state derived from the native window state.
 *
 * This property exists to provide a stable maximized indicator for QML, especially for frameless
 * Windows windows where QML-facing state (e.g. visibility/windowState) may not reflect the native
 * state reliably.
 * @return bool True if the window is maximized natively.
 */
[[nodiscard]] auto AppWindow::get_is_maximized_native() const -> bool
{
    bool result = false;

#ifdef Q_OS_WIN
    if (m_app_window_win != nullptr)
    {
        result = m_app_window_win->get_is_maximized_native();
    }
#elif defined(Q_OS_LINUX)
    if (m_app_window_linux != nullptr)
    {
        result = m_app_window_linux->get_is_maximized_native();
    }
#else
    result = (windowState() & Qt::WindowMaximized) != 0;
#endif

    return result;
}

/**
 * @brief Start a system-managed window move operation.
 *
 * This is used by the Linux title bar implementation to delegate moving of the
 * frameless window back to the window manager/compositor.
 *
 * @return bool True if the system move operation was started successfully.
 */
bool AppWindow::start_system_move()
{
    bool result = false;

#ifdef Q_OS_WIN
    if (m_app_window_win != nullptr)
    {
        result = m_app_window_win->start_system_move();
    }
#elif defined(Q_OS_LINUX)
    if (m_app_window_linux != nullptr)
    {
        result = m_app_window_linux->start_system_move();
    }
#endif

    return result;
}

/**
 * @brief Start a system-managed window resize operation.
 *
 * This is used by the Linux frameless window implementation to delegate resize handling
 * back to the window manager/compositor.
 *
 * @param edges Window edges that should participate in the resize operation.
 * @return bool True if the system resize operation was started successfully.
 */
bool AppWindow::start_system_resize(Qt::Edges edges)
{
    bool result = false;

#ifdef Q_OS_WIN
    if (m_app_window_win != nullptr)
    {
        result = m_app_window_win->start_system_resize(edges);
    }
#elif defined(Q_OS_LINUX)
    if (m_app_window_linux != nullptr)
    {
        result = m_app_window_linux->start_system_resize(edges);
    }
#else
    Q_UNUSED(edges);
#endif

    return result;
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
auto AppWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) -> bool
{
    bool final_result = false;

#ifdef Q_OS_WIN
    if (m_app_window_win != nullptr)
    {
        final_result = m_app_window_win->native_event(eventType, message, result);
    }
    else
    {
        final_result = QQuickWindow::nativeEvent(eventType, message, result);
    }
#else
    final_result = QQuickWindow::nativeEvent(eventType, message, result);
#endif

    return final_result;
}

/**
 * @brief Handle Qt events to keep isMaximizedNative in sync.
 *
 * This is required because frameless/native frame handling can desynchronize QML-observable
 * state from the actual native window state.
 *
 * @param event Event to process.
 * @return bool Result of QQuickWindow::event(event).
 */
auto AppWindow::event(QEvent* event) -> bool
{
    bool is_window_state_change = false;
    bool is_platform_surface = false;
    Qt::WindowStates old_window_state = Qt::WindowNoState;

    if (event != nullptr)
    {
        is_window_state_change = (event->type() == QEvent::WindowStateChange);
        is_platform_surface = (event->type() == QEvent::PlatformSurface);

        if (is_window_state_change)
        {
            auto* state_change_event = static_cast<QWindowStateChangeEvent*>(event);
            old_window_state = state_change_event->oldState();
        }
    }

    bool result = QQuickWindow::event(event);

#ifdef Q_OS_WIN
    if (is_window_state_change && m_app_window_win != nullptr)
    {
        m_app_window_win->handle_window_state_change(old_window_state);
    }
#endif

    if (is_window_state_change || is_platform_surface)
    {
        refresh_is_maximized_native();
    }

    return result;
}

/**
 * @brief Refresh cached isMaximizedNative state and emit change signal when needed.
 */
auto AppWindow::refresh_is_maximized_native() -> void
{
    bool can_refresh = true;

#ifdef Q_OS_WIN
    can_refresh = (handle() != nullptr);
#endif

    if (can_refresh)
    {
        const bool current = get_is_maximized_native();

        if (m_is_maximized_native != current)
        {
            m_is_maximized_native = current;
            emit is_maximized_native_changed();
        }
    }
}

}  // namespace QtQuickCommonLib
