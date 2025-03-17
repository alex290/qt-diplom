#include "airportinfo.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>

/**
 * @brief Конструктор класса информации об аэропорте
 * @param parent Родительский виджет
 */
AirportInfo::AirportInfo(QWidget *parent)
    : QWidget(parent)
{
    db = Database::getInstance();
    setupUi();
    loadAirports();
    
    // Соединение сигналов и слотов
    connect(loadButton, &QPushButton::clicked, this, &AirportInfo::loadAirportInfo);
    connect(airportComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AirportInfo::onAirportSelectionChanged);
}

/**
 * @brief Деструктор класса информации об аэропорте
 */
AirportInfo::~AirportInfo()
{
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void AirportInfo::setupUi()
{
    // Создание основной компоновки
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Создание группы выбора аэропорта
    QGroupBox *selectionGroup = new QGroupBox("Выбор аэропорта", this);
    QHBoxLayout *selectionLayout = new QHBoxLayout(selectionGroup);
    
    QLabel *airportLabel = new QLabel("Выберите аэропорт:", selectionGroup);
    airportComboBox = new QComboBox(selectionGroup);
    loadButton = new QPushButton("Загрузить информацию", selectionGroup);
    
    selectionLayout->addWidget(airportLabel);
    selectionLayout->addWidget(airportComboBox, 1);
    selectionLayout->addWidget(loadButton);
    
    // Создание группы информации об аэропорте
    QGroupBox *infoGroup = new QGroupBox("Информация об аэропорте", this);
    QFormLayout *infoLayout = new QFormLayout(infoGroup);
    
    nameLabel = new QLabel(infoGroup);
    codeLabel = new QLabel(infoGroup);
    cityLabel = new QLabel(infoGroup);
    countryLabel = new QLabel(infoGroup);
    timeZoneLabel = new QLabel(infoGroup);
    
    infoLayout->addRow("Название:", nameLabel);
    infoLayout->addRow("Код IATA:", codeLabel);
    infoLayout->addRow("Город:", cityLabel);
    infoLayout->addRow("Страна:", countryLabel);
    infoLayout->addRow("Часовой пояс:", timeZoneLabel);
    
    // Создание группы рейсов
    QGroupBox *flightsGroup = new QGroupBox("Рейсы из этого аэропорта", this);
    QVBoxLayout *flightsLayout = new QVBoxLayout(flightsGroup);
    
    flightsTable = new QTableView(flightsGroup);
    flightsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    flightsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    flightsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    flightsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    flightsLayout->addWidget(flightsTable);
    
    // Добавление групп в основную компоновку
    mainLayout->addWidget(selectionGroup);
    mainLayout->addWidget(infoGroup);
    mainLayout->addWidget(flightsGroup);
    
    // Установка компоновки
    setLayout(mainLayout);
}

/**
 * @brief Загрузка списка аэропортов
 */
void AirportInfo::loadAirports()
{
    // Очистка выпадающего списка
    airportComboBox->clear();
    
    // Загрузка аэропортов из базы данных
    QList<QMap<QString, QVariant>> airports = db->getAllAirports();
    
    for (const QMap<QString, QVariant> &airport : airports) {
        QString code = airport["code"].toString();
        QString name = airport["name"].toString();
        QString display = QString("%1 (%2)").arg(name).arg(code);
        
        airportComboBox->addItem(display, code);
    }
    
    // Сортировка аэропортов по алфавиту
    airportComboBox->model()->sort(0);
}

/**
 * @brief Загрузка информации о выбранном аэропорте
 */
void AirportInfo::loadAirportInfo()
{
    // Получение кода выбранного аэропорта
    int index = airportComboBox->currentIndex();
    if (index < 0) {
        QMessageBox::warning(this, "Выбор аэропорта", "Пожалуйста, выберите аэропорт.");
        return;
    }
    
    QString airportCode = airportComboBox->currentData().toString();
    
    // Загрузка информации об аэропорте
    QMap<QString, QVariant> airportInfo = db->getAirportInfo(airportCode);
    
    if (airportInfo.isEmpty()) {
        QMessageBox::warning(this, "Информация об аэропорте", "Не удалось загрузить информацию об аэропорте.");
        return;
    }
    
    // Отображение информации об аэропорте
    nameLabel->setText(airportInfo["name"].toString());
    codeLabel->setText(airportInfo["code"].toString());
    cityLabel->setText(airportInfo["city"].toString());
    countryLabel->setText(airportInfo["country"].toString());
    timeZoneLabel->setText(airportInfo["timezone"].toString());
    
    // Загрузка рейсов из этого аэропорта
    loadFlights(airportCode);
}

/**
 * @brief Загрузка рейсов из выбранного аэропорта
 * @param airportCode Код аэропорта
 */
void AirportInfo::loadFlights(const QString &airportCode)
{
    // Создание модели для таблицы рейсов
    QStandardItemModel *model = new QStandardItemModel(0, 5, this);
    model->setHorizontalHeaderLabels(QStringList() << "Номер рейса" << "Пункт назначения" << "Время отправления" << "Время прибытия" << "Статус");
    
    // Загрузка рейсов из базы данных
    QSqlQuery query;
    query.prepare("SELECT f.flight_number, a.name, f.departure_time, f.arrival_time, f.status "
                 "FROM flights f "
                 "JOIN airports a ON f.arrival_airport_id = a.id "
                 "WHERE f.departure_airport_id = (SELECT id FROM airports WHERE code = ?) "
                 "ORDER BY f.departure_time");
    query.addBindValue(airportCode);
    
    if (query.exec()) {
        int row = 0;
        while (query.next()) {
            model->insertRow(row);
            model->setData(model->index(row, 0), query.value(0).toString());
            model->setData(model->index(row, 1), query.value(1).toString());
            model->setData(model->index(row, 2), query.value(2).toString());
            model->setData(model->index(row, 3), query.value(3).toString());
            model->setData(model->index(row, 4), query.value(4).toString());
            row++;
        }
    }
    
    // Установка модели для табличного представления
    flightsTable->setModel(model);
    flightsTable->resizeColumnsToContents();
}

/**
 * @brief Обработка изменения выбора аэропорта
 * @param index Индекс выбранного элемента
 */
void AirportInfo::onAirportSelectionChanged(int index)
{
    // Включение/отключение кнопки загрузки в зависимости от выбора
    loadButton->setEnabled(index >= 0);
} 