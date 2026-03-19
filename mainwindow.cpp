#include <SDL2/SDL.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QList>
#include <QString>
#include <QFile>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QApplication>
#include <QStatusBar>
#include <QInputDialog>

#include <QFile>
#include <QDir>
#include <QProcess>

#include <QCryptographicHash>

#include <QEvent>
#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValueRef>

#include <QKeyEvent>
#include <Qt>
#include <QTimer>

QList<QString> getLocalGames() {
    // Returns a list of all games in the /games directory
    QString currentLocation = QDir::currentPath();
    qDebug() << currentLocation;
    QDir dir(currentLocation+"/games");
    QList<QString> localGames;
    for (const QString &dirName : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        qDebug() << dirName;
        localGames.append(dirName);
    };
    qDebug() << "local games: " << localGames;
    return localGames;
}

QJsonObject loadJsonFile(QString filePath) {
    // Returns the data from the JSON file at filePath
    QFile JsonFile(filePath);
    QJsonObject JsonData;
    if (JsonFile.exists() && JsonFile.open(QIODevice::ReadOnly)) {
        JsonData = (QJsonDocument::fromJson(JsonFile.readAll())).object();
    } else {
        JsonData = {};
    }
    return JsonData;
}

QNetworkReply* MainWindow::requestData(QUrl url) {
    // Returns the reply object for a network request
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = manager -> get(request);
    return reply;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (this -> userIsAdmin) {
        event -> accept();
    } else {
        event -> ignore();
    }
}

void MainWindow::GamepadMovementToScroll(int axis, int value) {
    // Moves focus of scrollArea based on axis and value input from controller
    if (axis == 0 && abs(value) > 10000) {
        this -> moveFocus(value/abs(value));
    } else if (axis == 1 && abs(value) > 10000) {
        if (value > 0) {
            this -> moveFocus(this -> loadedGames.keys().length() -this -> currentFocusIndex - 1);
        } else {
            this -> moveFocus(-1 * this -> currentFocusIndex);
        }
    }
}

void MainWindow::checkGamepad() {
    // Checks for controller events to initiate scrolling and selecting actions
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_JOYAXISMOTION) {
            // Axis 0 is usually the D-Pad Left/Right
            this -> GamepadMovementToScroll(event.jaxis.axis, event.jaxis.value);
        }

        if (event.type == SDL_JOYBUTTONDOWN) {
            if (event.jbutton.button == 8) { // button 8 is SELECT
                this -> executeSelectedGame(this -> loadedGames.keys()[this -> currentFocusIndex]);
            }
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    // catches certain keyEvents to prevent immediate interaction with object focus
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Left) {
            qDebug() << "left";
            // Left arrow -> move select to the left
            this->moveFocus(-1);
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Right) {
            // Right arrow -> move select to the right
            qDebug() << "right";
            this->moveFocus(1);
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Up) {
            // Up arrow -> move select to first button
            qDebug() << "up";
            this -> moveFocus(-1 * this -> currentFocusIndex);
        }
        else if (keyEvent -> key() == Qt::Key_Down) {
            // Down arrow -> move select to last button
            qDebug() << "down";
            this -> moveFocus(1 * (this -> loadedGames.keys().length() - this -> currentFocusIndex - 1));
        }
        else if (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Return) {
            // Space / Return -> run selected game
            this -> executeSelectedGame(this -> loadedGames.keys()[this -> currentFocusIndex]);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::moveFocus(int directionalMove) {
    // Moves the scrollArea a certain amount of directionalMove
    int currentSelectedIndex = this -> currentFocusIndex; // The index of the button currently in focus
    qDebug() << currentSelectedIndex;
    qDebug() << directionalMove;
    if (currentSelectedIndex + directionalMove >= 0 && currentSelectedIndex + directionalMove < this -> Buttons.length()) { // moving the selector is possible
        int currentValue = this -> scrollArea -> horizontalScrollBar() -> value(); // The current horizontal value of the scrollbar
        this -> scrollArea -> horizontalScrollBar() -> setValue(currentValue + directionalMove * (this -> buttonSize + this -> spacing)); // move the scrollbar
        QPushButton *oldFocusButton = this -> currentFocusButton; // load old selected button and unfocus
        this -> changeFocus(oldFocusButton, -1);
        QPushButton *newFocusButton = this -> Buttons[currentSelectedIndex + directionalMove]; // load new selected button and focus
        this -> changeFocus(newFocusButton, 1);

        this -> currentFocusIndex = currentSelectedIndex = currentSelectedIndex + directionalMove; // update save of current selected index
        this -> currentFocusButton = newFocusButton; // update save of current selected button
    };
}

void MainWindow::changeFocus(QPushButton *button, int setter) {
    // Switches the focus for a button to change visuals
    if (setter == -1) {
        qDebug() << "unfocus button";
        //currently has focus, so revert to normal state
        button -> setFixedSize(this -> buttonSize,this -> buttonSize);
        button -> setContentsMargins(0,0,0,0);
        button -> setIconSize(QSize(this -> buttonSize,this -> buttonSize));
    } else if (setter == 1) {
        qDebug() << "focus button";
        //currently doesn't have focus, so set focus
        button -> setFixedSize(this -> buttonSize * this -> resize,this -> buttonSize * this -> resize);
        button -> setContentsMargins(0,0,0,0);
        button -> setIconSize(QSize(this -> buttonSize * this -> resize,this -> buttonSize * this -> resize));
    };
    button -> update(); // update visual of button
}

void MainWindow::fetchGameData() {
    // Requests newest digital game data and executes parseGameJson when finished
    QUrl url("https://raw.githubusercontent.com/StijnTB/Corderius_Games/refs/heads/main/local_games_information.json");
    QNetworkReply *reply = this -> requestData(url);
    connect(reply, &QNetworkReply::finished, [this, reply]() { // when request is loaded, execute
        if (reply -> error() == QNetworkReply::NoError) {
            // if no error has occurred, parse data
            this -> parseGameJson(reply -> readAll());
        } else { // error has occurred, don't parse data; just output
            qDebug() << "Network Error: " << reply -> errorString();
            return;
        }
        reply -> deleteLater(); // delete reply when safe
    });
}

void MainWindow::parseGameJson(const QByteArray &data) {
    this -> Buttons = QList<QPushButton *>();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    QJsonObject root = doc.object();
    QFile currentGamesInformation("current_games_information.json");
    if (!currentGamesInformation.exists()) {
        if (currentGamesInformation.open(QIODevice::WriteOnly)) {
            currentGamesInformation.write("");
            currentGamesInformation.close();
        };
    };
    this -> oldGamesInformation = loadJsonFile("current_games_information.json");
    QList<QString> onlyLocalGames = getLocalGames();
    this -> newGamesInformation = root;
    QDir().mkdir("games");
    for (const QString &gameName : root.keys()) {
        this -> loadedGames[gameName] = "loaded";
        if (onlyLocalGames.contains(gameName)) {
            onlyLocalGames.removeAt(onlyLocalGames.indexOf(gameName));
        };
        QPushButton *currentButton = new QPushButton("Loading "+ gameName);
        currentButton->setMaximumSize((QSize(this -> buttonSize,this -> buttonSize)));
        currentButton->setContentsMargins(0,0,0,0);
        currentButton -> installEventFilter(this);
        currentButton -> setFocusPolicy(Qt::NoFocus);
        if (this -> Buttons.isEmpty()) {
            this -> changeFocus(currentButton, 1);
            this -> currentFocusButton = currentButton;
        };
        this -> Buttons.append(currentButton);

        QJsonObject gameObject = root[gameName].toObject();
        QString currentVersion = this->oldGamesInformation[gameName].toObject()["version"].toString();
        QString newVersion = gameObject["version"].toString();
        if (currentVersion != newVersion) {
            currentButton -> setEnabled(false);
            this -> updateGames.append(gameName);
            this -> updateButtons.append(currentButton);
            this -> loadedGames[gameName] = "unloaded";
            
        };
        this -> layout -> addWidget(currentButton);

        connect(currentButton, &QPushButton::clicked, this, [this, gameName, currentButton]() {
            this -> scrollArea -> horizontalScrollBar() -> setValue((currentButton -> geometry().center().x()) - ((this -> scrollArea -> viewport() -> width()) / 2));
            qDebug() << "Button" << gameName << "was clicked! Loading game ...";
            this -> executeSelectedGame(gameName);
        });
        this -> downloadImage(currentButton, gameName);
    };
    this -> processNextUpdate();
    this -> loadLocalGames(onlyLocalGames);
    //this -> currentFocusButton->setFocus();
    this -> layout -> addSpacing((this -> width() - this -> spacing * 2 - buttonSize)/2 - this -> spacing);
}

void MainWindow::loadLocalGames(QList<QString> onlyLocalGames) {
    for (const QString &gameName : onlyLocalGames) {
        QJsonObject gameData = loadJsonFile("games/"+gameName+"/"+gameName+"/gameInformation.json");
        QPushButton *currentButton = new QPushButton(gameName);
        currentButton -> setMaximumSize((QSize(this -> buttonSize, this -> buttonSize)));
        currentButton -> setFixedSize(this -> buttonSize, this -> buttonSize);
        currentButton -> setContentsMargins(0,0,0,0);
        currentButton -> setFocusPolicy(Qt::NoFocus);
        if (QFile::exists("games/"+gameName+"/"+gameName+"/"+gameData["iconPath"].toString())) {
            QIcon gameIcon("games/"+gameName+"/"+gameName+"/"+gameData["iconPath"].toString());
            currentButton -> setIcon(gameIcon);
        }
        if (Buttons.isEmpty()) {
            this -> changeFocus(currentButton, 1);
            this -> currentFocusButton = currentButton;
        }
        this -> layout -> addWidget(currentButton);
        this -> Buttons.append(currentButton);
        connect(currentButton, &QPushButton::clicked, this, [this, gameName, currentButton]() {
            this -> scrollArea -> horizontalScrollBar() -> setValue((currentButton -> geometry().center().x()) - ((this -> scrollArea -> viewport() -> width()) / 2));
            qDebug() << "Button" << gameName << "was clicked! Loading game ...";
            this -> executeSelectedGame(gameName);
        });
    };
}

void MainWindow::downloadImage(QPushButton *targetButton, QString gameName) {
    QUrl iconUrl(this ->newGamesInformation[gameName].toObject()["thumbnail_link"].toString());
    QList<QString> urlParts = iconUrl.toString().split(".");
    QString iconFileName = gameName+"_icon."+ urlParts[urlParts.length() - 1];
    if (QFile::exists("icons/"+iconFileName)) {
        QIcon gameIcon("icons/"+iconFileName);
        targetButton -> setIcon(gameIcon);
    };
    QNetworkReply *reply = this -> requestData(iconUrl);
    connect(reply, &QNetworkReply::finished, [this, reply, iconUrl, iconFileName, gameName, targetButton]() {
        // when reply has loaded, execute
        if (reply -> error() == QNetworkReply::NoError) {
            // update the button icon
            QByteArray iconData = reply -> readAll();
            QPixmap pixmap;
            pixmap.loadFromData(iconData);
            QIcon buttonIcon(pixmap.scaled(QSize(512,512)));
            targetButton -> setIcon(buttonIcon);
            targetButton -> setIconSize(QSize(this -> buttonSize,this -> buttonSize));

            targetButton -> setText(gameName);
            targetButton -> update();

            // save the button icon for quickload
            //QString fileName = urlParts[urlParts.length() - 2] + "_" + urlParts[urlParts.length() - 1];
            QDir().mkdir("icons");
            QString filePath = "icons/" + iconFileName;
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(iconData);
                file.close();
            };
        } else {
            qDebug() << "error with request " + iconUrl.toString();
        }
        reply -> deleteLater();
    });
}

void MainWindow::processNextUpdate() {
    if (this -> updateGames.isEmpty()) {
        qDebug() << "All updates finished!";
        return;
    };

    // Get the first game title from the list
    QString nextGame = this -> updateGames[0];
    
    // Start the update process for this specific game
    QJsonObject nextGameInformation = this -> newGamesInformation[nextGame].toObject();

    QUrl nextGameUrl = nextGameInformation["download_newest_version_link"].toString();
    QString newVersion = nextGameInformation["version"].toString();
    QPushButton *currentButton = this -> updateButtons[0];
    this -> downloadGame(nextGameUrl, nextGame, newVersion, currentButton);
}

void MainWindow::downloadGame(const QUrl &url, QString gameName, QString newVersion, QPushButton *currentButton) {
    QNetworkReply *reply = this -> requestData(url);

    connect(reply, &QNetworkReply::finished, [this, reply, gameName, currentButton, newVersion]() {
        if (reply -> error() == QNetworkReply::NoError) {
            QString mainDirectory = QDir::currentPath();
            QJsonObject localGameInformation = this -> oldGamesInformation[gameName].toObject();
            QByteArray savefileData;
            if (localGameInformation.keys().contains("savefile_path")) {
                QString localSavefilePath = localGameInformation["savefile_path"].toString();
                QJsonObject localSavefileObject = loadJsonFile(QDir::currentPath() + "/games/"+ gameName + "/" + gameName + "/" + localSavefilePath);
                //savefileData = localSavefileObject.
                QFile localSavefile(QDir::currentPath()+"/games/"+gameName+"/"+gameName+"/"+localSavefilePath);
                if (localSavefile.open(QIODevice::ReadOnly)) {
                    savefileData = localSavefile.readAll();
                    localSavefile.close();
                };
            };

            QDir().remove((mainDirectory+"/games/"+gameName));
            QByteArray zipData = reply -> readAll();
            QString tempZip = mainDirectory+"/games/temp_game_"+gameName+".zip";
            QFile file(tempZip);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(zipData);
                file.close();
                QString program;
                QList<QString> arguments;
                QProcess *unzipProcess = new QProcess(this);
                unzipProcess->setWorkingDirectory(QDir::currentPath());
                #ifdef Q_OS_WIN
                    program = "powershell";
                    arguments << "-Command" << "Expand-Archive -Path '" + tempZip + "' -DestinationPath 'games/"+gameName + "' -Force";
                #else
                    program = "unzip";
                    arguments << "'" + tempZip + "'-d" << "games/"+gameName;
                #endif

                unzipProcess->start(program, arguments);

                connect(unzipProcess, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
                    if (error == QProcess::FailedToStart) {
                        qDebug() << "Error: The 'unzip' command was not found on this system.";
                    }
                });
                connect(unzipProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, unzipProcess, gameName, currentButton, newVersion, savefileData](int exitCode, QProcess::ExitStatus exitStatus) {
                    if (exitStatus == QProcess::CrashExit) {
                        qDebug() << "unzip has crashed";
                    } else if (exitCode != 0) {
                        qDebug() << "Unzip failed with error code:"<<exitCode;
                        qDebug() << "error details:"<<unzipProcess->readAllStandardError();
                    } else {
                        qDebug() << "unzip successful";
                        this -> loadedGames[gameName] = "loaded";
                        if (!this->updateGames.isEmpty()) {
                            this -> updateGames.pop_front();
                            this -> updateButtons.pop_front();
                        };
                        if (this -> newGamesInformation[gameName].toObject().keys().contains("savefile_path")) {
                            QFile newSavefile(QDir::currentPath() + "/games/" + gameName + "/" + gameName + "/" +this -> newGamesInformation[gameName].toObject()["savefile_path"].toString());
                            if (newSavefile.open(QIODevice::WriteOnly)) {
                                newSavefile.write(savefileData);
                                newSavefile.close();
                            };
                        };
                        currentButton -> setText("");
                        this->processNextUpdate();
                        QFile currentGamesInformationFile("current_games_information.json");
                        if (currentGamesInformationFile.open(QIODevice::ReadWrite)) {
                            QByteArray currentGamesInformationBytes = currentGamesInformationFile.readAll();
                            QJsonDocument currentGamesInformationDoc = QJsonDocument::fromJson(currentGamesInformationBytes);
                            QJsonObject currentGamesInformation = currentGamesInformationDoc.object();
                            currentGamesInformationFile.resize(0); //clear file
                            currentGamesInformation.insert(gameName, this -> newGamesInformation[gameName].toObject());
                            QJsonDocument updatedDocument(currentGamesInformation);
                            currentGamesInformationFile.write(updatedDocument.toJson(QJsonDocument::Indented));
                            currentGamesInformationFile.close();
                        };
                        currentButton -> setEnabled(true);
                    }
                    QFile::remove("games/temp_game.zip");
                    unzipProcess -> deleteLater();
                });
            };
        };
        reply -> deleteLater();
    });
}

void MainWindow::setupPythonGame(QString gameName) {
    QProcess *setupGame = new QProcess(this);
    setupGame -> setWorkingDirectory("games/"+gameName+"/"+gameName);
    QString pythonPath;
    QString command;
    #ifdef Q_OS_WIN
        // Windows Logic
        pythonPath = setupGame->workingDirectory() + "/venv/Scripts/python.exe";
        command = "python -m venv venv && "
                  "\"" + setupGame->workingDirectory() + "/venv/Scripts/pip.exe\" install -r requirements.txt";
    #else
        // Raspberry Pi / Linux Logic
        pythonPath = setupGame -> workingDirectory() + "/venv/bin/python3";
        command = "python3 -m venv venv && "
                  "./venv/bin/pip install -r requirements.txt";
    #endif

    if (!QFile::exists(pythonPath)) {
        #ifdef Q_OS_WIN
                setupGame->start("cmd", QStringList() << "/c" << command);
        #else
                setupGame->start("sh", QStringList() << "-c" << command);
        #endif
    };

    connect (setupGame, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, gameName]() {
        qDebug() << "setup for game " + gameName + " complete.";
        this -> executeSelectedGame(gameName);
    });
}

void MainWindow::executeSelectedGame(const QString gameName) {
    if (!(this -> loadedGames[gameName].toString() == "loaded")) {
        qDebug() << "game "+ gameName + " not yet loaded.";
        return;
    };
    QJsonObject gameData;
    if (this -> newGamesInformation.contains(gameName)) {
        //get executable path through newGamesInformation dict
        gameData = this -> newGamesInformation[gameName].toObject();
    } else {
        //get executable path through gameInformation.json
        gameData = loadJsonFile("games/"+gameName+"/"+gameName+"/gameInformation.json");
    };
    QString gameExecutablePath = gameData["executable_file_path"].toString();
    QString executableFileType = gameData["executable_file_type"].toString();

    if (!QFile::exists("games/" + gameName + "/" + gameName + "/" + gameExecutablePath)) {
        qDebug() << "could not find executable file at relative path 'games/" + gameName + "/" + gameName + "/" + gameExecutablePath + "'.";
        return;
    };

    if (executableFileType != ".py") {
        qDebug() << "unknown executable type for game " + gameName + ": '" + executableFileType + "'. unable to load game";
        return;
    };

    QString baseDir = QDir::currentPath() + "/games/" + gameName + "/" + gameName;
    QProcess *executeGameProcess = new QProcess(this);
    executeGameProcess -> setWorkingDirectory(baseDir);

    if (executableFileType == ".py") {
        QDir venvDir(baseDir + "/venv");
        if (venvDir.exists()) {
            qDebug() << "virtual environment /venv found; execute game process.";
            this -> showOverlay();
            executeGameProcess -> start(QString("./venv/bin/python3"), {(baseDir + "/" + gameExecutablePath)});
        } else {
            qDebug() << "virtual environment folder /venv missing; start installation.";
            this -> setupPythonGame(gameName);
        };
    };
    connect(executeGameProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, executeGameProcess, gameName](int exitCode) {
        qDebug() << "Game " + gameName + " exited with code:" << exitCode;
        this -> loadingOverlay -> hide();
        executeGameProcess -> deleteLater();
    });
    connect(executeGameProcess, &QProcess::errorOccurred, [this, executeGameProcess, gameName](QProcess::ProcessError error) {
        qDebug() << "Failed to launch game "+ gameName + " due to error:" << error;
        this -> loadingOverlay -> hide();
        executeGameProcess -> deleteLater();
    });
    connect(executeGameProcess, &QProcess::readyReadStandardError, this, [executeGameProcess]() {
        QString error = executeGameProcess -> readAllStandardError();
        qDebug() << "exited with error: "<< error;
    });
}

void MainWindow::createOverlay() {
    if (!loadingOverlay) {
        loadingOverlay = new QWidget(this);
        loadingOverlay -> setStyleSheet("background-color: black;");
        loadingOverlay -> hide();
    };
}

void MainWindow::showOverlay() {
    if (loadingOverlay) {
        loadingOverlay -> setGeometry(0,0, this->width(), this -> height());
        loadingOverlay -> raise();
        loadingOverlay -> show();
    }
}

void setupSDLJoystick() {
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        qDebug() << "SDL could not initialize! Error:" << SDL_GetError();
    } else {
        int numJoysticks = SDL_NumJoysticks();
        qDebug() << "Number of joysticks connected:" << numJoysticks;
        for (int i = 0; i < numJoysticks; i++) {
            qDebug() << "Joystick" << i << ":" << SDL_JoystickNameForIndex(i);
            SDL_JoystickOpen(i);
        }
    }
}

void MainWindow::createStatusBar() {
    QStatusBar *mainStatusBar = new QStatusBar(this);
    QPushButton *adminLoginButton = new QPushButton(QString("Login"));
    adminLoginButton->setFixedSize(QSize(100,50));
    connect(adminLoginButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "login ...";
        this -> onLoginButtonClick();
    });
    adminLoginButton -> setFocusPolicy(Qt::NoFocus);
    mainStatusBar->insertPermanentWidget(0, adminLoginButton);
    this->setStatusBar(mainStatusBar);
}

void saveAdminPassword(QString textPassword) {
    QByteArray hash = QCryptographicHash::hash(textPassword.toUtf8(), QCryptographicHash::Sha256);
    QFile file("admin_access.dat");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(hash.toHex());
        file.close();
    };
}

bool checkPassword(QString passwordAttempt) {
    QFile file("admin_access.dat");
    if (!file.exists()) {
        saveAdminPassword(passwordAttempt);
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) return false;
    QByteArray storedHash = file.readAll();
    file.close();
    QByteArray inputHash =  QCryptographicHash::hash(passwordAttempt.toUtf8(), QCryptographicHash::Sha256).toHex();
    return (inputHash == storedHash);
}

void MainWindow::onLoginButtonClick() {
    QInputDialog *passwordInputBox = new QInputDialog(this);
    passwordInputBox->setFixedSize(QSize(this -> buttonSize,this -> buttonSize));
    passwordInputBox->setGeometry(((this -> geometry().x()) + 200),((this->geometry().y()) + 200),200,200);
    passwordInputBox->setLabelText(QString("Enter password"));
    passwordInputBox->setOkButtonText(QString("Login"));
    passwordInputBox->show();
    passwordInputBox->raise();
    passwordInputBox->setFocusPolicy(Qt::StrongFocus);
    passwordInputBox->setFocus();
    passwordInputBox->installEventFilter(this);
    connect(passwordInputBox, &QInputDialog::accepted, [this, passwordInputBox]() {
        QString passwordInput = passwordInputBox->textValue();
        qDebug() << "button clicked. input was: " << passwordInput;
        bool passwordWasCorrect = checkPassword(passwordInput);
        qDebug() << "password correct: " << passwordWasCorrect;
        if (passwordWasCorrect) {
            this -> userIsAdmin = true;
        };

    });
    connect(passwordInputBox, &QInputDialog::rejected, []() {
        qDebug() << "canceled input.";
    });
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this -> showFullScreen();
    QSize windowSize = qApp->screens()[0]->size();
    ui->setupUi(this);
    setupSDLJoystick();

    
    this -> createOverlay();

    this -> scrollArea = new QScrollArea(this);
    this -> scrollArea -> setWidgetResizable(true);
    this -> scrollArea -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this -> scrollArea -> setFrameShape(QFrame::NoFrame);
    // this -> scrollArea -> setFocusPolicy(Qt::NoFocus);
    this -> scrollArea -> horizontalScrollBar() ->setDisabled(true);

    QWidget *container = new QWidget();
    this -> layout = new QHBoxLayout(container);
    this -> layout -> setContentsMargins(this -> spacing, this -> spacing, this -> spacing, this -> spacing);
    this -> scrollArea -> setGeometry(QRect(this -> spacing, this -> buttonSize, (windowSize.width() - this -> spacing * 2), (windowSize.height() - this -> buttonSize * 2)));
    this->fetchGameData();

    int centerPadding = (windowSize.width() - this -> spacing * 2 - buttonSize)/2 - this -> spacing;
    this -> layout -> setSpacing(this -> spacing);
    this -> layout -> addSpacing(centerPadding);

    this -> scrollArea -> horizontalScrollBar() -> setValue(centerPadding + this -> spacing + buttonSize/2);

    this -> scrollArea -> setWidget(container);
    this->setCentralWidget(this -> scrollArea);
    this->createStatusBar();
    QTimer *controllerTimer = new QTimer(this);
    controllerTimer->setSingleShot(false);
    connect(controllerTimer, &QTimer::timeout, this, &MainWindow::checkGamepad);
    controllerTimer->start(1);

    this -> scrollArea -> setFocus();
    this -> scrollArea -> installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}