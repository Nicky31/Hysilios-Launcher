#include "FenPrincipale.h"
#include "ui_UIFenPrincipale.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

  // Fonds
    QPalette fond;
    fond.setBrush(backgroundRole(),QBrush(QImage("launcherDesign/body.png")));
    setPalette(fond); // Image de fond

    MyAppDirPath =  QCoreApplication::applicationDirPath();
    currentIndexMajs = 0;
    version = -1;
    update = true;
    loadLauncherXml(); // On charge le fichier xml du launcher
    downloadFile(QUrl(launcherUrlDir+"versions.xml"),QString("versions.xml")); // On t�l�charge le xml des versions
    QObject::connect(this,SIGNAL(versionsDownloaded(QByteArray&)),this,SLOT(checkVersion(QByteArray&)));
    QObject::connect(this,SIGNAL(readyToUpdate()),this,SLOT(toUpdate()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadLauncherXml()
{
 QDomDocument dom("mon_xml");
    QFile file("launcher.xml");
    if(!file.open(QIODevice::ReadOnly)) // V�rifie qu'on peut ouvrir le fichier
    {
        QMessageBox::critical(this,"Erreur","Impossible d'ouvrir le ficher XML. Veuillez r�installer le launcher.");
        file.close();
        return;
    }
    if(!dom.setContent(&file))  // V�rifie qu'on peut attribuer le fichier XML � l'objet QDomDocument
    {
        QMessageBox::critical(this,"Erreur","Impossible d'attribuer launcher.xml � l'objet QDomDocument.");
        file.close();
        return;
    }
    file.close();

    QDomElement dom_element = dom.documentElement(); // On r�cup�re tous les �l�ments du doc � l'aide de documentElement()

   // On r�cup�re le premier noeud
    QDomNode noeud = dom_element.firstChild();

    while(!noeud.isNull())// On parcoure
    {
        QDomElement element = noeud.toElement();
        if(!element.isNull())
        {
            if(element.tagName() == "version")  // tagName = nom de la balise actuelle
                    version = element.attribute("num").toInt();
            else if(element.tagName() == "site")
                    site = element.text(); // text() = Texte contenu entre les deux balises
            else if(element.tagName() == "forum")
                    forum = element.text();
            else if(element.tagName() == "launcherUrlDir")
                    launcherUrlDir = element.text();
        }
         noeud = noeud.nextSibling(); // On va � l'�l�ment suivant
    }

    if(version == -1 || site.isNull() || forum.isNull() || launcherUrlDir.isNull()) {
        QMessageBox::critical(this,"Erreur","Le fichier launcher.xml est erron�, veuillez r�installer ce Launcher.");
        update = false;
        ui->play->setEnabled(true);
    }
    else {
        ui->labelForum->setText("<a href=" + forum + "><img src=\"launcherDesign/forum.png\" /></a>");
        ui->labelSite->setText("<a href=" + site + "><img src=\"launcherDesign/site.png\" /></a>");
    }
}

void MainWindow::checkVersion(QByteArray &versions)
{
    if(update)
    {
             QDomDocument domVersion("versions");

             if(contentXmlVersions.isEmpty()) {
                 QMessageBox::critical(this,"Erreur","Impossible de mettre � jour le client : <em>versions.xml vide</em>");
                 return;
             }

             if(!domVersion.setContent(contentXmlVersions))  // V�rifie qu'on peut attribuer le fichier XML � l'objet QDomDocument
             {
                 QMessageBox::critical(this,"Erreur","Impossible de mettre � jour le client : <em>Erreur lors de l'attribution de versions.xml � l'objet QDomDocument</em>");
                 return;
             }

        QDomElement dom_element = domVersion.documentElement(); // On r�cup�re tous les �l�ments du doc � l'aide de documentElement()
        QDomNode noeud = dom_element.firstChild();

        while(!noeud.isNull())// Tant que le n�ud n'est pas vide. (Parcourt des versions)
        {
            QDomElement element = noeud.toElement();

            if(!element.isNull())
            {
                newVersion = element.tagName().remove("v").toInt(); // On r�cup�re juste le n� de la version
                if(version < newVersion)
                  {
                     QDomNode noeudVersion = element.firstChild(); // On entre dans la version
                         while(!noeudVersion.isNull()) // On parcoure les fichiers de la version courante
                           {
                             QDomElement elementFile = noeudVersion.toElement();
                             if(!element.isNull())
                             {
                                 majs.push_back(elementFile.attribute("path")); // On ajoute le fichier � la liste des M�J
                             }
                              noeudVersion = noeudVersion.nextSibling(); // ON passe au fichier suivant
                           }
                  }
            }
             noeud = noeud.nextSibling(); // Ce code permet d'aller � l'�l�ment suivant.
        }
        if(!majs.empty())
            emit readyToUpdate(); // Si le launcher n'est pas � jour, on installe les fichiers
        else
            ui->play->setEnabled(true); // Tout est OK, on peut jouer
    }
}

void MainWindow::toUpdate()
{
  if(!majs.empty()) // S'il y a bien des M�Js � effectuer
  {
             if(currentIndexMajs < majs.size())
             {
                 int numCurrent(currentIndexMajs + 1);
                 ui->labelEtapes->setText("M�J: Etape " + QString::number(numCurrent) + " / " + QString::number(majs.size()));
                 QStringList explode = majs[currentIndexMajs].split("/"); // Pour s�lectionner le nom du fichier
                 downloadFile(launcherUrlDir + "files/" + explode[explode.size() -1], majs[currentIndexMajs]);

                 ++currentIndexMajs; // On se positionne sur le prochain fichier
             } else {
              ui->play->setEnabled(true);
              editLauncherXml();
              currentIndexMajs = 0; // On remet � 0
             }
   }
}

void MainWindow::downloadFile(QUrl downloadUrl,QString path) // fonction pour t�l�charger un fichier
{
    if(path == "")
        path = downloadUrl.toString();

    const QNetworkRequest request(downloadUrl); // Construction de la requ�te
    QNetworkAccessManager *m = new QNetworkAccessManager;
    QNetworkReply *r = m->get(request); // Envoi de la requ�te

    QStringList explode = downloadUrl.toString().split('/'); // Pour s�lectionner le nom du fichier
    fileName = explode[explode.size() -1];
    filePath = path;

    connect(r, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(messageErreur(QNetworkReply::NetworkError)));
    connect(r, SIGNAL(finished()), this, SLOT(saveFile()));
    connect(r, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(progressionTelechargement(qint64, qint64) ));
}

void MainWindow::saveFile() // Fonction pour enregistrer un fichier
{
    if(QFile::exists(filePath)) // On supprime s'il existe d�j�
        QFile::remove(filePath);

    QNetworkReply *r = qobject_cast<QNetworkReply*>(sender()); // R�cup�ration le pointeur du retour de notre requ�te

    if(fileName == "versions.xml") {
        contentXmlVersions = r->readAll();
        emit versionsDownloaded(contentXmlVersions); // On peut lancer checkVersion()
        return;
    }

    QFile downloadedFile(fileName);
    downloadedFile.open(QIODevice::WriteOnly); // Ouverture du fichier en �criture seule
    downloadedFile.write(r->readAll()); // Ecriture du contenu de la requ�te (donc notre fichier XML) dans le fichier local
    downloadedFile.close(); // Fermeture du fichier
    r->deleteLater(); // Suppression du pointeur

    // On d�place le fichier
        QFile::rename(fileName,"dofus/" + filePath); // On d�place
        emit readyToUpdate(); // On passe au dl suivant
}

void MainWindow::editLauncherXml()
{
    QDomDocument doc("launcher.xml");
     QFile file("launcher.xml");
     if (!file.open(QIODevice::ReadOnly))
         return;
     if (!doc.setContent(&file)) {
         file.close();
         return;
     }
     file.close();

     QDomElement docElem = doc.documentElement();

     QDomNode n = docElem.firstChild(); // on r�cup�re le premier noeud
     while(!n.isNull()) { // on le parcourt
         QDomElement element = n.toElement();
         if(!element.isNull()) {
             if(element.tagName() == "version")
             {
                 element.setAttribute("num", newVersion);
                 break; // On quitte si il n'y a qu'une seule balise � modifier
             }
         }
         n = n.nextSibling(); // Element suivant
     }

     QFile newXml("launcher.xml");
     if(!newXml.open(QIODevice::WriteOnly))
         return;
     QTextStream out(&newXml);
     out << doc.toString();
     newXml.close();
}

void MainWindow::on_startMAJ_clicked()
{
    int reponse = QMessageBox::question(this, "Forcer la Mise � Jour","Voulez-vous forcer la mise � jour ? Cela r�installera la totalit� des fichiers du serveur jusqu'� la derni�re version.", QMessageBox::Yes | QMessageBox::No);

        if (reponse == QMessageBox::Yes) // S'il a accept� de forcer la M�J
        {
            version = 0;
            emit versionsDownloaded(contentXmlVersions); // On lance CheckVersion
        }
}

void MainWindow::progressionTelechargement(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal != -1)
    {
        ui ->progression->setRange(0, bytesTotal);
        ui ->progression->setValue(bytesReceived);
    }
}

void MainWindow::messageErreur(QNetworkReply::NetworkError)
{
    erreurTrouvee = true; //On indique qu'il y a eu une erreur au slot enregistrer
    QNetworkReply *r = qobject_cast<QNetworkReply*>(sender());
    QMessageBox::critical(this, "Erreur", "Erreur lors du chargement. V�rifiez votre connexion internet ou r�essayez plus tard <br /><br /> Code de l'erreur : <br /><em>" + r->errorString() + "</em>");
}

void MainWindow::on_play_clicked()
{
    if(!QProcess::startDetached("../Dofus.exe") && !QProcess::startDetached("\"C:\\Program Files\\Dofus\\Dofus.exe\""))
        QMessageBox::critical(this,"Erreur","Dofus.exe introuvable !");
}


