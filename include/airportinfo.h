#ifndef AIRPORTINFO_H
#define AIRPORTINFO_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include "database.h"

/**
 * @brief The AirportInfo class provides airport information
 */
class AirportInfo : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit AirportInfo(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~AirportInfo();
    
    /**
     * @brief Load flights for the selected airport
     * @param airportCode Airport IATA code
     */
    void loadFlights(const QString &airportCode);

private slots:
    /**
     * @brief Load information for the selected airport
     */
    void loadAirportInfo();
    
    /**
     * @brief Handle airport selection change
     * @param index Selected index
     */
    void onAirportSelectionChanged(int index);

private:
    /**
     * @brief Set up the UI
     */
    void setupUi();
    
    /**
     * @brief Load airports into the combo box
     */
    void loadAirports();
    
    QComboBox *airportComboBox;
    QPushButton *loadButton;
    QLabel *nameLabel;
    QLabel *codeLabel;
    QLabel *cityLabel;
    QLabel *countryLabel;
    QLabel *timeZoneLabel;
    QTableView *flightsTable;
    
    Database *db;
};

#endif // AIRPORTINFO_H 