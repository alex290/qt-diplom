#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QIcon>
#include <QSplitter>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QRegularExpression>

/**
 * @brief Конструктор главного окна
 * @param parent Родительский виджет
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentUserId(-1), airportLoadingPage(nullptr)
{
    // Инициализация базы данных
    db = Database::getInstance();
    if (!db->initialize()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных.");
        exit(1);
    }
    
    // Настройка пользовательского интерфейса
    setupUi();
    
    // Обновление статуса входа
    updateLoginStatus();
    
    // Показать поиск рейсов по умолчанию
    showFlightSearch();
}

/**
 * @brief Деструктор главного окна
 */
MainWindow::~MainWindow()
{
    // Нет необходимости удалять указатель ui, так как мы его не используем
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void MainWindow::setupUi()
{
    // Установка свойств окна
    setWindowTitle("Инспектор Аэропортов");
    resize(1024, 768);
    
    // Создание центрального виджета
    createCentralWidget();
    
    // Создание строки меню
    createMenuBar();
    
    // Создание строки состояния
    createStatusBar();
}

/**
 * @brief Создание строки меню
 */
void MainWindow::createMenuBar()
{
    // Создание строки меню
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // Меню "Файл"
    QMenu *fileMenu = menuBar->addMenu("Файл");
    QAction *exitAction = fileMenu->addAction("Выход");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // Меню "Пользователь"
    QMenu *userMenu = menuBar->addMenu("Пользователь");
    QAction *loginAction = userMenu->addAction("Вход");
    QAction *registerAction = userMenu->addAction("Регистрация");
    QAction *logoutAction = userMenu->addAction("Выход из аккаунта");
    QAction *profileAction = userMenu->addAction("Профиль");
    
    connect(loginAction, &QAction::triggered, this, &MainWindow::showLoginDialog);
    connect(registerAction, &QAction::triggered, this, &MainWindow::showRegistrationDialog);
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);
    connect(profileAction, &QAction::triggered, this, &MainWindow::showUserProfile);
    
    // Меню "Вид"
    QMenu *viewMenu = menuBar->addMenu("Вид");
    QAction *flightSearchAction = viewMenu->addAction("Поиск рейсов");
    QAction *airportInfoAction = viewMenu->addAction("Информация об аэропортах");
    
    connect(flightSearchAction, &QAction::triggered, this, &MainWindow::showFlightSearch);
    connect(airportInfoAction, &QAction::triggered, this, &MainWindow::showAirportInfo);
    
    // Меню "Справка"
    QMenu *helpMenu = menuBar->addMenu("Справка");
    QAction *helpAction = helpMenu->addAction("Помощь");
    QAction *aboutAction = helpMenu->addAction("О программе");
    
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelpDialog);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
}

/**
 * @brief Создание строки состояния
 */
void MainWindow::createStatusBar()
{
    // Создание строки состояния
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    
    // Добавление метки статуса
    statusLabel = new QLabel("Готово");
    statusBar->addWidget(statusLabel);
}

/**
 * @brief Создание центрального виджета
 */
void MainWindow::createCentralWidget()
{
    // Создание центрального виджета
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Создание компоновки
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Создание стекового виджета
    stackedWidget = new QStackedWidget(centralWidget);
    
    // Создание страницы входа
    QWidget *loginPage = new QWidget(stackedWidget);
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
    
    QLabel *welcomeLabel = new QLabel("Добро пожаловать в Инспектор Аэропортов", loginPage);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont welcomeFont = welcomeLabel->font();
    welcomeFont.setPointSize(16);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    
    QLabel *descriptionLabel = new QLabel("Пожалуйста, войдите или зарегистрируйтесь для продолжения", loginPage);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    
    QPushButton *loginButton = new QPushButton("Вход", loginPage);
    QPushButton *registerButton = new QPushButton("Регистрация", loginPage);
    
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::showLoginDialog);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::showRegistrationDialog);
    
    loginLayout->addStretch();
    loginLayout->addWidget(welcomeLabel);
    loginLayout->addWidget(descriptionLabel);
    loginLayout->addSpacing(20);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    buttonLayout->addStretch();
    
    loginLayout->addLayout(buttonLayout);
    loginLayout->addStretch();
    
    // Создание основной страницы приложения
    QWidget *appPage = new QWidget(stackedWidget);
    QVBoxLayout *appLayout = new QVBoxLayout(appPage);
    appLayout->setContentsMargins(0, 0, 0, 0);
    appLayout->setSpacing(5);
    
    // Создание разделенного виджета для основной части приложения
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, appPage);
    
    // Создание левой части (зона 2 - таблица отображения рейсов)
    QWidget *flightsTableWidget = new QWidget(mainSplitter);
    QVBoxLayout *flightsTableLayout = new QVBoxLayout(flightsTableWidget);
    flightsTableLayout->setContentsMargins(5, 5, 5, 5);
    
    QLabel *flightsTableLabel = new QLabel("Таблица отображения рейсов", flightsTableWidget);
    flightsTableLabel->setAlignment(Qt::AlignCenter);
    flightsTableLabel->setStyleSheet("font-weight: bold; background-color: #e0e0e0; padding: 5px;");
    
    // Создание таблицы рейсов
    QTableWidget *flightsTable = new QTableWidget(flightsTableWidget);
    flightsTable->setColumnCount(6);
    QStringList headers;
    headers << "ID" << "Рейс" << "Откуда" << "Куда" << "Дата" << "Время";
    flightsTable->setHorizontalHeaderLabels(headers);
    flightsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    flightsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    flightsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    flightsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    flightsTableLayout->addWidget(flightsTableLabel);
    flightsTableLayout->addWidget(flightsTable);
    
    // Создание правой части (зоны 3 и 4)
    QWidget *rightWidget = new QWidget(mainSplitter);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(5, 5, 5, 5);
    
    // Зона 3 - Зона выбора аэропорта и направления вылетов
    QGroupBox *selectionGroupBox = new QGroupBox("Выбор аэропорта и направления вылетов", rightWidget);
    QVBoxLayout *selectionLayout = new QVBoxLayout(selectionGroupBox);
    
    QFormLayout *formLayout = new QFormLayout();
    
    QComboBox *departureAirportComboBox = new QComboBox(selectionGroupBox);
    QComboBox *arrivalAirportComboBox = new QComboBox(selectionGroupBox);
    QDateEdit *departureDateEdit = new QDateEdit(QDate::currentDate(), selectionGroupBox);
    departureDateEdit->setCalendarPopup(true);
    
    formLayout->addRow("Аэропорт вылета:", departureAirportComboBox);
    formLayout->addRow("Аэропорт прибытия:", arrivalAirportComboBox);
    formLayout->addRow("Дата вылета:", departureDateEdit);
    
    selectionLayout->addLayout(formLayout);
    
    // Зона 4 - Кнопки управления
    QGroupBox *buttonsGroupBox = new QGroupBox("Управление", rightWidget);
    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsGroupBox);
    
    QPushButton *searchButton = new QPushButton("Поиск рейсов", buttonsGroupBox);
    QPushButton *bookButton = new QPushButton("Забронировать билет", buttonsGroupBox);
    QPushButton *viewAirportInfoButton = new QPushButton("Информация об аэропорте", buttonsGroupBox);
    QPushButton *viewAirportLoadingButton = new QPushButton("Загруженность аэропорта", buttonsGroupBox);
    QPushButton *viewProfileButton = new QPushButton("Профиль пользователя", buttonsGroupBox);
    
    searchButton->setIcon(QIcon(":/icons/search.png"));
    bookButton->setIcon(QIcon(":/icons/ticket.png"));
    viewAirportInfoButton->setIcon(QIcon(":/icons/airport.png"));
    viewAirportLoadingButton->setIcon(QIcon(":/icons/airport.png"));
    viewProfileButton->setIcon(QIcon(":/icons/user.png"));
    
    buttonsLayout->addWidget(searchButton);
    buttonsLayout->addWidget(bookButton);
    buttonsLayout->addWidget(viewAirportInfoButton);
    buttonsLayout->addWidget(viewAirportLoadingButton);
    buttonsLayout->addWidget(viewProfileButton);
    
    // Добавление групповых боксов в правую часть
    rightLayout->addWidget(selectionGroupBox);
    rightLayout->addWidget(buttonsGroupBox);
    rightLayout->addStretch();
    
    // Установка размеров сплиттера
    mainSplitter->addWidget(flightsTableWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 2); // Таблица занимает 2/3 ширины
    mainSplitter->setStretchFactor(1, 1); // Правая часть занимает 1/3 ширины
    
    // Добавление сплиттера в основную компоновку
    appLayout->addWidget(mainSplitter);
    
    // Зона 5 - Зона отображения статуса подключения к БД
    QFrame *statusFrame = new QFrame(appPage);
    statusFrame->setFrameShape(QFrame::StyledPanel);
    statusFrame->setFrameShadow(QFrame::Sunken);
    QHBoxLayout *statusLayout = new QHBoxLayout(statusFrame);
    
    QLabel *dbStatusLabel = new QLabel("Статус подключения к БД:", statusFrame);
    QLabel *dbStatusValueLabel = new QLabel("Подключено", statusFrame);
    dbStatusValueLabel->setStyleSheet("color: green; font-weight: bold;");
    
    statusLayout->addWidget(dbStatusLabel);
    statusLayout->addWidget(dbStatusValueLabel);
    statusLayout->addStretch();
    
    appLayout->addWidget(statusFrame);
    
    // Подключение сигналов кнопок к слотам
    connect(searchButton, &QPushButton::clicked, [=]() {
        QString departureAirport = departureAirportComboBox->currentText();
        QString arrivalAirport = arrivalAirportComboBox->currentText();
        QDate departureDate = departureDateEdit->date();
        
        // Очистка таблицы
        flightsTable->setRowCount(0);
        
        // Получение списка рейсов
        QList<QMap<QString, QVariant>> flights = db->searchFlights(departureAirport, arrivalAirport, departureDate);
        
        // Заполнение таблицы
        for (const QMap<QString, QVariant> &flight : flights) {
            int row = flightsTable->rowCount();
            flightsTable->insertRow(row);
            
            flightsTable->setItem(row, 0, new QTableWidgetItem(QString::number(flight["id"].toInt())));
            flightsTable->setItem(row, 1, new QTableWidgetItem(flight["flight_number"].toString()));
            flightsTable->setItem(row, 2, new QTableWidgetItem(flight["departure_city"].toString()));
            flightsTable->setItem(row, 3, new QTableWidgetItem(flight["arrival_city"].toString()));
            flightsTable->setItem(row, 4, new QTableWidgetItem(flight["departure_date"].toDate().toString("dd.MM.yyyy")));
            flightsTable->setItem(row, 5, new QTableWidgetItem(flight["departure_time"].toTime().toString("hh:mm")));
        }
        
        statusLabel->setText("Найдено рейсов: " + QString::number(flights.size()));
    });
    
    connect(bookButton, &QPushButton::clicked, [=]() {
        // Получение выбранного рейса
        int row = flightsTable->currentRow();
        if (row >= 0) {
            int flightId = flightsTable->item(row, 0)->text().toInt();
            ticketBookingPage->setFlightId(flightId);
            showTicketBooking();
        } else {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите рейс для бронирования.");
        }
    });
    
    connect(viewAirportInfoButton, &QPushButton::clicked, [=]() {
        QString airportCode = departureAirportComboBox->currentText();
        if (!airportCode.isEmpty()) {
            // Создание диалога информации об аэропорте
            QDialog airportDialog(this);
            airportDialog.setWindowTitle("Информация об аэропорте");
            airportDialog.setMinimumSize(600, 400);
            
            QVBoxLayout *layout = new QVBoxLayout(&airportDialog);
            
            // Добавление виджета информации об аэропорте в диалог
            layout->addWidget(airportInfoPage);
            
            // Извлечение кода аэропорта из строки формата "Название (КОД)"
            QRegularExpression rx("\\(([A-Z]{3})\\)");
            QRegularExpressionMatch match = rx.match(airportCode);
            if (match.hasMatch()) {
                QString code = match.captured(1);
                // Загрузка информации об аэропорте
                airportInfoPage->loadFlights(code);
            }
            
            // Добавление кнопки закрытия
            QHBoxLayout *buttonLayout = new QHBoxLayout();
            QPushButton *closeButton = new QPushButton("Закрыть", &airportDialog);
            buttonLayout->addStretch();
            buttonLayout->addWidget(closeButton);
            
            layout->addLayout(buttonLayout);
            
            connect(closeButton, &QPushButton::clicked, &airportDialog, &QDialog::accept);
            
            airportDialog.exec();
        } else {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите аэропорт.");
        }
    });
    
    connect(viewAirportLoadingButton, &QPushButton::clicked, this, &MainWindow::showAirportLoading);
    
    connect(viewProfileButton, &QPushButton::clicked, this, &MainWindow::showUserProfile);
    
    // Заполнение комбобоксов аэропортов
    QList<QMap<QString, QVariant>> airports = db->getAllAirports();
    for (const QMap<QString, QVariant> &airport : airports) {
        QString airportName = airport["name"].toString() + " (" + airport["code"].toString() + ")";
        departureAirportComboBox->addItem(airportName, airport["code"]);
        arrivalAirportComboBox->addItem(airportName, airport["code"]);
    }
    
    // Добавление страниц в стековый виджет
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(appPage);
    
    // Создание страниц для других функций
    flightSearchPage = new FlightSearch(nullptr);
    airportInfoPage = new AirportInfo(nullptr);
    ticketBookingPage = new TicketBooking(nullptr);
    userProfilePage = new UserProfile(nullptr);
    
    // Добавление стекового виджета в основную компоновку
    mainLayout->addWidget(stackedWidget);
}

/**
 * @brief Показать страницу поиска рейсов
 */
void MainWindow::showFlightSearch()
{
    // Показать страницу поиска рейсов
    if (stackedWidget->currentIndex() == 1) {
        // В новом интерфейсе нет необходимости выполнять дополнительные действия,
        // так как страница поиска рейсов уже отображается по умолчанию
        statusLabel->setText("Поиск рейсов");
    }
}

/**
 * @brief Показать страницу информации об аэропортах
 */
void MainWindow::showAirportInfo()
{
    // Показать страницу информации об аэропортах
    if (stackedWidget->currentIndex() == 1) {
        // В новом интерфейсе нет необходимости выполнять дополнительные действия,
        // так как информация об аэропортах отображается через диалоговое окно
        
        // Находим сплиттер
        QWidget *appPage = qobject_cast<QWidget*>(stackedWidget->widget(1));
        QVBoxLayout *appLayout = qobject_cast<QVBoxLayout*>(appPage->layout());
        QSplitter *mainSplitter = qobject_cast<QSplitter*>(appLayout->itemAt(0)->widget());
        
        // Находим правую часть
        QWidget *rightWidget = qobject_cast<QWidget*>(mainSplitter->widget(1));
        
        // Находим групповой бокс выбора аэропорта
        QGroupBox *selectionGroupBox = qobject_cast<QGroupBox*>(rightWidget->layout()->itemAt(0)->widget());
        
        // Находим комбобокс выбора аэропорта вылета
        QFormLayout *formLayout = qobject_cast<QFormLayout*>(selectionGroupBox->layout()->itemAt(0)->layout());
        QComboBox *departureAirportComboBox = qobject_cast<QComboBox*>(formLayout->itemAt(0, QFormLayout::FieldRole)->widget());
        
        if (departureAirportComboBox && !departureAirportComboBox->currentText().isEmpty()) {
            QString airportCode = departureAirportComboBox->currentText();
            
            // Создание диалога информации об аэропорте
            QDialog airportDialog(this);
            airportDialog.setWindowTitle("Информация об аэропорте");
            airportDialog.setMinimumSize(600, 400);
            
            QVBoxLayout *layout = new QVBoxLayout(&airportDialog);
            
            // Добавление виджета информации об аэропорте в диалог
            layout->addWidget(airportInfoPage);
            
            // Извлечение кода аэропорта из строки формата "Название (КОД)"
            QRegularExpression rx("\\(([A-Z]{3})\\)");
            QRegularExpressionMatch match = rx.match(airportCode);
            if (match.hasMatch()) {
                QString code = match.captured(1);
                // Загрузка информации об аэропорте
                airportInfoPage->loadFlights(code);
            }
            
            // Добавление кнопки закрытия
            QHBoxLayout *buttonLayout = new QHBoxLayout();
            QPushButton *closeButton = new QPushButton("Закрыть", &airportDialog);
            buttonLayout->addStretch();
            buttonLayout->addWidget(closeButton);
            
            layout->addLayout(buttonLayout);
            
            connect(closeButton, &QPushButton::clicked, &airportDialog, &QDialog::accept);
            
            airportDialog.exec();
        } else {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите аэропорт.");
        }
        
        statusLabel->setText("Информация об аэропорте");
    }
}

/**
 * @brief Показать страницу профиля пользователя
 */
void MainWindow::showUserProfile()
{
    // Создание диалога профиля пользователя
    QDialog profileDialog(this);
    profileDialog.setWindowTitle("Профиль пользователя");
    profileDialog.setMinimumSize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&profileDialog);
    
    // Добавление виджета профиля пользователя в диалог
    layout->addWidget(userProfilePage);
    
    // Установка ID пользователя
    userProfilePage->setUserId(currentUserId);
    
    // Загрузка профиля пользователя
    userProfilePage->loadUserProfile();
    userProfilePage->loadUserBookings();
    
    // Добавление кнопки закрытия
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("Закрыть", &profileDialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
    
    connect(closeButton, &QPushButton::clicked, &profileDialog, &QDialog::accept);
    
    profileDialog.exec();
    
    statusLabel->setText("Профиль пользователя");
}

/**
 * @brief Показать диалог входа в систему
 */
void MainWindow::showLoginDialog()
{
    // Создание диалога входа
    QDialog loginDialog(this);
    loginDialog.setWindowTitle("Вход");
    loginDialog.setFixedSize(300, 150);
    
    QVBoxLayout *layout = new QVBoxLayout(&loginDialog);
    
    QFormLayout *formLayout = new QFormLayout();
    QLineEdit *usernameEdit = new QLineEdit(&loginDialog);
    QLineEdit *passwordEdit = new QLineEdit(&loginDialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    formLayout->addRow("Имя пользователя:", usernameEdit);
    formLayout->addRow("Пароль:", passwordEdit);
    
    layout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *loginButton = new QPushButton("Войти", &loginDialog);
    QPushButton *cancelButton = new QPushButton("Отмена", &loginDialog);
    
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
    
    connect(loginButton, &QPushButton::clicked, [&]() {
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();
        
        if (username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(&loginDialog, "Ошибка входа", "Пожалуйста, введите имя пользователя и пароль.");
            return;
        }
        
        // Проверка учетных данных
        int userId = db->authenticateUser(username, password);
        if (userId >= 0) {
            // Вход успешен
            onUserLoggedIn(userId);
            loginDialog.accept();
        } else {
            QMessageBox::warning(&loginDialog, "Ошибка входа", "Неверное имя пользователя или пароль.");
        }
    });
    
    connect(cancelButton, &QPushButton::clicked, &loginDialog, &QDialog::reject);
    
    loginDialog.exec();
}

/**
 * @brief Показать диалог регистрации
 */
void MainWindow::showRegistrationDialog()
{
    // Создание диалога регистрации
    QDialog registerDialog(this);
    registerDialog.setWindowTitle("Регистрация");
    registerDialog.setFixedSize(300, 250);
    
    QVBoxLayout *layout = new QVBoxLayout(&registerDialog);
    
    QFormLayout *formLayout = new QFormLayout();
    QLineEdit *usernameEdit = new QLineEdit(&registerDialog);
    QLineEdit *passwordEdit = new QLineEdit(&registerDialog);
    QLineEdit *confirmPasswordEdit = new QLineEdit(&registerDialog);
    QLineEdit *emailEdit = new QLineEdit(&registerDialog);
    QLineEdit *fullNameEdit = new QLineEdit(&registerDialog);
    
    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    
    formLayout->addRow("Имя пользователя:", usernameEdit);
    formLayout->addRow("Пароль:", passwordEdit);
    formLayout->addRow("Подтверждение пароля:", confirmPasswordEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Полное имя:", fullNameEdit);
    
    layout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *registerButton = new QPushButton("Зарегистрироваться", &registerDialog);
    QPushButton *cancelButton = new QPushButton("Отмена", &registerDialog);
    
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
    
    connect(registerButton, &QPushButton::clicked, [&]() {
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();
        QString email = emailEdit->text();
        QString fullName = fullNameEdit->text();
        
        if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty() || 
            email.isEmpty() || fullName.isEmpty()) {
            QMessageBox::warning(&registerDialog, "Ошибка регистрации", "Пожалуйста, заполните все поля.");
            return;
        }
        
        if (password != confirmPassword) {
            QMessageBox::warning(&registerDialog, "Ошибка регистрации", "Пароли не совпадают.");
            return;
        }
        
        // Регистрация пользователя
        int userId = db->registerUser(username, password, email, fullName);
        if (userId >= 0) {
            QMessageBox::information(&registerDialog, "Регистрация", "Регистрация успешна. Теперь вы можете войти.");
            registerDialog.accept();
        } else {
            QMessageBox::warning(&registerDialog, "Ошибка регистрации", "Не удалось зарегистрировать пользователя. Возможно, имя пользователя уже существует.");
        }
    });
    
    connect(cancelButton, &QPushButton::clicked, &registerDialog, &QDialog::reject);
    
    registerDialog.exec();
}

/**
 * @brief Выход из системы
 */
void MainWindow::logout()
{
    // Выход пользователя
    currentUserId = -1;
    currentUsername = "";
    
    // Обновление интерфейса
    updateLoginStatus();
    
    // Показать страницу входа
    stackedWidget->setCurrentIndex(0);
    
    // Обновление строки состояния
    statusLabel->setText("Выполнен выход из аккаунта");
}

/**
 * @brief Обновление статуса входа
 */
void MainWindow::updateLoginStatus()
{
    // Обновление интерфейса на основе статуса входа
    if (currentUserId > 0) {
        // Пользователь вошел в систему
        stackedWidget->setCurrentIndex(1);
        statusLabel->setText("Вход выполнен как " + currentUsername);
    } else {
        // Пользователь не вошел в систему
        stackedWidget->setCurrentIndex(0);
        statusLabel->setText("Вход не выполнен");
    }
}

/**
 * @brief Обработка входа пользователя
 * @param userId ID пользователя
 */
void MainWindow::onUserLoggedIn(int userId)
{
    // Установка текущего пользователя
    currentUserId = userId;
    
    // Получение имени пользователя
    QMap<QString, QVariant> userProfile = db->getUserProfile(userId);
    currentUsername = userProfile["username"].toString();
    
    // Обновление интерфейса
    updateLoginStatus();
    
    // Установка ID пользователя для бронирования билетов и профиля пользователя
    ticketBookingPage->setUserId(userId);
    userProfilePage->setUserId(userId);
    
    // Показать страницу поиска рейсов
    showFlightSearch();
    
    // Обновление строки состояния
    statusLabel->setText("Вход выполнен как " + currentUsername);
}

/**
 * @brief Показать диалог "О программе"
 */
void MainWindow::showAboutDialog()
{
    // Показать диалог "О программе"
    QMessageBox::about(this, "О программе Инспектор Аэропортов",
                      "Инспектор Аэропортов v1.0\n\n"
                      "Приложение для поиска рейсов и бронирования билетов.\n\n"
                      "Разработано для Росавиации.");
}

/**
 * @brief Показать диалог справки
 */
void MainWindow::showHelpDialog()
{
    // Показать диалог справки
    QMessageBox::information(this, "Справка",
                           "Справка по Инспектору Аэропортов\n\n"
                           "Поиск рейсов: Поиск рейсов по городу отправления, городу прибытия и дате.\n"
                           "Информация об аэропортах: Просмотр информации об аэропортах.\n"
                           "Бронирование билетов: Бронирование билетов на рейсы.\n"
                           "Профиль пользователя: Просмотр и редактирование информации профиля.");
}

/**
 * @brief Показать страницу бронирования билетов
 */
void MainWindow::showTicketBooking()
{
    // Создание диалога бронирования билетов
    QDialog bookingDialog(this);
    bookingDialog.setWindowTitle("Бронирование билетов");
    bookingDialog.setMinimumSize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&bookingDialog);
    
    // Добавление виджета бронирования билетов в диалог
    layout->addWidget(ticketBookingPage);
    
    // Добавление кнопки закрытия
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("Закрыть", &bookingDialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
    
    connect(closeButton, &QPushButton::clicked, &bookingDialog, &QDialog::accept);
    
    bookingDialog.exec();
}

/**
 * @brief Показать страницу загруженности аэропортов
 */
void MainWindow::showAirportLoading()
{
    // Создание диалога загруженности аэропортов
    QDialog loadingDialog(this);
    loadingDialog.setWindowTitle("Загруженность аэропортов");
    loadingDialog.setMinimumSize(800, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(&loadingDialog);
    
    // Создание виджета загруженности аэропортов, если он еще не создан
    if (!airportLoadingPage) {
        airportLoadingPage = new AirportLoadingWidget(&loadingDialog);
    }
    
    // Добавление виджета загруженности аэропортов в диалог
    layout->addWidget(airportLoadingPage);
    
    // Получение кода аэропорта
    QString airportCode = "";
    
    // Находим боковую панель и контент
    QWidget *appPage = qobject_cast<QWidget*>(stackedWidget->widget(1));
    QVBoxLayout *appLayout = qobject_cast<QVBoxLayout*>(appPage->layout());
    
    // Находим сплиттер
    QSplitter *mainSplitter = qobject_cast<QSplitter*>(appLayout->itemAt(0)->widget());
    
    // Находим правую часть
    QWidget *rightWidget = qobject_cast<QWidget*>(mainSplitter->widget(1));
    
    // Находим групповой бокс выбора аэропорта
    QGroupBox *selectionGroupBox = qobject_cast<QGroupBox*>(rightWidget->layout()->itemAt(0)->widget());
    
    // Находим комбобокс выбора аэропорта вылета
    QFormLayout *formLayout = qobject_cast<QFormLayout*>(selectionGroupBox->layout()->itemAt(0)->layout());
    QComboBox *departureAirportComboBox = qobject_cast<QComboBox*>(formLayout->itemAt(0, QFormLayout::FieldRole)->widget());
    
    if (departureAirportComboBox) {
        QString airportText = departureAirportComboBox->currentText();
        
        // Извлечение кода аэропорта из строки формата "Название (КОД)"
        QRegularExpression rx("\\(([A-Z]{3})\\)");
        QRegularExpressionMatch match = rx.match(airportText);
        if (match.hasMatch()) {
            airportCode = match.captured(1);
        }
    }
    
    if (!airportCode.isEmpty()) {
        // Установка кода аэропорта
        airportLoadingPage->setAirportCode(airportCode);
    } else {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите аэропорт.");
        return;
    }
    
    // Подключение сигнала закрытия
    connect(airportLoadingPage, &AirportLoadingWidget::closeRequested, &loadingDialog, &QDialog::accept);
    
    // Отображение диалога
    loadingDialog.exec();
} 