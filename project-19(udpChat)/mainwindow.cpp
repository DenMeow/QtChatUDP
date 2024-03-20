#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) // Конструктор
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(nullptr) //инициализировали сокет nullptr
{
    ui -> setupUi(this);
    setWindowTitle("QtGram");
    lock(true);

    ui -> createButton -> setStyleSheet("color: green");
    ui -> disconnectButton -> setStyleSheet("color: red");
    ui -> inputEdit -> setPlaceholderText("Введите сообщение...");
}

MainWindow::~MainWindow() // Деструктор
{
    if (socket) {
        // Отправляем прощальное сообщение
        QString leaveMessage = "Пользователь " + userName + "  отключился";
        DataToSend leaveData;
        leaveData.dateTime = QDateTime::currentDateTime();
        leaveData.message = leaveMessage;
        leaveData.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << leaveData; // Сериализация данных

        socket->writeDatagram(buffer_array, previousChatAddress, previousChatPort);

        socket->close();
        delete socket;
    }
    delete ui;
}

void MainWindow::keyPressEvent( QKeyEvent * event ) // Обработка нажатия клавиши Enter
{
    if ((event -> key() == Qt::Key_Return) and send)
        on_sendButton_clicked();
}

void MainWindow::lock(bool set) //Управление блокировок кнопок и ввода ip/портов
{
    ui -> nameEdit -> setEnabled(set);
    ui -> colorButton -> setEnabled(set);

    ui -> localAddressEdit -> setEnabled(set);
    ui -> localPortEdit -> setEnabled(set);

    ui -> remoteAddressEdit -> setEnabled(set);
    ui -> remotePortEdit -> setEnabled(set);

    ui -> sendButton -> setEnabled(!set);
    ui -> inputEdit -> setEnabled(!set);

    ui -> createButton -> setEnabled(set);
    ui -> disconnectButton -> setEnabled(!set);
}


void MainWindow::on_createButton_clicked() //Подтверждение ip и порта
{
    if(socket != nullptr)
        socket -> deleteLater();

    if(ui -> nameEdit -> text().size() == 0) {
        QMessageBox::information(this, "Ошибка", "Для начала введите имя!");
        return;
    }

    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead,
            this, &MainWindow::socketReadyRead);

    QHostAddress localAddress;
    QHostAddress remoteAddress;
    userName = ui -> nameEdit -> text();

    if (!remoteAddress.setAddress(ui->remoteAddressEdit->text()) or !localAddress.setAddress(ui->localAddressEdit->text())) {
        QMessageBox::warning(this, "Ошибка!", "Вы ввели некорректный IP адрес");
        return;
    }

    ushort localPort = (ushort)ui -> localPortEdit -> value();
    ushort remotePort = (ushort)ui->remotePortEdit->value();

    if(!socket -> bind(localAddress, localPort)) {
        QMessageBox::information(this,"Ошибка","Не удалось привязать сокет к указанному адресу и порту");
        return;
    }

    ui -> chatEdit -> clear();

    // Отправляем сообщение-приветсвтие
    QString welcomeMessage = "Пользователь " + userName + "  подключился к вам!";
    DataToSend welcomeData;
    welcomeData.dateTime = QDateTime::currentDateTime();
    welcomeData.message = welcomeMessage;
    welcomeData.name = userName;

    QByteArray buffer_array;
    QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
    dataStream << welcomeData; // Сериализация данных

    previousChatAddress = remoteAddress;
    previousChatPort = remotePort;
    socket->writeDatagram(buffer_array, remoteAddress, remotePort);

    // Отобразите приветственное сообщение в чате
    ui -> chatEdit -> appendHtml("Вы подключились к чату!");

    lock(false);
    ui -> sendButton -> setEnabled(false);
}

void MainWindow::on_disconnectButton_clicked() //Отключение сокета
{
    // Отправляем прощальное сообщение
    QString leaveMessage = "Пользователь " + userName + "  отключился";
    DataToSend leaveData;
    leaveData.dateTime = QDateTime::currentDateTime();
    leaveData.message = leaveMessage;
    leaveData.name = userName;

    QByteArray buffer_array;
    QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
    dataStream << leaveData; // Сериализация данных

    socket->writeDatagram(buffer_array, previousChatAddress, previousChatPort);

    ui -> chatEdit -> appendHtml("Вы отлючились от чата :(");

    if(socket != nullptr) {
        socket -> close();
        socket -> deleteLater();
        socket = nullptr;
    }

    lock(true);

    ui -> inputEdit -> clear();
}


void MainWindow::on_sendButton_clicked() //Отправление сообщений
{
    if(socket != nullptr) {
        QString text = userName + ": " + ui-> inputEdit -> text();

        // Отправляем обычное сообщение
        DataToSend m_data;
        m_data.color = colorSendMessage.name();
        m_data.message = text;
        m_data.dateTime = QDateTime::currentDateTime();
        m_data.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << m_data; // Сериализация данных

        QHostAddress remoteAddress(ui->remoteAddressEdit->text());
        ushort remotePort = (ushort)ui->remotePortEdit->value();
        socket->writeDatagram(buffer_array, remoteAddress, remotePort);

        QString messageText = text; // Текст сообщения
        QColor messageColor = colorSendMessage; // Цвет сообщения

        QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                       .arg(messageColor.name())
                                       .arg(m_data.dateTime.toString("hh:mm:ss"))
                                       .arg(messageText);

        ui -> chatEdit -> appendHtml(formattedMessage);

        ui -> inputEdit -> clear();
    }
}

void MainWindow::socketReadyRead() //Прием сообщений
{
    while (socket -> hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(socket->pendingDatagramSize());
        QHostAddress remoteAddress;
        ushort remotePort;

        int bytesRead = socket->readDatagram(buffer.data(), buffer.size(), &remoteAddress, &remotePort);

        if (bytesRead == -1) {
            QMessageBox::information(this," (>﹏<) ","На другом конце никого нет");
        }
        else {
            // Данные успешно прочитаны
            QDataStream dataStream(&buffer, QIODevice::ReadOnly);
            DataToSend receivedData;
            dataStream >> receivedData;

            QString messageText = receivedData.message;
            QColor messageColor = receivedData.color;

            QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                           .arg(messageColor.name())
                                           .arg(receivedData.dateTime.toString("hh:mm:ss"))
                                           .arg(messageText);

            ui -> chatEdit -> appendHtml(formattedMessage);
        }
    }
}

void MainWindow::on_inputEdit_textChanged() //Проверка что в строке отправки есть символы
{
    QString text = ui -> inputEdit -> text();
    if (text.trimmed().isEmpty()) {
        ui -> sendButton -> setEnabled(false);
        send = false;
    }
    else {
        ui -> sendButton -> setEnabled(true);
        send = true;
    }
}

void MainWindow::on_colorButton_clicked() //Выбор цвета сообщений
{
    colorSendMessage = QColorDialog::getColor(QColor(Qt::white),this,"Выберите цвет фона");
}


