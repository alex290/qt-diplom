#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include "database.h"
#include "flightsearch.h"
#include "airportinfo.h"
#include "ticketbooking.h"
#include "userprofile.h"
#include "airportloading.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The MainWindow class is the main application window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~MainWindow();

private slots:
    /**
     * @brief Show the flight search page
     */
    void showFlightSearch();
    
    /**
     * @brief Show the airport information page
     */
    void showAirportInfo();
    
    /**
     * @brief Show the ticket booking page
     */
    void showTicketBooking();
    
    /**
     * @brief Show the user profile page
     */
    void showUserProfile();
    
    /**
     * @brief Show the airport loading page
     */
    void showAirportLoading();
    
    /**
     * @brief Show the login dialog
     */
    void showLoginDialog();
    
    /**
     * @brief Show the registration dialog
     */
    void showRegistrationDialog();
    
    /**
     * @brief Log out the current user
     */
    void logout();
    
    /**
     * @brief Update the UI based on login status
     */
    void updateLoginStatus();
    
    /**
     * @brief Handle user login
     * @param userId User ID
     */
    void onUserLoggedIn(int userId);
    
    /**
     * @brief Show the about dialog
     */
    void showAboutDialog();
    
    /**
     * @brief Show the help dialog
     */
    void showHelpDialog();

private:
    /**
     * @brief Set up the UI
     */
    void setupUi();
    
    /**
     * @brief Create the menu bar
     */
    void createMenuBar();
    
    /**
     * @brief Create the status bar
     */
    void createStatusBar();
    
    /**
     * @brief Create the central widget
     */
    void createCentralWidget();
    
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    FlightSearch *flightSearchPage;
    AirportInfo *airportInfoPage;
    TicketBooking *ticketBookingPage;
    UserProfile *userProfilePage;
    AirportLoadingWidget *airportLoadingPage;
    
    QLabel *statusLabel;
    
    int currentUserId;
    QString currentUsername;
    
    Database *db;
};

#endif // MAINWINDOW_H 