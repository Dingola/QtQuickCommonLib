#pragma once

#include <QWindow>

namespace QtQuickCommonLib
{

class AppWindow;

/**
 * @class AppWindowLinux
 * @brief Linux-specific helper for AppWindow behavior.
 *
 * This helper encapsulates Linux-specific window behavior so the public AppWindow API can remain
 * stable while platform logic is implemented separately.
 */
class AppWindowLinux
{
    public:
        /**
         * @brief Construct the Linux helper for a specific AppWindow instance.
         *
         * @param app_window Owning AppWindow instance.
         */
        explicit AppWindowLinux(AppWindow* app_window);

        /**
         * @brief Returns the current maximized state derived from the window state.
         *
         * @return bool True if the window is maximized.
         */
        [[nodiscard]] auto get_is_maximized_native() const -> bool;

        /**
         * @brief Start a system-managed move operation for the owning window.
         *
         * @return bool True if the compositor accepted the move request.
         */
        auto start_system_move() -> bool;

        /**
         * @brief Start a system-managed resize operation for the owning window.
         *
         * @param edges Window edges that should participate in the resize operation.
         * @return bool True if the compositor accepted the resize request.
         */
        auto start_system_resize(Qt::Edges edges) -> bool;

    private:
        AppWindow* m_app_window = nullptr;
};

}  // namespace QtQuickCommonLib