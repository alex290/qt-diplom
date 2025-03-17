#include "userprofile.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>

/**
 * @brief Конструктор класса профиля пользователя
 * @param parent Родительский виджет
 */
UserProfile::UserProfile(QWidget *parent)
    : QWidget(parent), currentUserId(-1)
{
    db = Database::getInstance();
    setupUi();
    
    // Соединение сигналов и слотов
    connect(saveButton, &QPushButton::clicked, this, &UserProfile::saveProfile);
}

/**
 * @brief Деструктор класса профиля пользователя
 */
UserProfile::~UserProfile()
{
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void UserProfile::setupUi()
{
    // Создание элементов пользовательского интерфейса
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Группа профиля
    QGroupBox *profileGroupBox = new QGroupBox("Информация профиля", this);
    QFormLayout *profileLayout = new QFormLayout(profileGroupBox);
    
    QLabel *usernameTitleLabel = new QLabel("Имя пользователя:", this);
    QLabel *emailLabel = new QLabel("Email:", this);
    QLabel *fullNameLabel = new QLabel("Полное имя:", this);
    QLabel *registrationDateTitleLabel = new QLabel("Дата регистрации:", this);
    
    usernameLabel = new QLabel(this);
    emailEdit = new QLineEdit(this);
    fullNameEdit = new QLineEdit(this);
    registrationDateLabel = new QLabel(this);
    saveButton = new QPushButton("Сохранить изменения", this);
    
    profileLayout->addRow(usernameTitleLabel, usernameLabel);
    profileLayout->addRow(emailLabel, emailEdit);
    profileLayout->addRow(fullNameLabel, fullNameEdit);
    profileLayout->addRow(registrationDateTitleLabel, registrationDateLabel);
    profileLayout->addRow("", saveButton);
    
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
    mainLayout->addWidget(profileGroupBox);
    mainLayout->addWidget(bookingsGroupBox);
    
    setLayout(mainLayout);
}

/**
 * @brief Установка ID пользователя
 * @param userId ID пользователя
 */
void UserProfile::setUserId(int userId)
{
    currentUserId = userId;
}

/**
 * @brief Загрузка профиля пользователя
 */
void UserProfile::loadUserProfile()
{
    if (currentUserId < 0) {
        return;
    }
    
    // Получение профиля пользователя
    QMap<QString, QVariant> profile = db->getUserProfile(currentUserId);
    
    if (profile.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить профиль пользователя.");
        return;
    }
    
    // Отображение профиля пользователя
    displayUserProfile(profile);
}

/**
 * @brief Загрузка бронирований пользователя
 */
void UserProfile::loadUserBookings()
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
 * @brief Сохранение изменений в профиле
 */
void UserProfile::saveProfile()
{
    if (currentUserId < 0) {
        QMessageBox::warning(this, "Ошибка", "Пользователь не вошел в систему.");
        return;
    }
    
    // Получение информации профиля
    QString email = emailEdit->text();
    QString fullName = fullNameEdit->text();
    
    // Проверка ввода
    if (email.isEmpty() || fullName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, введите email и полное имя.");
        return;
    }
    
    // Обновление профиля
    bool success = db->updateUserProfile(currentUserId, email, fullName);
    
    if (success) {
        QMessageBox::information(this, "Успех", "Профиль успешно обновлен.");
        loadUserProfile();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось обновить профиль.");
    }
}

/**
 * @brief Отображение профиля пользователя
 * @param profile Данные профиля пользователя
 */
void UserProfile::displayUserProfile(const QMap<QString, QVariant> &profile)
{
    usernameLabel->setText(profile["username"].toString());
    emailEdit->setText(profile["email"].toString());
    fullNameEdit->setText(profile["full_name"].toString());
    
    QDateTime registrationDateTime = QDateTime::fromString(profile["registration_date"].toString(), Qt::ISODate);
    registrationDateLabel->setText(registrationDateTime.toString("yyyy-MM-dd hh:mm"));
}

/**
 * @brief Отображение бронирований пользователя
 * @param bookings Список бронирований
 */
void UserProfile::displayUserBookings(const QList<QMap<QString, QVariant>> &bookings)
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