#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
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
 * @brief The UserProfile class provides user profile management
 */
class UserProfile : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit UserProfile(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~UserProfile();

public slots:
    /**
     * @brief Set the user ID
     * @param userId User ID
     */
    void setUserId(int userId);
    
    /**
     * @brief Load user profile information
     */
    void loadUserProfile();
    
    /**
     * @brief Load user bookings
     */
    void loadUserBookings();

private slots:
    /**
     * @brief Save user profile changes
     */
    void saveProfile();

private:
    /**
     * @brief Set up the UI
     */
    void setupUi();
    
    /**
     * @brief Display user profile information
     * @param profile User profile information
     */
    void displayUserProfile(const QMap<QString, QVariant> &profile);
    
    /**
     * @brief Display user bookings
     * @param bookings List of bookings
     */
    void displayUserBookings(const QList<QMap<QString, QVariant>> &bookings);
    
    QLabel *usernameLabel;
    QLineEdit *emailEdit;
    QLineEdit *fullNameEdit;
    QLabel *registrationDateLabel;
    QPushButton *saveButton;
    
    QTableView *bookingsTableView;
    QStandardItemModel *bookingsModel;
    
    int currentUserId;
    
    Database *db;
};

#endif // USERPROFILE_H 