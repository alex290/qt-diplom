#ifndef AIRPORTLOADING_H
#define AIRPORTLOADING_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFrame>
#include "database.h"

/**
 * @brief Класс для отображения загруженности аэропортов
 */
class AirportLoadingWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parent Родительский виджет
     */
    explicit AirportLoadingWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~AirportLoadingWidget();
    
    /**
     * @brief Установить код аэропорта для отображения загруженности
     * @param airportCode Код аэропорта (IATA)
     */
    void setAirportCode(const QString &airportCode);

private slots:
    /**
     * @brief Обработчик нажатия кнопки выхода
     */
    void onCloseButtonClicked();
    
    /**
     * @brief Загрузить данные о загруженности аэропорта
     */
    void loadAirportLoadingData();

private:
    /**
     * @brief Настройка пользовательского интерфейса
     */
    void setupUi();
    
    /**
     * @brief Создание графика загруженности
     * @param loadingData Данные о загруженности
     */
    void createLoadingChart(const QList<QMap<QString, QVariant>> &loadingData);
    
    // Зона 1: Информационная зона
    QLabel *airportNameLabel;
    
    // Зона 2: Зона построения графиков
    QFrame *chartFrame;
    QLabel *chartLabel;
    
    // Зона 3: Кнопка выхода
    QPushButton *closeButton;
    
    // Основная компоновка
    QVBoxLayout *mainLayout;
    
    // Код аэропорта
    QString airportCode;
    
    // База данных
    Database *db;

signals:
    /**
     * @brief Сигнал о закрытии виджета
     */
    void closeRequested();
};

#endif // AIRPORTLOADING_H 