#include <QApplication>
#include <QMainWindow>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QMainWindow window;

    // Setup vendor information
    QCoreApplication::setOrganizationName("flerovium");
    QCoreApplication::setApplicationName("NanoboyAdvance");

    window.show();
    return app.exec();
}