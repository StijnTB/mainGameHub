#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>
#include <QString>
#include <QUrl>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QNetworkReply>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QJsonObject oldGamesInformation;
    QJsonObject newGamesInformation;
    QJsonObject loadedGames;
    QList<QString> updateGames;
    QList<QPushButton *> updateButtons;
    QList<QPushButton *> Buttons;
    QPushButton * currentFocusButton;
    int currentFocusIndex;
    QHBoxLayout *layout;
    QScrollArea *scrollArea;
    QList<int> buttonXCoordinates;
    int spacing = 20;
    int buttonSize = 200;
    float resize = 1.1;
    bool userIsAdmin = false;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QNetworkReply* requestData(QUrl url);
    void parseGameJson(const QByteArray &data);
    void fetchGameData();
    void downloadImage(QPushButton *currentButton, QString gameName);
    void downloadGame(const QUrl &url, QString gameName, QString newVersion, QPushButton *currentButton);
    void executeSelectedGame(const QString gameName);
    void processNextUpdate();
    void moveFocus(int direction);
    void changeFocus(QPushButton *button, int setter);
    void checkGamepad();
    void setupPythonGame(QString gameName);
    void createOverlay();
    void showOverlay();
    void GamepadMovementToScroll(int axis, int value);
    void loadLocalGames(QList<QString> onlyLocalGames);
    void createStatusBar();
    void onLoginButtonClick();
private slots:


private:
    Ui::MainWindow *ui;
    QWidget *loadingOverlay = nullptr;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event);
    //void keyPressEvent(QKeyEvent *event);
};


#endif // MAINWINDOW_H
