/**
 * @file main.cpp
 * @brief This file contains the main function of the application.
 */

#include <QGuiApplication>
#include <QQuickStyle>

#include "QtQuickCommonLib/QMLApplication.h"
#include "QtQuickCommonLib/Views/AppWindow.h"

// GCOVR_EXCL_START  LCOV_EXCL_START

/**
 * @brief The main function initializes the Qt application and executes the application event loop.
 *
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 *
 * @return The exit code of the application.
 */
auto main(int argc, char* argv[]) -> int
{
    // This line ensures that the resources are included in the final application binary when using
    // static linking.
    Q_INIT_RESOURCE(resources);

    // Prefer OpenGLES (often better for 2D QML interfaces on Windows).
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    // Set the style to "Basic" to allow full customization of controls (backgrounds, etc.)
    // Native styles (like Windows) do not support overwriting background/contentItem.
    QQuickStyle::setStyle("Basic");

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QtQuickApp"));
    app.setOrganizationName(QStringLiteral("QtQuickCommonLib"));
    app.setOrganizationDomain(QStringLiteral("AdrianHelbig.de"));

    qmlRegisterType<QtQuickCommonLib::AppWindow>("QtQuickCommonLib", 1, 0, "AppWindow");

    QMLApplication qml_app;

    return qml_app.exec();
}

// GCOVR_EXCL_STOP   LCOV_EXCL_STOP
