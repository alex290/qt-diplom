#ifndef TICKETBOOKING_H
#define TICKETBOOKING_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>
#include "database.h"

/**
 * @brief The TicketBooking class provides ticket booking functionality
 */
class TicketBooking : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit TicketBooking(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~TicketBooking();

public slots:
    /**
     * @brief Set the flight ID for booking
     * @param flightId Flight ID
     */
    void setFlightId(int flightId);
    
    /**
     * @brief Set the user ID for booking
     * @param userId User ID
     */
    void setUserId(int userId);
    
    /**
     * @brief Load user bookings
     */
    void loadUserBookings();

private slots:
    /**
     * @brief Book a ticket
     */
    void bookTicket();
    
    /**
     * @brief Update the price based on selected seat class
     * @param index Selected index
     */
    void updatePrice(int index);

private:
    /**
     * @brief Set up the UI
     */
    void setupUi();
    
    /**
     * @brief Load flight details
     */
    void loadFlightDetails();
    
    /**
     * @brief Display user bookings
     * @param bookings List of bookings
     */
    void displayUserBookings(const QList<QMap<QString, QVariant>> &bookings);
    
    QLabel *flightNumberLabel;
    QLabel *airlineLabel;
    QLabel *departureLabel;
    QLabel *arrivalLabel;
    QLabel *departureDateTimeLabel;
    QLabel *arrivalDateTimeLabel;
    QLabel *priceLabel;
    
    QLineEdit *passengerNameEdit;
    QLineEdit *passengerPassportEdit;
    QComboBox *seatClassComboBox;
    QPushButton *bookButton;
    
    QTableView *bookingsTableView;
    QStandardItemModel *bookingsModel;
    
    int currentFlightId;
    int currentUserId;
    QMap<QString, QVariant> currentFlight;
    
    Database *db;
};

#endif // TICKETBOOKING_H 