#include "UiCmdPlugin.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(C, PLUGIN_NAME)

#include "cli/cli.h"
#include "cli/loopscheduler.h"
#include "cli/clifilesession.h"
#include "qrcodegen.hpp"

Plugin* Plugin::s_pInstance = nullptr;

QString Plugin::name() const
{
    return QLatin1String(PLUGIN_NAME);
}

QStringList Plugin::requirements() const
{
    qCDebug(C) << __func__;
    return QStringList();
}

bool Plugin::init()
{
    if( isInitialized() )
        return true;

    qCDebug(C) << __func__;
    Plugin::s_pInstance = this;

    // TODO: Init here

    // setup cli

    auto rootMenu = std::make_unique< cli::Menu >( "cli" );
    rootMenu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, world\n"; },
            "Print hello world" );
    rootMenu -> Insert(
            "qrcodetest",
            [](std::ostream& out, std::string text) {
                out << "QR code:\n";
                qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text.data(), qrcodegen::QrCode::Ecc::MEDIUM);

                int border = 4;
                for (int y = -border; y < qr.getSize() + border; y++) {
                    for (int x = -border; x < qr.getSize() + border; x++) {
                        out << (qr.getModule(x, y) ? "██" : "  ");
                    }
                    out << std::endl;
                }
            },
            "Print the example qr code" ),
    rootMenu -> Insert(
            "hello_everysession",
            [](std::ostream&){ cli::Cli::cout() << "Hello, everybody" << std::endl; },
            "Print hello everybody on all open sessions" );
    rootMenu -> Insert(
            "answer",
            [](std::ostream& out, int x){ out << "The answer is: " << x << "\n"; },
            "Print the answer to Life, the Universe and Everything " );
    rootMenu -> Insert(
            "color",
            [](std::ostream& out){ out << "Colors ON\n"; cli::SetColor(); },
            "Enable colors in the cli" );
    rootMenu -> Insert(
            "nocolor",
            [](std::ostream& out){ out << "Colors OFF\n"; cli::SetNoColor(); },
            "Disable colors in the cli" );

    auto subMenu = std::make_unique< cli::Menu >( "sub" );
    subMenu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, submenu world\n"; },
            "Print hello world in the submenu" );
    subMenu -> Insert(
            "demo",
            [](std::ostream& out){ out << "This is a sample!\n"; },
            "Print a demo string" );

    auto subSubMenu = std::make_unique< cli::Menu >( "subsub" );
        subSubMenu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, subsubmenu world\n"; },
            "Print hello world in the sub-submenu" );
    subMenu -> Insert( std::move(subSubMenu));

    rootMenu -> Insert( std::move(subMenu) );


    cli::Cli cli( std::move(rootMenu) );
    // global exit action
    cli.ExitAction( [](auto& out){ out << "Goodbye and thanks for all the fish.\n"; } );

    cli::CliFileSession input(cli);
    input.Start();

    qCDebug(C) << "init() done";

    setInitialized(true);

    emit appNotice(this->name().append(" initialized"));

    return true;
}

bool Plugin::deinit()
{
    if( !isInitialized() )
        return true;
    qCDebug(C) << __func__;

    // TODO: Deinit here

    Plugin::s_pInstance = nullptr;

    emit appNotice(this->name().append(" deinitialized"));
    qCDebug(C) << "deinit() done";
    setInitialized(false);

    return true;
}

bool Plugin::configure()
{
    qCDebug(C) << __func__;
    return true;
}
