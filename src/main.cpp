#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "mainwindow.h"

/**
 * @brief Главная функция приложения
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код завершения приложения
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Установка информации о приложении
    QApplication::setApplicationName("Инспектор Аэропортов");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Росавиация");
    
    // Загрузка таблицы стилей приложения
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QTextStream styleStream(&styleFile);
        app.setStyleSheet(styleStream.readAll());
        styleFile.close();
    }
    
    // Создание и отображение главного окна
    MainWindow mainWindow;
    mainWindow.show();
    
    // Запуск цикла обработки событий приложения
    return app.exec();
} 