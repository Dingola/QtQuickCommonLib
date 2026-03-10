#include "QtQuickCommonLib/Views/AppWindowLinux.h"

#include "QtQuickCommonLib/Views/AppWindow.h"

namespace QtQuickCommonLib
{

/**
 * @brief Construct the Linux helper for a specific AppWindow instance.
 *
 * @param app_window Owning AppWindow instance.
 */
AppWindowLinux::AppWindowLinux(AppWindow* app_window): m_app_window(app_window) {}

/**
 * @brief Returns the current maximized state derived from the window state.
 *
 * @return bool True if the window is maximized.
 */
auto AppWindowLinux::get_is_maximized_native() const -> bool
{
    bool result = false;

    if (m_app_window != nullptr)
    {
        result = (m_app_window->windowState() & Qt::WindowMaximized) != 0;
    }

    return result;
}

/**
 * @brief Start a system-managed move operation for the owning window.
 *
 * @return bool True if the compositor accepted the move request.
 */
auto AppWindowLinux::start_system_move() -> bool
{
    bool result = false;

#ifdef Q_OS_LINUX
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
 * @return bool True if the compositor accepted the resize request.
 */
auto AppWindowLinux::start_system_resize(Qt::Edges edges) -> bool
{
    bool result = false;

#ifdef Q_OS_LINUX
    if (m_app_window != nullptr)
    {
        result = m_app_window->startSystemResize(edges);
    }
#else
    Q_UNUSED(edges);
#endif

    return result;
}

}  // namespace QtQuickCommonLib