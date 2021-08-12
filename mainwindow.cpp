#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <fstream>
#include <regex>
#include <QDialog>
#include <ctime>
#include <iostream>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QTableWidget>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    carica_utenti();

}


/*
 * Funzione che scrive il file 'utenti.csv' in modalità write-olny, gli utenti(compreso il nuovo utente appena registrato)
 * vengono prelevati dal vector 'db_utenti'
 */
void MainWindow::salva_nuovo_utente(){

    QString fileName = "utenti.csv";
    QFile file(fileName);
    QStringList lista_utenti;
    for(int i = 0; i < numero_utenti; i++){
        std::stringstream sslinea;
        sslinea << (db_utenti[i].nome) << "," << (db_utenti[i].cognome) << "," << (db_utenti[i].email_cellulare) << "," << (db_utenti[i].password) << "," << (db_utenti[i].data_nascita) << "," << (db_utenti[i].sesso) << "\n";
        std::string stringa_utente = sslinea.str();
        lista_utenti << stringa_utente.c_str();
    }


    //apre il file in write only, riscrive tutto il contenuto
    if (file.open(QIODevice::WriteOnly))
    {
        for (int i = 0; i < lista_utenti.size(); i++)
        {
            file.write(lista_utenti[i].toStdString().c_str());
        }
        file.close();
    }
}

/*
 * Funzione che legge il file 'utenti.csv' e salva il contenuto nel vector 'db_utenti'
 */
void MainWindow::carica_utenti(){

    QString file_utenti = "utenti.csv";
    QFile file(file_utenti);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    QStringList list;
    list.clear();
    QTextStream in(&file);
    int cont = 0;
    for(int i = 0; !in.atEnd(); i++)
    {
        QString fileLine = in.readLine();
        list = fileLine.split(",", QString::SkipEmptyParts);

        utente u;
        u.nome = list.at(0).toStdString();
        u.cognome = list.at(1).toStdString();
        u.email_cellulare = list.at(2).toStdString();
        u.password = list.at(3).toStdString();
        u.data_nascita = list.at(4).toStdString();
        u.sesso = list.at(5).toStdString();
        db_utenti.push_back(u);
        cont++;
    }
    numero_utenti = cont;
    file.close();
}

/*
 * Funzione che prende in input una string contenente una data di nascita(dd/mm/yyyy), la confronta con la data corrente e
 * ritorna un intero che rappresenta l'età del soggetto
 */
int MainWindow::eta(const std::string &data_nascita){

    int giorno_nascita = std::stoi(data_nascita.substr(0,2));
    int mese_nascita = std::stoi(data_nascita.substr(3,2));
    int anno_nascita = std::stoi(data_nascita.substr(6,4));

    time_t ttime = time(0);
    tm *local_time = localtime(&ttime);
    int anno_corrente = 1900 + local_time->tm_year;
    int mese_corrente = 1 + local_time->tm_mon;
    int giorno_corrente = local_time->tm_mday;


    int delta_anni = anno_corrente - anno_nascita;
    int delta_mesi = mese_corrente - mese_nascita;
    int delta_giorni = giorno_corrente - giorno_nascita;

    if(delta_mesi >= 0 && delta_giorni >= 0)
        return delta_anni;
    else
        return (delta_anni-1);
}


/*
 * Funzione che prende in input 4 stringhe(nome,cognome,emailcell,password) e controlla usando le espressioni regolari che tutti i
 * parametri siano nel formato corretto, ritorna true se tutti i parametri matchano con le regexp, false atrimenti.
 * Per il nome e cognome sono consentite lettere maiuscole,minuscole e spazi, il numero di telefono deve essere composto solo da
 * 8,9 o 10 cifre, per la password sono ammessi lettere maiuscole, minuscole e numeri, l'indirizzo mail deve essere il un formato
 * corretto(<xxxxxx>@<xxxxx>.<xxx>) può contenere lettere, numeri, simbolo meno, underscore, punti, e la chiocciola.
 */
bool MainWindow::check_registrazione(const std::string &n, const std::string &c, const std::string &ec, const std::string &p){

    std::string regex_email = "(\\w+)(\\.|_|-)?(\\w*)@(\\w+)(\\.(\\w+))+";
    std::string regex_cell = "[0-9]{8,10}";
    std::string regex_nome_cognome = "[a-zA-Z ]+";
    std::string regex_password = "^[A-Za-z0-9]+$";

    std::regex rx_email(regex_email);
    std::regex rx_cell(regex_cell);
    std::regex rx_nome_cognome(regex_nome_cognome);
    std::regex rx_password(regex_password);

    if(!((std::regex_match(ec.c_str(), rx_email)) || (std::regex_match(ec.c_str(), rx_cell))))
        return false;
    if(!(std::regex_match(n.c_str(), rx_nome_cognome)))
        return false;
    if(!(std::regex_match(c.c_str(), rx_nome_cognome)))
        return false;
    if(!(std::regex_match(p.c_str(), rx_password)))
        return false;
    return true;
}


/*
 * Funzione che prende in input una stringa contenente un indirizzo mail o un numero di cellulare, controlla nel
 *  vector 'db_utenti' se e' gia presente un altro utente che si è registrato precedentemente con questa mail/cell,
 * ritorna true se lo trova, false altrimenti.
 */
bool MainWindow::utente_esistente(const std::string &mail_cell){
    if(db_utenti.size() == 0)
        return false;
    for(int i = 0; i < numero_utenti; ++i){
        if(this->db_utenti[i].email_cellulare == mail_cell)
            return true;
    }
    return false;
}


/*
 * Funzione che prende in input 2 stringhe: una mail/cell e una password, ritorna il nome associato all'account che ha come mail/cell
 * e password quelle passate come parametri. Altrimenti ritorna una stgringa vuota
 */
std::string MainWindow::check_login(const std::string &mail_cell, const std::string &psw){
    for(int i = 0; i < numero_utenti; ++i){
        if((this->db_utenti[i].email_cellulare == mail_cell) && (this->db_utenti[i].password == psw))
            return db_utenti[i].nome;
    }
    return "";
}


MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * Funzione che viene richiamata quando viene cliccato il bottone 'accedi', controlla che le credenziali inserite nelle lineedit
 * superiori siano quelle dell'admin, in caso affermativo mostra la dialog dell'admin contenente una tabella con tutti gli utenti
 * registrati e 2 grafici a torta(uno con le percentuali di uomini/donne e il secondo con le percentuali di utenti divise per
 * fasce d'età).
 * Se le credenziali inserite corrispondono ad un utente registrato viene mostrata una dialog contenente la pagina personale
 * dell'utente, altrimenti viene mostrata una messagebox d'errore
 */
void MainWindow::on_pushButton_accedi_clicked()
{
    //pannello admin
    if((ui->lineEdit_accedi_email->text().toStdString() == "admin@pas.com") && (ui->lineEdit_accedi_password->text().toStdString() == "admin")){

        float numero_uomini = 0;
        float numero_donne = 0;

        float num_18_26 = 0;
        float num_27_35 = 0;
        float num_36_44 = 0;
        float num_45_53 = 0;
        float num_54_piu = 0;

        //creazione pagina admin
        QDialog admin_dialog;
        admin_dialog.setWindowTitle("Pannello Admin");
        admin_dialog.setMinimumSize(1000,500);
        QVBoxLayout *vlayout = new QVBoxLayout;


        //creazione tabella
        QTableWidget *tabella_utenti = new QTableWidget;
        QStringList nomi_colonne;
        nomi_colonne << "Nome" << "Cognome" << "Email/cell" << "Data nascita" << "Sesso";
        tabella_utenti->setColumnCount(5);
        tabella_utenti->setHorizontalHeaderLabels(nomi_colonne);


        if(db_utenti.size() > 0){
        for(int i = 0; i < numero_utenti; i++){
            //rilevo il genere di ogni utente per il primo diagramma a torta
            if(db_utenti[i].sesso == "m")
                numero_uomini++;
            else
                numero_donne++;

            //inserimento righe e popolamento celle
            tabella_utenti->insertRow(tabella_utenti->rowCount());
            tabella_utenti->setItem(tabella_utenti->rowCount() - 1, 0, new QTableWidgetItem(db_utenti[i].nome.c_str()));
            tabella_utenti->setItem(tabella_utenti->rowCount() - 1, 1, new QTableWidgetItem(db_utenti[i].cognome.c_str()));
            tabella_utenti->setItem(tabella_utenti->rowCount() - 1, 2, new QTableWidgetItem(db_utenti[i].email_cellulare.c_str()));
            tabella_utenti->setItem(tabella_utenti->rowCount() - 1, 3, new QTableWidgetItem(db_utenti[i].data_nascita.c_str()));
            tabella_utenti->setItem(tabella_utenti->rowCount() - 1, 4, new QTableWidgetItem(db_utenti[i].sesso.c_str()));

            //rilevo l'età di ogni utente per il secondo diagramma a torta
            if((eta(db_utenti[i].data_nascita)) >= 18 && (eta(db_utenti[i].data_nascita)) <= 26){
                num_18_26++;
            }
            else if((eta(db_utenti[i].data_nascita)) >= 27 && (eta(db_utenti[i].data_nascita)) <= 35){
                num_27_35++;
            }
            else if((eta(db_utenti[i].data_nascita)) >= 36 && (eta(db_utenti[i].data_nascita)) <= 44){
                num_36_44++;
            }
            else if((eta(db_utenti[i].data_nascita)) >= 45 && (eta(db_utenti[i].data_nascita)) <= 53){
                num_45_53++;
            }
            else{
                num_54_piu++;
            }
        }}

        QLabel *titolo_label = new QLabel("<b><u>Elenco utenti registrati:</u></b>");
        tabella_utenti->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        tabella_utenti->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
        tabella_utenti->setMaximumHeight(200);
        tabella_utenti->resizeColumnsToContents();
        tabella_utenti->setWindowModality(Qt::ApplicationModal);
        tabella_utenti->setEditTriggers(QAbstractItemView::NoEditTriggers);

        //aggiunge il titolo e la tabella al vertical layout
        vlayout->addWidget(titolo_label);
        vlayout->addWidget(tabella_utenti);
        vlayout->setAlignment(titolo_label, Qt::AlignCenter);
        vlayout->setAlignment(tabella_utenti, Qt::AlignCenter);


//Grafico a torta uomini,donne
        float perc_uomini = (numero_uomini/(numero_donne+numero_uomini))*100;
        float perc_donne = (numero_donne/(numero_donne+numero_uomini))*100;
        std::string label_uomini = "Uomini(";
        label_uomini.append((std::to_string(perc_uomini)).substr(0,5));
        label_uomini.append("%)");

        std::string label_donne = "Donne(";
        label_donne.append((std::to_string(perc_donne)).substr(0,5));
        label_donne.append("%)");

        QtCharts::QPieSeries *serie_dati1 = new QtCharts::QPieSeries();
        serie_dati1->append(label_uomini.c_str(), numero_uomini);
        serie_dati1->append(label_donne.c_str(), numero_donne);


        //Uomini
            QtCharts::QPieSlice *slice_uomini = serie_dati1->slices().at(0);
            slice_uomini->setBrush(Qt::blue);

        //Donne
            QtCharts::QPieSlice *slice_donne = serie_dati1->slices().at(1);
            slice_donne->setBrush(QColor(255,0,255));

        QtCharts::QChart *piechart_uomini_donne = new QtCharts::QChart();
        piechart_uomini_donne->setTitle("<b>Sesso utenti registrati:</b>");
        piechart_uomini_donne->addSeries(serie_dati1);
        piechart_uomini_donne->legend()->setShowToolTips(true);

        QtCharts::QChartView *chartView_uomini_donne = new QtCharts::QChartView(piechart_uomini_donne);
        chartView_uomini_donne->setRenderHint(QPainter::Antialiasing);
//Grafico a torta uomini,donne


//Grafico a torta eta'
        float perc_18_26 = (num_18_26/(numero_donne+numero_uomini))*100;
        float perc_27_35 = (num_27_35/(numero_donne+numero_uomini))*100;
        float perc_36_44 = (num_36_44/(numero_donne+numero_uomini))*100;
        float perc_45_53 = (num_45_53/(numero_donne+numero_uomini))*100;
        float perc_54_piu = (num_54_piu/(numero_donne+numero_uomini))*100;

        std::string label_18_26 = "Da 18 a 26 (";
        label_18_26.append((std::to_string(perc_18_26)).substr(0,5));
        label_18_26.append("%)");

        std::string label_27_35 = "Da 27 a 35 (";
        label_27_35.append((std::to_string(perc_27_35)).substr(0,5));
        label_27_35.append("%)");

        std::string label_36_44 = "Da 36 a 44 (";
        label_36_44.append((std::to_string(perc_36_44)).substr(0,5));
        label_36_44.append("%)");

        std::string label_45_53 = "Da 45 a 53 (";
        label_45_53.append((std::to_string(perc_45_53)).substr(0,5));
        label_45_53.append("%)");

        std::string label_54_piu = "Da 54 in su (";
        label_54_piu.append((std::to_string(perc_54_piu)).substr(0,5));
        label_54_piu.append("%)");


        QtCharts::QPieSeries *serie_dati2 = new QtCharts::QPieSeries();
        serie_dati2->append(label_18_26.c_str(), num_18_26);
        serie_dati2->append(label_27_35.c_str(), num_27_35);
        serie_dati2->append(label_36_44.c_str(), num_36_44);
        serie_dati2->append(label_45_53.c_str(), num_45_53);
        serie_dati2->append(label_54_piu.c_str(), num_54_piu);

        //da 18 a 26
            QtCharts::QPieSlice *slice_18_26 = serie_dati2->slices().at(0);
            slice_18_26->setBrush(Qt::blue);

        //da 27 a 35
            QtCharts::QPieSlice *slice_27_35 = serie_dati2->slices().at(1);
            slice_27_35->setBrush(Qt::red);

        //da 36 a 44
            QtCharts::QPieSlice *slice_36_44 = serie_dati2->slices().at(2);
            slice_36_44->setBrush(Qt::green);

        //da 45 a 53
            QtCharts::QPieSlice *slice_45_53 = serie_dati2->slices().at(3);
            slice_45_53->setBrush(Qt::yellow);

        //da 54 piu
            QtCharts::QPieSlice *slice_54_piu = serie_dati2->slices().at(4);
            slice_54_piu->setBrush(Qt::cyan);

        QtCharts::QChart *piechart_eta = new QtCharts::QChart();
        piechart_eta->setTitle("<b>Età utenti registrati:</b>");
        piechart_eta->addSeries(serie_dati2);

        //abilito i suggerimenti sulle varie voci della legenda nel caso in cui il testo venga troncato
        piechart_eta->legend()->setShowToolTips(true);
        piechart_eta->legend()->setAlignment(Qt::AlignLeading);

        QtCharts::QChartView *chartView_eta = new QtCharts::QChartView(piechart_eta);
        chartView_eta->setRenderHint(QPainter::Antialiasing);
//Grafico a torta eta'

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(chartView_uomini_donne);
        hlayout->addWidget(chartView_eta);

        vlayout->addItem(hlayout);
        admin_dialog.setLayout(vlayout);
        admin_dialog.exec();
    }
 //controlla che check login ritorni un nome, se ritorna "" vuol dire che nessun utente e psw inseriti sono registrati
    else if( (db_utenti.size() > 0) && (check_login(ui->lineEdit_accedi_email->text().toStdString(), ui->lineEdit_accedi_password->text().toStdString())) != ""){
        QDialog login_successo_dialog;
        login_successo_dialog.setWindowTitle("Pagina personale");
        login_successo_dialog.setMinimumSize(500,100);
        QVBoxLayout *vlayout = new QVBoxLayout;
        std::string bentornato = "Ciao ";
        std::string nome = (check_login(ui->lineEdit_accedi_email->text().toStdString(), ui->lineEdit_accedi_password->text().toStdString()));
        bentornato.append(nome).append(", ecco la tua pagina personale.");
        QLabel *label = new QLabel(bentornato.c_str());
        vlayout->addWidget(label);
        vlayout->setAlignment(Qt::AlignCenter);
        login_successo_dialog.setLayout(vlayout);
        login_successo_dialog.exec();

    }
    //altrimenti mostra un messagebox con errore
    else{
        QMessageBox msgBox_login_fallito;
        msgBox_login_fallito.setWindowTitle("Login Fallito");
        msgBox_login_fallito.setText("I dati di login sono errati!");
        msgBox_login_fallito.exec();

    }
}

/*
 * Funzione che viene richiamata quando viene premuto il bottone 'iscriviti', vengono controllati tutti i dati inseriti richiamando le
 * funzioni 'check_registrazione' e 'utente_esistente', se tutto è andato a buon fine viene salvato il nuovo utente tramite la funzione
 * 'salva_nuovo_utente' e mostrato un messaggio di confermata registrazione. Altrimenti viene mostrato un messaggio di errore
 */
void MainWindow::on_pushButton_iscriviti_clicked()
{
    //check_registrazione(const std::string n, const std::string c, const std::string ec, const std::string p)
    bool dati_validi = check_registrazione(ui->lineEdit_nome->text().toStdString(), ui->lineEdit_cognome->text().toStdString(), ui->lineEdit_cellulare_o_mail->text().toStdString(), ui->lineEdit_nuova_password->text().toStdString());
    bool utente_gia_esistente = ((ui->lineEdit_cellulare_o_mail->text().toStdString() == "admin@pas.com") || (utente_esistente(ui->lineEdit_cellulare_o_mail->text().toStdString())));
    bool genere_selezionato = (ui->radioButton_uomo->isChecked() || ui->radioButton_donna->isChecked());

    if((eta(ui->dateEdit->text().toStdString().c_str()) >= 18) && dati_validi && (!utente_gia_esistente) && genere_selezionato){
        utente nuovo_utente;
        nuovo_utente.nome = ui->lineEdit_nome->text().toStdString();
        nuovo_utente.cognome = ui->lineEdit_cognome->text().toStdString();
        nuovo_utente.email_cellulare = ui->lineEdit_cellulare_o_mail->text().toStdString();
        nuovo_utente.password = ui->lineEdit_nuova_password->text().toStdString();
        nuovo_utente.data_nascita = ui->dateEdit->text().toStdString();
        if(ui->radioButton_uomo->isChecked())
            nuovo_utente.sesso = "m";
        if(ui->radioButton_donna->isChecked())
            nuovo_utente.sesso = "f";

        this->db_utenti.push_back(nuovo_utente);
        numero_utenti++;
        salva_nuovo_utente();

        QDialog registrazione_successo_dialog;
        registrazione_successo_dialog.setWindowTitle("Conferma Registrazione");
        registrazione_successo_dialog.setMinimumSize(550,100);
        QVBoxLayout *vlayout = new QVBoxLayout;

        //creo il messaggio di benvenuto
        std::string benvenuto;
        if(nuovo_utente.sesso == "f")
            benvenuto = "Benvenuta ";
        else
            benvenuto = "Benvenuto ";
        std::string nome = ui->lineEdit_nome->text().toStdString();
        benvenuto.append(nome).append(", ti confermiamo che la registrazione è avvenuta con successo.");
        QLabel *label = new QLabel(benvenuto.c_str());
        vlayout->addWidget(label);
        vlayout->setAlignment(Qt::AlignCenter);
        registrazione_successo_dialog.setLayout(vlayout);
        registrazione_successo_dialog.exec();
    }
    else{
        QMessageBox msgBox_reg_no;
        msgBox_reg_no.setWindowTitle(" ");
        msgBox_reg_no.setText("Registrazione Fallita! Controlla la correttezza dei dati inseriti e non usare un email/cell già registrato.");
        msgBox_reg_no.exec();
    }
}

/*
 * Funzione che viene richiamanta quando viene cliccato il bottone di recupero password, se nella lineedit di accesso è stata inserita
 * una mail/cell esistenti allora viene mostrato un messaggio di confermato ripristino della password, altrimenti viene mostrato un
 * messaggio di errore
 */
void MainWindow::on_pushButton_recupera_password_clicked()
{
    if((utente_esistente(ui->lineEdit_accedi_email->text().toStdString())) && ((ui->lineEdit_accedi_email->text().toStdString()) != "admin@pas.com")){
        QDialog recupero_password_dialog;
        recupero_password_dialog.setWindowTitle("Recupero Password Completato");
        recupero_password_dialog.setMinimumSize(550,100);
        QVBoxLayout *vlayout = new QVBoxLayout;
        QLabel *label = new QLabel("Email con procedura di ripristino inviata. Controlla la tua casella di posta elettronica.");
        vlayout->addWidget(label);
        vlayout->setAlignment(Qt::AlignCenter);
        recupero_password_dialog.setLayout(vlayout);
        recupero_password_dialog.exec();
    }
    else{
        QDialog recupero_password_dialog;
        recupero_password_dialog.setWindowTitle("Errore Recupero Password");
        recupero_password_dialog.setMinimumSize(550,100);
        QVBoxLayout *vlayout = new QVBoxLayout;
        QLabel *label = new QLabel("Email o cellulare non registrato. Procedura di ripristino password non completata.");
        vlayout->addWidget(label);
        vlayout->setAlignment(Qt::AlignCenter);
        recupero_password_dialog.setLayout(vlayout);
        recupero_password_dialog.exec();
    }
}




