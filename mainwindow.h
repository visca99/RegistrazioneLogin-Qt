#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct utente{
    std::string nome;
    std::string cognome;
    std::string email_cellulare;
    std::string password;
    std::string data_nascita;
    std::string sesso;

    utente() {}

    utente(const std::string &n, const std::string &c, const std::string &e, const std::string &p, const std::string &d, const std::string &s) : nome(n), cognome(c), email_cellulare(e), password(p), data_nascita(d), sesso(s) {}

};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_accedi_clicked();

    void on_pushButton_iscriviti_clicked();

    void on_pushButton_recupera_password_clicked();

private:
    Ui::MainWindow *ui;

    std::vector<utente> db_utenti;

    int numero_utenti = 0;

    void salva_nuovo_utente();

    void carica_utenti();

    int eta(const std::string &data_nascita);

    bool check_registrazione(const std::string &n, const std::string &c, const std::string &ec, const std::string &p);

    bool utente_esistente(const std::string &mail_cell);

    std::string check_login(const std::string &mail_cell, const std::string &psw);


};
#endif // MAINWINDOW_H
