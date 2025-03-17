#include "flightsearch.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QDateTime>

/**
 * @brief Конструктор класса поиска рейсов
 * @param parent Родительский виджет
 */
FlightSearch::FlightSearch(QWidget *parent)
    : QWidget(parent), userId(-1)
{
    db = Database::getInstance();
    setupUi();
    loadAirports();
    
    // Соединение сигналов и слотов
    connect(searchButton, &QPushButton::clicked, this, &FlightSearch::searchFlights);
    connect(flightsTable, &QTableView::clicked, this, &FlightSearch::onFlightSelected);
}

/**
 * @brief Деструктор класса поиска рейсов
 */
FlightSearch::~FlightSearch()
{
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void FlightSearch::setupUi()
{
    // Создание основной компоновки
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Создание группы формы поиска
    QGroupBox *searchGroup = new QGroupBox("Поиск рейсов", this);
    QFormLayout *searchLayout = new QFormLayout(searchGroup);
    
    // Создание полей формы
    QLabel *departureLabel = new QLabel("Аэропорт отправления:", searchGroup);
    QLabel *arrivalLabel = new QLabel("Аэропорт прибытия:", searchGroup);
    QLabel *departureDateLabel = new QLabel("Дата отправления:", searchGroup);
    
    departureComboBox = new QComboBox(searchGroup);
    arrivalComboBox = new QComboBox(searchGroup);
    departureDateEdit = new QDateEdit(searchGroup);
    departureDateEdit->setCalendarPopup(true);
    departureDateEdit->setDate(QDate::currentDate());
    departureDateEdit->setMinimumDate(QDate::currentDate());
    
    searchButton = new QPushButton("Найти рейсы", searchGroup);
    
    // Добавление полей в компоновку формы
    searchLayout->addRow(departureLabel, departureComboBox);
    searchLayout->addRow(arrivalLabel, arrivalComboBox);
    searchLayout->addRow(departureDateLabel, departureDateEdit);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchButton);
    searchLayout->addRow("", buttonLayout);
    
    // Создание группы результатов
    QGroupBox *resultsGroup = new QGroupBox("Результаты поиска", this);
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsGroup);
    
    flightsTable = new QTableView(resultsGroup);
    flightsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    flightsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    flightsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    flightsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    resultsLayout->addWidget(flightsTable);
    
    // Добавление групп в основную компоновку
    mainLayout->addWidget(searchGroup);
    mainLayout->addWidget(resultsGroup);
    
    // Установка компоновки
    setLayout(mainLayout);
}

/**
 * @brief Загрузка списка аэропортов
 */
void FlightSearch::loadAirports()
{
    // Очистка выпадающих списков
    departureComboBox->clear();
    arrivalComboBox->clear();
    
    // Загрузка аэропортов из базы данных
    QList<QMap<QString, QVariant>> airports = db->getAllAirports();
    
    for (const QMap<QString, QVariant> &airport : airports) {
        QString code = airport["code"].toString();
        QString name = airport["name"].toString();
        QString city = airport["city"].toString();
        QString display = QString("%1 - %2 (%3)").arg(code).arg(name).arg(city);
        
        departureComboBox->addItem(display, code);
        arrivalComboBox->addItem(display, code);
    }
    
    // Сортировка аэропортов по алфавиту
    departureComboBox->model()->sort(0);
    arrivalComboBox->model()->sort(0);
}

/**
 * @brief Поиск рейсов по заданным параметрам
 */
void FlightSearch::searchFlights()
{
    // Получение параметров поиска
    QString departureAirport = departureComboBox->currentData().toString();
    QString arrivalAirport = arrivalComboBox->currentData().toString();
    QDate departureDate = departureDateEdit->date();
    
    // Проверка ввода
    if (departureAirport.isEmpty() || arrivalAirport.isEmpty()) {
        QMessageBox::warning(this, "Ошибка поиска", "Пожалуйста, выберите аэропорты отправления и прибытия.");
        return;
    }
    
    if (departureAirport == arrivalAirport) {
        QMessageBox::warning(this, "Ошибка поиска", "Аэропорты отправления и прибытия не могут быть одинаковыми.");
        return;
    }
    
    // Поиск рейсов
    QList<QMap<QString, QVariant>> flights = db->searchFlights(departureAirport, arrivalAirport, departureDate);
    
    // Отображение результатов
    displaySearchResults(flights);
}

/**
 * @brief Отображение результатов поиска рейсов
 * @param flights Список найденных рейсов
 */
void FlightSearch::displaySearchResults(const QList<QMap<QString, QVariant>> &flights)
{
    // Создание модели для таблицы рейсов
    QStandardItemModel *model = new QStandardItemModel(0, 6, this);
    model->setHorizontalHeaderLabels(QStringList() << "Номер рейса" << "Отправление" << "Прибытие" 
                                    << "Время отправления" << "Время прибытия" << "Цена");
    
    // Добавление рейсов в модель
    for (int i = 0; i < flights.size(); ++i) {
        const QMap<QString, QVariant> &flight = flights[i];
        
        model->insertRow(i);
        model->setData(model->index(i, 0), flight["flight_number"]);
        model->setData(model->index(i, 1), flight["departure_airport"]);
        model->setData(model->index(i, 2), flight["arrival_airport"]);
        model->setData(model->index(i, 3), QDateTime::fromString(flight["departure_time"].toString(), Qt::ISODate).toString("yyyy-MM-dd hh:mm"));
        model->setData(model->index(i, 4), QDateTime::fromString(flight["arrival_time"].toString(), Qt::ISODate).toString("yyyy-MM-dd hh:mm"));
        model->setData(model->index(i, 5), QString("%1 руб.").arg(flight["price"].toDouble(), 0, 'f', 2));
        
        // Сохранение ID рейса в пользовательской роли
        model->setData(model->index(i, 0), flight["id"], Qt::UserRole);
    }
    
    // Установка модели для табличного представления
    flightsTable->setModel(model);
    flightsTable->resizeColumnsToContents();
    
    // Показать сообщение, если рейсы не найдены
    if (flights.isEmpty()) {
        QMessageBox::information(this, "Результаты поиска", "Не найдено рейсов, соответствующих вашим критериям.");
    }
}

/**
 * @brief Обработка выбора рейса в таблице
 * @param index Индекс выбранной ячейки
 */
void FlightSearch::onFlightSelected(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    // Получение ID рейса из пользовательской роли
    int flightId = flightsTable->model()->data(flightsTable->model()->index(index.row(), 0), Qt::UserRole).toInt();
    
    // Отправка сигнала с выбранным ID рейса
    emit flightSelected(flightId);
}

/**
 * @brief Установка ID пользователя
 * @param id ID пользователя
 */
void FlightSearch::setUserId(int id)
{
    userId = id;
} 