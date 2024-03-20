#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QMessageBox>
#include <QDateTime>
#include <QColorDialog>

struct DataToSend { // Структура для передачи данных
    QString name;
    QString message;
    QColor color;
    QDateTime dateTime;

    // Оператор для сериализации
    friend QDataStream &operator << (QDataStream &out, const DataToSend &data) {
        out << data.name;
        out << data.message;
        out << data.color;
        out << data.dateTime;
        return out;
    }

    // Оператор для десериализации
    friend QDataStream &operator >> (QDataStream &in, DataToSend &data) {
        in >> data.name;
        in >> data.message;
        in >> data.color;
        in >> data.dateTime;
        return in;
    }

    // "friend" указывает на то, что эта функция будет дружественной для структуры,
    // и она имеет доступ к его закрытым членам.
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_createButton_clicked();

    void on_sendButton_clicked();

    void socketReadyRead();

    void lock(bool set);

    void on_inputEdit_textChanged();

    void on_disconnectButton_clicked();

    void on_colorButton_clicked();

    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    QUdpSocket *socket;
    QString userName;
    QColor colorSendMessage = Qt::red;
    QColor colorRecievedMessage = Qt::blue;
    QHostAddress previousChatAddress; // Адрес предыдущего подключения
    ushort previousChatPort; // Порт предыдущего подключения
    bool send = false;
};
#endif // MAINWINDOW_H
