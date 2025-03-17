#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QString>

/**
 * @brief The Database class handles all database operations
 */
class Database : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance of the Database
     * @return Database instance
     */
    static Database* getInstance();

    /**
     * @brief Initialize the database connection
     * @return True if successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Close the database connection
     */
    void close();

    /**
     * @brief Search for flights based on criteria
     * @param departureCity Departure city
     * @param arrivalCity Arrival city
     * @param departureDate Departure date
     * @param returnDate Optional return date for round trips
     * @return List of flights matching the criteria
     */
    QList<QMap<QString, QVariant>> searchFlights(const QString& departureCity, 
                                                const QString& arrivalCity, 
                                                const QDate& departureDate, 
                                                const QDate& returnDate = QDate());

    /**
     * @brief Get information about an airport
     * @param airportCode IATA code of the airport
     * @return Map containing airport information
     */
    QMap<QString, QVariant> getAirportInfo(const QString& airportCode);

    /**
     * @brief Get all available airports
     * @return List of airports
     */
    QList<QMap<QString, QVariant>> getAllAirports();

    /**
     * @brief Book a ticket for a flight
     * @param flightId Flight ID
     * @param userId User ID
     * @param seatClass Seat class (Economy, Business, First)
     * @param passengerName Passenger name
     * @param passengerPassport Passenger passport number
     * @return Booking ID if successful, -1 otherwise
     */
    int bookTicket(int flightId, int userId, const QString& seatClass, 
                  const QString& passengerName, const QString& passengerPassport);

    /**
     * @brief Get user bookings
     * @param userId User ID
     * @return List of bookings
     */
    QList<QMap<QString, QVariant>> getUserBookings(int userId);

    /**
     * @brief Register a new user
     * @param username Username
     * @param password Password
     * @param email Email
     * @param fullName Full name
     * @return User ID if successful, -1 otherwise
     */
    int registerUser(const QString& username, const QString& password, 
                    const QString& email, const QString& fullName);

    /**
     * @brief Authenticate a user
     * @param username Username
     * @param password Password
     * @return User ID if successful, -1 otherwise
     */
    int authenticateUser(const QString& username, const QString& password);

    /**
     * @brief Get user profile information
     * @param userId User ID
     * @return Map containing user information
     */
    QMap<QString, QVariant> getUserProfile(int userId);

    /**
     * @brief Update user profile information
     * @param userId User ID
     * @param email Email
     * @param fullName Full name
     * @return True if successful, false otherwise
     */
    bool updateUserProfile(int userId, const QString& email, const QString& fullName);

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    explicit Database(QObject *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~Database();

    /**
     * @brief Create database tables if they don't exist
     */
    void createTables();

    /**
     * @brief Populate database with sample data
     */
    void populateSampleData();

    static Database* instance;
    QSqlDatabase db;
};

#endif // DATABASE_H 