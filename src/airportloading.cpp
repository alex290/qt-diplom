#include "airportloading.h"
#include <QMessageBox>

/**
 * @brief Конструктор
 * @param parent Родительский виджет
 */
AirportLoadingWidget::AirportLoadingWidget(QWidget *parent)
    : QWidget(parent)
{
    // Получение экземпляра базы данных
    db = Database::getInstance();
    
    // Настройка пользовательского интерфейса
    setupUi();
}

/**
 * @brief Деструктор
 */
AirportLoadingWidget::~AirportLoadingWidget()
{
    // Нет необходимости удалять указатели, так как они будут удалены автоматически
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void AirportLoadingWidget::setupUi()
{
    // Создание основной компоновки
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Зона 1: Информационная зона с отображением названия аэропорта
    airportNameLabel = new QLabel("Загруженность аэропорта", this);
    QFont titleFont = airportNameLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    airportNameLabel->setFont(titleFont);
    airportNameLabel->setAlignment(Qt::AlignCenter);
    airportNameLabel->setStyleSheet("background-color: #e0e0e0; padding: 10px; border-radius: 5px;");
    
    mainLayout->addWidget(airportNameLabel);
    
    // Зона 2: Зона построения графиков
    chartFrame = new QFrame(this);
    chartFrame->setFrameShape(QFrame::StyledPanel);
    chartFrame->setFrameShadow(QFrame::Raised);
    chartFrame->setMinimumHeight(400);
    chartFrame->setStyleSheet("background-color: white;");
    
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    
    chartLabel = new QLabel("График загруженности аэропорта", chartFrame);
    chartLabel->setAlignment(Qt::AlignCenter);
    chartLabel->setStyleSheet("font-size: 12pt; color: #555;");
    
    chartLayout->addWidget(chartLabel);
    
    mainLayout->addWidget(chartFrame);
    
    // Зона 3: Кнопка выхода из окна
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    closeButton = new QPushButton("Закрыть", this);
    closeButton->setMinimumWidth(100);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Подключение сигналов
    connect(closeButton, &QPushButton::clicked, this, &AirportLoadingWidget::onCloseButtonClicked);
    
    // Установка компоновки
    setLayout(mainLayout);
    
    // Установка минимального размера виджета
    setMinimumSize(800, 600);
}

/**
 * @brief Установить код аэропорта для отображения загруженности
 * @param code Код аэропорта (IATA)
 */
void AirportLoadingWidget::setAirportCode(const QString &code)
{
    airportCode = code;
    
    // Получение информации об аэропорте
    QMap<QString, QVariant> airportInfo = db->getAirportInfo(airportCode);
    
    if (!airportInfo.isEmpty()) {
        // Обновление заголовка
        QString airportName = airportInfo["name"].toString();
        airportNameLabel->setText("Загруженность аэропорта: " + airportName + " (" + airportCode + ")");
        
        // Загрузка данных о загруженности
        loadAirportLoadingData();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось получить информацию об аэропорте с кодом " + airportCode);
    }
}

/**
 * @brief Загрузить данные о загруженности аэропорта
 */
void AirportLoadingWidget::loadAirportLoadingData()
{
    // Здесь должен быть код для получения данных о загруженности аэропорта из базы данных
    // Для примера создадим тестовые данные
    QList<QMap<QString, QVariant>> loadingData;
    
    // Тестовые данные: количество рейсов по часам
    QStringList hours = {"00:00", "02:00", "04:00", "06:00", "08:00", "10:00", "12:00", "14:00", "16:00", "18:00", "20:00", "22:00"};
    
    // Генерация случайных данных для примера
    for (const QString &hour : hours) {
        QMap<QString, QVariant> data;
        data["time"] = hour;
        data["departures"] = QVariant(rand() % 20 + 5); // 5-25 вылетов
        data["arrivals"] = QVariant(rand() % 20 + 5);   // 5-25 прилетов
        loadingData.append(data);
    }
    
    // Создание графика загруженности
    createLoadingChart(loadingData);
}

/**
 * @brief Создание графика загруженности
 * @param loadingData Данные о загруженности
 */
void AirportLoadingWidget::createLoadingChart(const QList<QMap<QString, QVariant>> &loadingData)
{
    // В реальном приложении здесь был бы код для создания графика
    // Для примера просто обновим текст метки
    
    QString chartText = "<h3>Загруженность аэропорта " + airportCode + " по часам</h3>";
    chartText += "<table border='1' cellspacing='0' cellpadding='5' style='margin: 0 auto;'>";
    chartText += "<tr><th>Время</th><th>Вылеты</th><th>Прилеты</th><th>Всего</th></tr>";
    
    int totalDepartures = 0;
    int totalArrivals = 0;
    
    for (const QMap<QString, QVariant> &data : loadingData) {
        int departures = data["departures"].toInt();
        int arrivals = data["arrivals"].toInt();
        int total = departures + arrivals;
        
        totalDepartures += departures;
        totalArrivals += arrivals;
        
        chartText += "<tr>";
        chartText += "<td>" + data["time"].toString() + "</td>";
        chartText += "<td align='center'>" + QString::number(departures) + "</td>";
        chartText += "<td align='center'>" + QString::number(arrivals) + "</td>";
        chartText += "<td align='center'>" + QString::number(total) + "</td>";
        chartText += "</tr>";
    }
    
    chartText += "<tr style='font-weight: bold;'>";
    chartText += "<td>Итого</td>";
    chartText += "<td align='center'>" + QString::number(totalDepartures) + "</td>";
    chartText += "<td align='center'>" + QString::number(totalArrivals) + "</td>";
    chartText += "<td align='center'>" + QString::number(totalDepartures + totalArrivals) + "</td>";
    chartText += "</tr>";
    
    chartText += "</table>";
    
    chartText += "<p style='text-align: center; margin-top: 20px;'>В реальном приложении здесь был бы график загруженности аэропорта.</p>";
    
    chartLabel->setText(chartText);
}

/**
 * @brief Обработчик нажатия кнопки выхода
 */
void AirportLoadingWidget::onCloseButtonClicked()
{
    // Отправка сигнала о закрытии
    emit closeRequested();
} 