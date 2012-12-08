#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#include <QMainWindow>
#include <QtNetwork>
#include <QtXml>
#include <QtGui>
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void loadLauncherXml();
    void downloadFile(QUrl downloadUrl,QString path="");
    void editLauncherXml();

private:
    Ui::MainWindow *ui;
    bool erreurTrouvee; //Variable qui nous permet de savoir s'il y a eu une erreur ou non.
    bool update; // M�J possible ?(=> si xml launcher valide)
    int version;   // N� de la version
    QString site;  // Url du site
    QString forum; // Url du forum
    QString launcherUrlDir; // Url vers le dossier du launcher (versions.xml, fichiers de M�J...)
    std::vector<QString> majs; // Tableau contenant l'url de tous les fichiers � dl pour la mise � jour
    QString fileName; // Nom du fichier courant � sauvegarder, d�fini dans downloadFile()
    QString filePath; // Chemin du fichier courant � sauvegarder, d�fini dans downloadFile()
    QString MyAppDirPath; // Chemin jusqu'au r�pertoire de l'up, & donc de DOFUS
    int newVersion; // Nouvelle version du launcher (apr�s update)
    int currentIndexMajs; // N� du file actuellement en dl
    QByteArray contentXmlVersions; // contenu de versions.xml

private slots:
    void on_startMAJ_clicked();
    void on_play_clicked();
    void progressionTelechargement(qint64 bytesReceived, qint64 bytesTotal);
    void messageErreur(QNetworkReply::NetworkError);
    void checkVersion(QByteArray &versions);
    void saveFile();
    void toUpdate();

signals:
    void versionsDownloaded(QByteArray &versions);
    void readyToUpdate();

};

#endif // FENPRINCIPALE_H
