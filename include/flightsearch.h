#ifndef FLIGHTSEARCH_H
#define FLIGHTSEARCH_H

#include <QWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTableView>
#include "database.h"

/**
 * @brief The FlightSearch class provides flight search functionality
 */
class FlightSearch : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit FlightSearch(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~FlightSearch();

signals:
    /**
     * @brief Signal emitted when a flight is selected for booking
     * @param flightId Flight ID
     */
    void flightSelected(int flightId);

public slots:
    /**
     * @brief Update the user ID when login status changes
     * @param userId User ID
     */
    void setUserId(int userId);

private slots:
    /**
     * @brief Search for flights based on the criteria
     */
    void searchFlights();
    
    /**
     * @brief Handle flight selection in the table
     * @param index Selected index
     */
    void onFlightSelected(const QModelIndex &index);

private:
    /**
     * @brief Set up the UI
     */
    void setupUi();
    
    /**
     * @brief Load airports into the combo boxes
     */
    void loadAirports();
    
    /**
     * @brief Display search results in the table
     * @param flights List of flights
     */
    void displaySearchResults(const QList<QMap<QString, QVariant>> &flights);
    
    QComboBox *departureComboBox;
    QComboBox *arrivalComboBox;
    QDateEdit *departureDateEdit;
    QPushButton *searchButton;
    QTableView *flightsTable;
    
    int userId;
    
    Database *db;
};

#endif // FLIGHTSEARCH_H 