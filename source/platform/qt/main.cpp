#include <QApplication>
#include "mainwindow.hpp"

int main(int argc, char** argv) {
  QApplication app{ argc, argv };
  MainWindow window{ &app };

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoboyAdvance");

  window.show();
  return app.exec();
}