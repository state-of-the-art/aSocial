#include "uiworker.h"

#include <QLoggingCategory>

#include "cli/loopscheduler.h"
#include "cli/clifilesession.h"
#include "qrcodegen.hpp"

Q_LOGGING_CATEGORY(Cw, PLUGIN_NAME "-worker")

void UiWorker::configure()
{
    qCDebug(Cw) << __func__;

    root_menu = std::make_unique< cli::Menu >( "cli" );
    root_menu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, world\n"; },
            "Print hello world" );
    root_menu -> Insert(
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
    root_menu -> Insert(
            "hello_everysession",
            [](std::ostream&){ cli::Cli::cout() << "Hello, everybody" << std::endl; },
            "Print hello everybody on all open sessions" );
    root_menu -> Insert(
            "answer",
            [](std::ostream& out, int x){ out << "The answer is: " << x << "\n"; },
            "Print the answer to Life, the Universe and Everything " );
    root_menu -> Insert(
            "color",
            [](std::ostream& out){ out << "Colors ON\n"; cli::SetColor(); },
            "Enable colors in the cli" );
    root_menu -> Insert(
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
    subMenu -> Insert( std::move(subSubMenu) );

    root_menu -> Insert( std::move(subMenu) );
}

void UiWorker::run()
{
    qCDebug(Cw) << __func__;

    cli::Cli cli( std::move(root_menu) );
    // global exit action
    cli.ExitAction( [](auto& out){ out << "Goodbye and thanks for all the fish.\n"; } );

    cli::CliFileSession input(cli);
    input.Start();
}
