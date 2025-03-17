#include "database.h"
#include <QCryptographicHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include <QDate>
#include <QRandomGenerator>
#include <QFile>
#include <QDir>

// Initialize static instance
Database* Database::instance = nullptr;

Database* Database::getInstance()
{
    if (!instance) {
        instance = new Database();
    }
    return instance;
}

Database::Database(QObject *parent) : QObject(parent)
{
    // Initialize database connection
}

Database::~Database()
{
    close();
}

bool Database::initialize()
{
    // Set up SQLite database
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("airport_inspector.db");
    
    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }
    
    // Create tables if they don't exist
    createTables();
    
    // Populate with sample data if needed
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM airports");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        populateSampleData();
    }
    
    return true;
}

void Database::close()
{
    if (db.isOpen()) {
        db.close();
    }
}

void Database::createTables()
{
    QSqlQuery query;
    
    // Create airports table
    query.exec("CREATE TABLE IF NOT EXISTS airports ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "code TEXT NOT NULL UNIQUE, "
               "name TEXT NOT NULL, "
               "city TEXT NOT NULL, "
               "country TEXT NOT NULL, "
               "latitude REAL, "
               "longitude REAL, "
               "timezone TEXT, "
               "description TEXT)");
    
    // Create airlines table
    query.exec("CREATE TABLE IF NOT EXISTS airlines ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "code TEXT NOT NULL UNIQUE, "
               "name TEXT NOT NULL, "
               "country TEXT NOT NULL, "
               "logo TEXT)");
    
    // Create flights table
    query.exec("CREATE TABLE IF NOT EXISTS flights ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "flight_number TEXT NOT NULL, "
               "airline_id INTEGER NOT NULL, "
               "departure_airport_id INTEGER NOT NULL, "
               "arrival_airport_id INTEGER NOT NULL, "
               "departure_time DATETIME NOT NULL, "
               "arrival_time DATETIME NOT NULL, "
               "price_economy REAL NOT NULL, "
               "price_business REAL NOT NULL, "
               "price_first REAL NOT NULL, "
               "available_seats_economy INTEGER NOT NULL, "
               "available_seats_business INTEGER NOT NULL, "
               "available_seats_first INTEGER NOT NULL, "
               "FOREIGN KEY (airline_id) REFERENCES airlines(id), "
               "FOREIGN KEY (departure_airport_id) REFERENCES airports(id), "
               "FOREIGN KEY (arrival_airport_id) REFERENCES airports(id))");
    
    // Create users table
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT NOT NULL UNIQUE, "
               "password TEXT NOT NULL, "
               "email TEXT NOT NULL, "
               "full_name TEXT NOT NULL, "
               "registration_date DATETIME NOT NULL)");
    
    // Create bookings table
    query.exec("CREATE TABLE IF NOT EXISTS bookings ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "flight_id INTEGER NOT NULL, "
               "user_id INTEGER NOT NULL, "
               "booking_date DATETIME NOT NULL, "
               "seat_class TEXT NOT NULL, "
               "passenger_name TEXT NOT NULL, "
               "passenger_passport TEXT NOT NULL, "
               "status TEXT NOT NULL, "
               "FOREIGN KEY (flight_id) REFERENCES flights(id), "
               "FOREIGN KEY (user_id) REFERENCES users(id))");
}

void Database::populateSampleData()
{
    QSqlQuery query;
    
    // Insert sample airports
    QList<QStringList> airports = {
        {"SVO", "Sheremetyevo International Airport", "Moscow", "Russia", "55.972", "37.415", "Europe/Moscow", "Major international airport serving Moscow"},
        {"DME", "Domodedovo International Airport", "Moscow", "Russia", "55.408", "37.906", "Europe/Moscow", "One of the major airports serving Moscow"},
        {"LED", "Pulkovo Airport", "Saint Petersburg", "Russia", "59.800", "30.263", "Europe/Moscow", "International airport serving Saint Petersburg"},
        {"AER", "Sochi International Airport", "Sochi", "Russia", "43.449", "39.956", "Europe/Moscow", "International airport serving Sochi"},
        {"KZN", "Kazan International Airport", "Kazan", "Russia", "55.606", "49.278", "Europe/Moscow", "International airport serving Kazan"},
        {"VVO", "Vladivostok International Airport", "Vladivostok", "Russia", "43.396", "132.148", "Asia/Vladivostok", "International airport serving Vladivostok"},
        {"OVB", "Tolmachevo Airport", "Novosibirsk", "Russia", "55.012", "82.650", "Asia/Novosibirsk", "International airport serving Novosibirsk"},
        {"KRR", "Krasnodar International Airport", "Krasnodar", "Russia", "45.035", "39.171", "Europe/Moscow", "International airport serving Krasnodar"},
        {"ROV", "Platov International Airport", "Rostov-on-Don", "Russia", "47.494", "39.924", "Europe/Moscow", "International airport serving Rostov-on-Don"},
        {"UFA", "Ufa International Airport", "Ufa", "Russia", "54.558", "55.874", "Asia/Yekaterinburg", "International airport serving Ufa"}
    };
    
    for (const QStringList& airport : airports) {
        query.prepare("INSERT INTO airports (code, name, city, country, latitude, longitude, timezone, description) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(airport[0]);
        query.addBindValue(airport[1]);
        query.addBindValue(airport[2]);
        query.addBindValue(airport[3]);
        query.addBindValue(airport[4]);
        query.addBindValue(airport[5]);
        query.addBindValue(airport[6]);
        query.addBindValue(airport[7]);
        query.exec();
    }
    
    // Insert sample airlines
    QList<QStringList> airlines = {
        {"AFL", "Aeroflot", "Russia", "aeroflot.png"},
        {"SVR", "Rossiya Airlines", "Russia", "rossiya.png"},
        {"SBI", "S7 Airlines", "Russia", "s7.png"},
        {"UTR", "Utair", "Russia", "utair.png"},
        {"PBD", "Pobeda", "Russia", "pobeda.png"}
    };
    
    for (const QStringList& airline : airlines) {
        query.prepare("INSERT INTO airlines (code, name, country, logo) VALUES (?, ?, ?, ?)");
        query.addBindValue(airline[0]);
        query.addBindValue(airline[1]);
        query.addBindValue(airline[2]);
        query.addBindValue(airline[3]);
        query.exec();
    }
    
    // Insert sample flights
    // Get current date for sample flights
    QDate currentDate = QDate::currentDate();
    
    // Create sample flights for the next 30 days
    for (int day = 0; day < 30; day++) {
        QDate flightDate = currentDate.addDays(day);
        
        // Create flights between different city pairs
        QList<QPair<int, int>> routes = {
            {1, 3}, // Moscow SVO to Saint Petersburg
            {3, 1}, // Saint Petersburg to Moscow SVO
            {1, 4}, // Moscow SVO to Sochi
            {4, 1}, // Sochi to Moscow SVO
            {2, 5}, // Moscow DME to Kazan
            {5, 2}, // Kazan to Moscow DME
            {1, 6}, // Moscow SVO to Vladivostok
            {6, 1}, // Vladivostok to Moscow SVO
            {2, 7}, // Moscow DME to Novosibirsk
            {7, 2}, // Novosibirsk to Moscow DME
            {3, 8}, // Saint Petersburg to Krasnodar
            {8, 3}, // Krasnodar to Saint Petersburg
            {1, 9}, // Moscow SVO to Rostov-on-Don
            {9, 1}, // Rostov-on-Don to Moscow SVO
            {2, 10}, // Moscow DME to Ufa
            {10, 2}  // Ufa to Moscow DME
        };
        
        for (const auto& route : routes) {
            // Morning flight
            QDateTime departureTime(flightDate, QTime(8, 0).addSecs(QRandomGenerator::global()->bounded(4 * 3600)));
            QDateTime arrivalTime = departureTime.addSecs(QRandomGenerator::global()->bounded(2 * 3600, 5 * 3600));
            
            int airlineId = QRandomGenerator::global()->bounded(1, 6);
            double basePrice = QRandomGenerator::global()->bounded(3000, 15000);
            
            query.prepare("INSERT INTO flights (flight_number, airline_id, departure_airport_id, arrival_airport_id, "
                         "departure_time, arrival_time, price_economy, price_business, price_first, "
                         "available_seats_economy, available_seats_business, available_seats_first) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            
            query.addBindValue(QString("FL%1").arg(QRandomGenerator::global()->bounded(1000, 9999)));
            query.addBindValue(airlineId);
            query.addBindValue(route.first);
            query.addBindValue(route.second);
            query.addBindValue(departureTime.toString(Qt::ISODate));
            query.addBindValue(arrivalTime.toString(Qt::ISODate));
            query.addBindValue(basePrice);
            query.addBindValue(basePrice * 2.5);
            query.addBindValue(basePrice * 4.0);
            query.addBindValue(QRandomGenerator::global()->bounded(50, 150));
            query.addBindValue(QRandomGenerator::global()->bounded(10, 30));
            query.addBindValue(QRandomGenerator::global()->bounded(5, 15));
            query.exec();
            
            // Evening flight
            departureTime = QDateTime(flightDate, QTime(16, 0).addSecs(QRandomGenerator::global()->bounded(4 * 3600)));
            arrivalTime = departureTime.addSecs(QRandomGenerator::global()->bounded(2 * 3600, 5 * 3600));
            
            airlineId = QRandomGenerator::global()->bounded(1, 6);
            basePrice = QRandomGenerator::global()->bounded(3000, 15000);
            
            query.prepare("INSERT INTO flights (flight_number, airline_id, departure_airport_id, arrival_airport_id, "
                         "departure_time, arrival_time, price_economy, price_business, price_first, "
                         "available_seats_economy, available_seats_business, available_seats_first) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            
            query.addBindValue(QString("FL%1").arg(QRandomGenerator::global()->bounded(1000, 9999)));
            query.addBindValue(airlineId);
            query.addBindValue(route.first);
            query.addBindValue(route.second);
            query.addBindValue(departureTime.toString(Qt::ISODate));
            query.addBindValue(arrivalTime.toString(Qt::ISODate));
            query.addBindValue(basePrice);
            query.addBindValue(basePrice * 2.5);
            query.addBindValue(basePrice * 4.0);
            query.addBindValue(QRandomGenerator::global()->bounded(50, 150));
            query.addBindValue(QRandomGenerator::global()->bounded(10, 30));
            query.addBindValue(QRandomGenerator::global()->bounded(5, 15));
            query.exec();
        }
    }
    
    // Insert sample user
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData("password123");
    QString hashedPassword = QString(hash.result().toHex());
    
    query.prepare("INSERT INTO users (username, password, email, full_name, registration_date) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue("user");
    query.addBindValue(hashedPassword);
    query.addBindValue("user@example.com");
    query.addBindValue("Sample User");
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.exec();
}

QList<QMap<QString, QVariant>> Database::searchFlights(const QString& departureCity, 
                                                     const QString& arrivalCity, 
                                                     const QDate& departureDate, 
                                                     const QDate& returnDate)
{
    QList<QMap<QString, QVariant>> results;
    
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
        "WHERE dep.city = ? AND arr.city = ? "
        "AND date(f.departure_time) = ? "
        "ORDER BY f.departure_time"
    );
    
    query.addBindValue(departureCity);
    query.addBindValue(arrivalCity);
    query.addBindValue(departureDate.toString(Qt::ISODate));
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> flight;
            flight["id"] = query.value("id");
            flight["flight_number"] = query.value("flight_number");
            flight["airline_name"] = query.value("airline_name");
            flight["departure_code"] = query.value("departure_code");
            flight["departure_city"] = query.value("departure_city");
            flight["arrival_code"] = query.value("arrival_code");
            flight["arrival_city"] = query.value("arrival_city");
            flight["departure_time"] = query.value("departure_time");
            flight["arrival_time"] = query.value("arrival_time");
            flight["price_economy"] = query.value("price_economy");
            flight["price_business"] = query.value("price_business");
            flight["price_first"] = query.value("price_first");
            flight["available_seats_economy"] = query.value("available_seats_economy");
            flight["available_seats_business"] = query.value("available_seats_business");
            flight["available_seats_first"] = query.value("available_seats_first");
            
            results.append(flight);
        }
    } else {
        qDebug() << "Error searching flights:" << query.lastError().text();
    }
    
    // If return date is specified, search for return flights
    if (returnDate.isValid()) {
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
            "WHERE dep.city = ? AND arr.city = ? "
            "AND date(f.departure_time) = ? "
            "ORDER BY f.departure_time"
        );
        
        query.addBindValue(arrivalCity);
        query.addBindValue(departureCity);
        query.addBindValue(returnDate.toString(Qt::ISODate));
        
        if (query.exec()) {
            while (query.next()) {
                QMap<QString, QVariant> flight;
                flight["id"] = query.value("id");
                flight["flight_number"] = query.value("flight_number");
                flight["airline_name"] = query.value("airline_name");
                flight["departure_code"] = query.value("departure_code");
                flight["departure_city"] = query.value("departure_city");
                flight["arrival_code"] = query.value("arrival_code");
                flight["arrival_city"] = query.value("arrival_city");
                flight["departure_time"] = query.value("departure_time");
                flight["arrival_time"] = query.value("arrival_time");
                flight["price_economy"] = query.value("price_economy");
                flight["price_business"] = query.value("price_business");
                flight["price_first"] = query.value("price_first");
                flight["available_seats_economy"] = query.value("available_seats_economy");
                flight["available_seats_business"] = query.value("available_seats_business");
                flight["available_seats_first"] = query.value("available_seats_first");
                flight["is_return"] = true;
                
                results.append(flight);
            }
        } else {
            qDebug() << "Error searching return flights:" << query.lastError().text();
        }
    }
    
    return results;
}

QMap<QString, QVariant> Database::getAirportInfo(const QString& airportCode)
{
    QMap<QString, QVariant> result;
    
    QSqlQuery query;
    query.prepare("SELECT * FROM airports WHERE code = ?");
    query.addBindValue(airportCode);
    
    if (query.exec() && query.next()) {
        result["id"] = query.value("id");
        result["code"] = query.value("code");
        result["name"] = query.value("name");
        result["city"] = query.value("city");
        result["country"] = query.value("country");
        result["latitude"] = query.value("latitude");
        result["longitude"] = query.value("longitude");
        result["timezone"] = query.value("timezone");
        result["description"] = query.value("description");
    } else {
        qDebug() << "Error getting airport info:" << query.lastError().text();
    }
    
    return result;
}

QList<QMap<QString, QVariant>> Database::getAllAirports()
{
    QList<QMap<QString, QVariant>> results;
    
    QSqlQuery query("SELECT * FROM airports ORDER BY city, name");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> airport;
            airport["id"] = query.value("id");
            airport["code"] = query.value("code");
            airport["name"] = query.value("name");
            airport["city"] = query.value("city");
            airport["country"] = query.value("country");
            
            results.append(airport);
        }
    } else {
        qDebug() << "Error getting all airports:" << query.lastError().text();
    }
    
    return results;
}

int Database::bookTicket(int flightId, int userId, const QString& seatClass, 
                       const QString& passengerName, const QString& passengerPassport)
{
    QSqlQuery query;
    
    // Check if flight exists and has available seats
    query.prepare("SELECT available_seats_economy, available_seats_business, available_seats_first "
                 "FROM flights WHERE id = ?");
    query.addBindValue(flightId);
    
    if (!query.exec() || !query.next()) {
        qDebug() << "Error checking flight availability:" << query.lastError().text();
        return -1;
    }
    
    int availableSeats = 0;
    QString seatColumn;
    
    if (seatClass == "Economy") {
        availableSeats = query.value("available_seats_economy").toInt();
        seatColumn = "available_seats_economy";
    } else if (seatClass == "Business") {
        availableSeats = query.value("available_seats_business").toInt();
        seatColumn = "available_seats_business";
    } else if (seatClass == "First") {
        availableSeats = query.value("available_seats_first").toInt();
        seatColumn = "available_seats_first";
    } else {
        qDebug() << "Invalid seat class:" << seatClass;
        return -1;
    }
    
    if (availableSeats <= 0) {
        qDebug() << "No available seats for class:" << seatClass;
        return -1;
    }
    
    // Begin transaction
    db.transaction();
    
    // Update available seats
    query.prepare(QString("UPDATE flights SET %1 = %1 - 1 WHERE id = ?").arg(seatColumn));
    query.addBindValue(flightId);
    
    if (!query.exec()) {
        qDebug() << "Error updating available seats:" << query.lastError().text();
        db.rollback();
        return -1;
    }
    
    // Create booking
    query.prepare("INSERT INTO bookings (flight_id, user_id, booking_date, seat_class, "
                 "passenger_name, passenger_passport, status) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(flightId);
    query.addBindValue(userId);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(seatClass);
    query.addBindValue(passengerName);
    query.addBindValue(passengerPassport);
    query.addBindValue("Confirmed");
    
    if (!query.exec()) {
        qDebug() << "Error creating booking:" << query.lastError().text();
        db.rollback();
        return -1;
    }
    
    int bookingId = query.lastInsertId().toInt();
    
    // Commit transaction
    if (!db.commit()) {
        qDebug() << "Error committing transaction:" << db.lastError().text();
        db.rollback();
        return -1;
    }
    
    return bookingId;
}

QList<QMap<QString, QVariant>> Database::getUserBookings(int userId)
{
    QList<QMap<QString, QVariant>> results;
    
    QSqlQuery query;
    query.prepare(
        "SELECT b.id, b.booking_date, b.seat_class, b.passenger_name, b.passenger_passport, b.status, "
        "f.flight_number, a.name as airline_name, "
        "dep.code as departure_code, dep.city as departure_city, "
        "arr.code as arrival_code, arr.city as arrival_city, "
        "f.departure_time, f.arrival_time "
        "FROM bookings b "
        "JOIN flights f ON b.flight_id = f.id "
        "JOIN airlines a ON f.airline_id = a.id "
        "JOIN airports dep ON f.departure_airport_id = dep.id "
        "JOIN airports arr ON f.arrival_airport_id = arr.id "
        "WHERE b.user_id = ? "
        "ORDER BY f.departure_time DESC"
    );
    
    query.addBindValue(userId);
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> booking;
            booking["id"] = query.value("id");
            booking["booking_date"] = query.value("booking_date");
            booking["seat_class"] = query.value("seat_class");
            booking["passenger_name"] = query.value("passenger_name");
            booking["passenger_passport"] = query.value("passenger_passport");
            booking["status"] = query.value("status");
            booking["flight_number"] = query.value("flight_number");
            booking["airline_name"] = query.value("airline_name");
            booking["departure_code"] = query.value("departure_code");
            booking["departure_city"] = query.value("departure_city");
            booking["arrival_code"] = query.value("arrival_code");
            booking["arrival_city"] = query.value("arrival_city");
            booking["departure_time"] = query.value("departure_time");
            booking["arrival_time"] = query.value("arrival_time");
            
            results.append(booking);
        }
    } else {
        qDebug() << "Error getting user bookings:" << query.lastError().text();
    }
    
    return results;
}

int Database::registerUser(const QString& username, const QString& password, 
                         const QString& email, const QString& fullName)
{
    // Check if username already exists
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        qDebug() << "Username already exists:" << username;
        return -1;
    }
    
    // Hash password
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    QString hashedPassword = QString(hash.result().toHex());
    
    // Insert new user
    query.prepare("INSERT INTO users (username, password, email, full_name, registration_date) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashedPassword);
    query.addBindValue(email);
    query.addBindValue(fullName);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    
    if (query.exec()) {
        return query.lastInsertId().toInt();
    } else {
        qDebug() << "Error registering user:" << query.lastError().text();
        return -1;
    }
}

int Database::authenticateUser(const QString& username, const QString& password)
{
    QSqlQuery query;
    query.prepare("SELECT id, password FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        QString storedPassword = query.value("password").toString();
        
        // Hash input password
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(password.toUtf8());
        QString hashedPassword = QString(hash.result().toHex());
        
        if (storedPassword == hashedPassword) {
            return query.value("id").toInt();
        }
    }
    
    return -1;
}

QMap<QString, QVariant> Database::getUserProfile(int userId)
{
    QMap<QString, QVariant> result;
    
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE id = ?");
    query.addBindValue(userId);
    
    if (query.exec() && query.next()) {
        result["id"] = query.value("id");
        result["username"] = query.value("username");
        result["email"] = query.value("email");
        result["full_name"] = query.value("full_name");
        result["registration_date"] = query.value("registration_date");
    } else {
        qDebug() << "Error getting user profile:" << query.lastError().text();
    }
    
    return result;
}

bool Database::updateUserProfile(int userId, const QString& email, const QString& fullName)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET email = ?, full_name = ? WHERE id = ?");
    query.addBindValue(email);
    query.addBindValue(fullName);
    query.addBindValue(userId);
    
    if (query.exec()) {
        return true;
    } else {
        qDebug() << "Error updating user profile:" << query.lastError().text();
        return false;
    }
} 