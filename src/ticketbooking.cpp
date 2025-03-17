#include "ticketbooking.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QSqlQuery>

/**
 * @brief Конструктор класса бронирования билетов
 * @param parent Родительский виджет
 */
TicketBooking::TicketBooking(QWidget *parent)
    : QWidget(parent)
    , currentFlightId(-1)
    , currentUserId(-1)
{
    db = Database::getInstance();
    setupUi();
    
    // Соединение сигналов и слотов
    connect(bookButton, &QPushButton::clicked, this, &TicketBooking::bookTicket);
    connect(seatClassComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TicketBooking::updatePrice);
}

/**
 * @brief Деструктор класса бронирования билетов
 */
TicketBooking::~TicketBooking()
{
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void TicketBooking::setupUi()
{
    // Создание элементов пользовательского интерфейса
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Группа деталей рейса
    QGroupBox *flightDetailsGroupBox = new QGroupBox("Детали рейса", this);
    QFormLayout *flightDetailsLayout = new QFormLayout(flightDetailsGroupBox);
    
    QLabel *flightNumberTitleLabel = new QLabel("Номер рейса:", this);
    QLabel *airlineTitleLabel = new QLabel("Авиакомпания:", this);
    QLabel *departureTitleLabel = new QLabel("Отправление:", this);
    QLabel *arrivalTitleLabel = new QLabel("Прибытие:", this);
    QLabel *departureDateTimeTitleLabel = new QLabel("Дата/время отправления:", this);
    QLabel *arrivalDateTimeTitleLabel = new QLabel("Дата/время прибытия:", this);
    
    flightNumberLabel = new QLabel(this);
    airlineLabel = new QLabel(this);
    departureLabel = new QLabel(this);
    arrivalLabel = new QLabel(this);
    departureDateTimeLabel = new QLabel(this);
    arrivalDateTimeLabel = new QLabel(this);
    
    flightDetailsLayout->addRow(flightNumberTitleLabel, flightNumberLabel);
    flightDetailsLayout->addRow(airlineTitleLabel, airlineLabel);
    flightDetailsLayout->addRow(departureTitleLabel, departureLabel);
    flightDetailsLayout->addRow(arrivalTitleLabel, arrivalLabel);
    flightDetailsLayout->addRow(departureDateTimeTitleLabel, departureDateTimeLabel);
    flightDetailsLayout->addRow(arrivalDateTimeTitleLabel, arrivalDateTimeLabel);
    
    // Группа информации о бронировании
    QGroupBox *bookingGroupBox = new QGroupBox("Информация о бронировании", this);
    QFormLayout *bookingLayout = new QFormLayout(bookingGroupBox);
    
    QLabel *passengerNameLabel = new QLabel("Имя пассажира:", this);
    QLabel *passengerPassportLabel = new QLabel("Номер паспорта:", this);
    QLabel *seatClassLabel = new QLabel("Класс места:", this);
    QLabel *priceTitleLabel = new QLabel("Цена:", this);
    
    passengerNameEdit = new QLineEdit(this);
    passengerPassportEdit = new QLineEdit(this);
    seatClassComboBox = new QComboBox(this);
    priceLabel = new QLabel(this);
    bookButton = new QPushButton("Забронировать билет", this);
    
    seatClassComboBox->addItem("Эконом");
    seatClassComboBox->addItem("Бизнес");
    seatClassComboBox->addItem("Первый класс");
    
    bookingLayout->addRow(passengerNameLabel, passengerNameEdit);
    bookingLayout->addRow(passengerPassportLabel, passengerPassportEdit);
    bookingLayout->addRow(seatClassLabel, seatClassComboBox);
    bookingLayout->addRow(priceTitleLabel, priceLabel);
    bookingLayout->addRow("", bookButton);
    
    // Группа бронирований
    QGroupBox *bookingsGroupBox = new QGroupBox("Ваши бронирования", this);
    QVBoxLayout *bookingsLayout = new QVBoxLayout(bookingsGroupBox);
    
    bookingsTableView = new QTableView(this);
    bookingsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookingsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    bookingsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookingsTableView->setAlternatingRowColors(true);
    
    bookingsModel = new QStandardItemModel(this);
    bookingsTableView->setModel(bookingsModel);
    
    bookingsLayout->addWidget(bookingsTableView);
    
    // Добавление групп в основную компоновку
    mainLayout->addWidget(flightDetailsGroupBox);
    mainLayout->addWidget(bookingGroupBox);
    mainLayout->addWidget(bookingsGroupBox);
    
    setLayout(mainLayout);
}

/**
 * @brief Установка ID рейса
 * @param flightId ID рейса
 */
void TicketBooking::setFlightId(int flightId)
{
    currentFlightId = flightId;
    loadFlightDetails();
}

/**
 * @brief Установка ID пользователя
 * @param userId ID пользователя
 */
void TicketBooking::setUserId(int userId)
{
    currentUserId = userId;
    loadUserBookings();
}

/**
 * @brief Загрузка деталей рейса
 */
void TicketBooking::loadFlightDetails()
{
    if (currentFlightId < 0) {
        return;
    }
    
    // Поиск рейса в базе данных
    QSqlQuery query;
    query.prepare(
        "SELECT f.id, f.flight_number, a.name as airline_name, "
        "dep.code as departure_code, dep.city as departure_city, "
        "arr.code as arrival_code, arr.city as arrival_city, "
        "f.departure_time, f.arrival_time, "
        "f.price_economy, f.price_business, f.price_first, "
        "f.available_seats_economy, f.available_seats_business, f.available_seats_first "
        "FROM flights f "
        "JOIN airlines a ON f.airline_id = a.id "
        "JOIN airports dep ON f.departure_airport_id = dep.id "
        "JOIN airports arr ON f.arrival_airport_id = arr.id "
        "WHERE f.id = ?"
    );
    
    query.addBindValue(currentFlightId);
    
    if (query.exec() && query.next()) {
        // Сохранение деталей рейса
        currentFlight.clear();
        currentFlight["id"] = query.value("id");
        currentFlight["flight_number"] = query.value("flight_number");
        currentFlight["airline_name"] = query.value("airline_name");
        currentFlight["departure_code"] = query.value("departure_code");
        currentFlight["departure_city"] = query.value("departure_city");
        currentFlight["arrival_code"] = query.value("arrival_code");
        currentFlight["arrival_city"] = query.value("arrival_city");
        currentFlight["departure_time"] = query.value("departure_time");
        currentFlight["arrival_time"] = query.value("arrival_time");
        currentFlight["price_economy"] = query.value("price_economy");
        currentFlight["price_business"] = query.value("price_business");
        currentFlight["price_first"] = query.value("price_first");
        currentFlight["available_seats_economy"] = query.value("available_seats_economy");
        currentFlight["available_seats_business"] = query.value("available_seats_business");
        currentFlight["available_seats_first"] = query.value("available_seats_first");
        
        // Отображение деталей рейса
        flightNumberLabel->setText(currentFlight["flight_number"].toString());
        airlineLabel->setText(currentFlight["airline_name"].toString());
        departureLabel->setText(QString("%1 (%2)").arg(currentFlight["departure_city"].toString(), currentFlight["departure_code"].toString()));
        arrivalLabel->setText(QString("%1 (%2)").arg(currentFlight["arrival_city"].toString(), currentFlight["arrival_code"].toString()));
        
        QDateTime departureDateTime = QDateTime::fromString(currentFlight["departure_time"].toString(), Qt::ISODate);
        QDateTime arrivalDateTime = QDateTime::fromString(currentFlight["arrival_time"].toString(), Qt::ISODate);
        
        departureDateTimeLabel->setText(departureDateTime.toString("yyyy-MM-dd hh:mm"));
        arrivalDateTimeLabel->setText(arrivalDateTime.toString("yyyy-MM-dd hh:mm"));
        
        // Обновление цены в зависимости от выбранного класса места
        updatePrice(seatClassComboBox->currentIndex());
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить детали рейса.");
    }
}

/**
 * @brief Загрузка бронирований пользователя
 */
void TicketBooking::loadUserBookings()
{
    if (currentUserId < 0) {
        return;
    }
    
    // Получение бронирований пользователя
    QList<QMap<QString, QVariant>> bookings = db->getUserBookings(currentUserId);
    
    // Отображение бронирований
    displayUserBookings(bookings);
}

/**
 * @brief Бронирование билета
 */
void TicketBooking::bookTicket()
{
    if (currentFlightId < 0) {
        QMessageBox::warning(this, "Ошибка бронирования", "Рейс не выбран.");
        return;
    }
    
    if (currentUserId < 0) {
        QMessageBox::warning(this, "Ошибка бронирования", "Пожалуйста, войдите в систему для бронирования билета.");
        return;
    }
    
    // Получение информации о бронировании
    QString passengerName = passengerNameEdit->text();
    QString passengerPassport = passengerPassportEdit->text();
    QString seatClass = seatClassComboBox->currentText();
    
    // Перевод класса места на английский для базы данных
    QString dbSeatClass;
    if (seatClass == "Эконом") {
        dbSeatClass = "Economy";
    } else if (seatClass == "Бизнес") {
        dbSeatClass = "Business";
    } else if (seatClass == "Первый класс") {
        dbSeatClass = "First";
    }
    
    // Проверка ввода
    if (passengerName.isEmpty() || passengerPassport.isEmpty()) {
        QMessageBox::warning(this, "Ошибка бронирования", "Пожалуйста, введите имя пассажира и номер паспорта.");
        return;
    }
    
    // Проверка доступности мест
    int availableSeats = 0;
    if (dbSeatClass == "Economy") {
        availableSeats = currentFlight["available_seats_economy"].toInt();
    } else if (dbSeatClass == "Business") {
        availableSeats = currentFlight["available_seats_business"].toInt();
    } else if (dbSeatClass == "First") {
        availableSeats = currentFlight["available_seats_first"].toInt();
    }
    
    if (availableSeats <= 0) {
        QMessageBox::warning(this, "Ошибка бронирования", QString("Нет доступных мест класса %1 для этого рейса.").arg(seatClass));
        return;
    }
    
    // Бронирование билета
    int bookingId = db->bookTicket(currentFlightId, currentUserId, dbSeatClass, passengerName, passengerPassport);
    
    if (bookingId >= 0) {
        QMessageBox::information(this, "Бронирование успешно", 
                               QString("Ваш билет успешно забронирован.\nНомер бронирования: %1").arg(bookingId));
        
        // Очистка формы
        passengerNameEdit->clear();
        passengerPassportEdit->clear();
        
        // Перезагрузка деталей рейса и бронирований пользователя
        loadFlightDetails();
        loadUserBookings();
    } else {
        QMessageBox::warning(this, "Ошибка бронирования", "Не удалось забронировать билет.");
    }
}

/**
 * @brief Обновление цены в зависимости от выбранного класса места
 * @param index Индекс выбранного класса места
 */
void TicketBooking::updatePrice(int index)
{
    if (currentFlight.isEmpty()) {
        return;
    }
    
    double price = 0.0;
    
    switch (index) {
        case 0: // Эконом
            price = currentFlight["price_economy"].toDouble();
            break;
        case 1: // Бизнес
            price = currentFlight["price_business"].toDouble();
            break;
        case 2: // Первый класс
            price = currentFlight["price_first"].toDouble();
            break;
    }
    
    priceLabel->setText(QString("%1 руб.").arg(price, 0, 'f', 2));
}

/**
 * @brief Отображение бронирований пользователя
 * @param bookings Список бронирований
 */
void TicketBooking::displayUserBookings(const QList<QMap<QString, QVariant>> &bookings)
{
    // Очистка модели
    bookingsModel->clear();
    
    // Установка заголовков
    QStringList headers = {"ID бронирования", "Рейс", "Отправление", "Прибытие", "Дата/время", "Класс места", "Пассажир", "Статус"};
    bookingsModel->setHorizontalHeaderLabels(headers);
    
    // Добавление бронирований в модель
    for (const auto &booking : bookings) {
        QList<QStandardItem*> row;
        
        row.append(new QStandardItem(booking["id"].toString()));
        row.append(new QStandardItem(booking["flight_number"].toString()));
        row.append(new QStandardItem(QString("%1 (%2)").arg(booking["departure_city"].toString(), booking["departure_code"].toString())));
        row.append(new QStandardItem(QString("%1 (%2)").arg(booking["arrival_city"].toString(), booking["arrival_code"].toString())));
        
        QDateTime departureDateTime = QDateTime::fromString(booking["departure_time"].toString(), Qt::ISODate);
        row.append(new QStandardItem(departureDateTime.toString("yyyy-MM-dd hh:mm")));
        
        // Перевод класса места на русский
        QString seatClass = booking["seat_class"].toString();
        QString translatedSeatClass;
        if (seatClass == "Economy") {
            translatedSeatClass = "Эконом";
        } else if (seatClass == "Business") {
            translatedSeatClass = "Бизнес";
        } else if (seatClass == "First") {
            translatedSeatClass = "Первый класс";
        } else {
            translatedSeatClass = seatClass;
        }
        
        row.append(new QStandardItem(translatedSeatClass));
        row.append(new QStandardItem(booking["passenger_name"].toString()));
        
        // Перевод статуса на русский
        QString status = booking["status"].toString();
        QString translatedStatus;
        if (status == "Confirmed") {
            translatedStatus = "Подтверждено";
        } else if (status == "Pending") {
            translatedStatus = "В ожидании";
        } else if (status == "Cancelled") {
            translatedStatus = "Отменено";
        } else {
            translatedStatus = status;
        }
        
        row.append(new QStandardItem(translatedStatus));
        
        // Добавление строки в модель
        bookingsModel->appendRow(row);
    }
    
    // Изменение размера столбцов по содержимому
    bookingsTableView->resizeColumnsToContents();
} 