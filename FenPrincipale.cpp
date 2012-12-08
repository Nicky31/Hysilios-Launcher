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
    downloadFile(QUrl(launcherUrlDir+"versions.xml"),QString("versions.xml")); // On télécharge le xml des versions
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
    if(!file.open(QIODevice::ReadOnly)) // Vérifie qu'on peut ouvrir le fichier
    {
        QMessageBox::critical(this,"Erreur","Impossible d'ouvrir le ficher XML. Veuillez réinstaller le launcher.");
        file.close();
        return;
    }
    if(!dom.setContent(&file))  // Vérifie qu'on peut attribuer le fichier XML à l'objet QDomDocument
    {
        QMessageBox::critical(this,"Erreur","Impossible d'attribuer launcher.xml à l'objet QDomDocument.");
        file.close();
        return;
    }
    file.close();

    QDomElement dom_element = dom.documentElement(); // On récupère tous les éléments du doc à l'aide de documentElement()

   // On récupère le premier noeud
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
         noeud = noeud.nextSibling(); // On va à l'élément suivant
    }

    if(version == -1 || site.isNull() || forum.isNull() || launcherUrlDir.isNull()) {
        QMessageBox::critical(this,"Erreur","Le fichier launcher.xml est erroné, veuillez réinstaller ce Launcher.");
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
                 QMessageBox::critical(this,"Erreur","Impossible de mettre à jour le client : <em>versions.xml vide</em>");
                 return;
             }

             if(!domVersion.setContent(contentXmlVersions))  // Vérifie qu'on peut attribuer le fichier XML à l'objet QDomDocument
             {
                 QMessageBox::critical(this,"Erreur","Impossible de mettre à jour le client : <em>Erreur lors de l'attribution de versions.xml à l'objet QDomDocument</em>");
                 return;
             }

        QDomElement dom_element = domVersion.documentElement(); // On récupère tous les éléments du doc à l'aide de documentElement()
        QDomNode noeud = dom_element.firstChild();

        while(!noeud.isNull())// Tant que le nœud n'est pas vide. (Parcourt des versions)
        {
            QDomElement element = noeud.toElement();

            if(!element.isNull())
            {
                newVersion = element.tagName().remove("v").toInt(); // On récupère juste le n° de la version
                if(version < newVersion)
                  {
                     QDomNode noeudVersion = element.firstChild(); // On entre dans la version
                         while(!noeudVersion.isNull()) // On parcoure les fichiers de la version courante
                           {
                             QDomElement elementFile = noeudVersion.toElement();
                             if(!element.isNull())
                             {
                                 majs.push_back(elementFile.attribute("path")); // On ajoute le fichier à la liste des MàJ
                             }
                              noeudVersion = noeudVersion.nextSibling(); // ON passe au fichier suivant
                           }
                  }
            }
             noeud = noeud.nextSibling(); // Ce code permet d'aller à l'élément suivant.
        }
        if(!majs.empty())
            emit readyToUpdate(); // Si le launcher n'est pas à jour, on installe les fichiers
        else
            ui->play->setEnabled(true); // Tout est OK, on peut jouer
    }
}

void MainWindow::toUpdate()
{
  if(!majs.empty()) // S'il y a bien des MàJs à effectuer
  {
             if(currentIndexMajs < majs.size())
             {
                 int numCurrent(currentIndexMajs + 1);
                 ui->labelEtapes->setText("MàJ: Etape " + QString::number(numCurrent) + " / " + QString::number(majs.size()));
                 QStringList explode = majs[currentIndexMajs].split("/"); // Pour sélectionner le nom du fichier
                 downloadFile(launcherUrlDir + "files/" + explode[explode.size() -1], majs[currentIndexMajs]);

                 ++currentIndexMajs; // On se positionne sur le prochain fichier
             } else {
              ui->play->setEnabled(true);
              editLauncherXml();
              currentIndexMajs = 0; // On remet à 0
             }
   }
}

void MainWindow::downloadFile(QUrl downloadUrl,QString path) // fonction pour télécharger un fichier
{
    if(path == "")
        path = downloadUrl.toString();

    const QNetworkRequest request(downloadUrl); // Construction de la requête
    QNetworkAccessManager *m = new QNetworkAccessManager;
    QNetworkReply *r = m->get(request); // Envoi de la requête

    QStringList explode = downloadUrl.toString().split('/'); // Pour sélectionner le nom du fichier
    fileName = explode[explode.size() -1];
    filePath = path;

    connect(r, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(messageErreur(QNetworkReply::NetworkError)));
    connect(r, SIGNAL(finished()), this, SLOT(saveFile()));
    connect(r, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(progressionTelechargement(qint64, qint64) ));
}

void MainWindow::saveFile() // Fonction pour enregistrer un fichier
{
    if(QFile::exists(filePath)) // On supprime s'il existe déjà
        QFile::remove(filePath);

    QNetworkReply *r = qobject_cast<QNetworkReply*>(sender()); // Récupération le pointeur du retour de notre requête

    if(fileName == "versions.xml") {
        contentXmlVersions = r->readAll();
        emit versionsDownloaded(contentXmlVersions); // On peut lancer checkVersion()
        return;
    }

    QFile downloadedFile(fileName);
    downloadedFile.open(QIODevice::WriteOnly); // Ouverture du fichier en écriture seule
    downloadedFile.write(r->readAll()); // Ecriture du contenu de la requête (donc notre fichier XML) dans le fichier local
    downloadedFile.close(); // Fermeture du fichier
    r->deleteLater(); // Suppression du pointeur

    // On déplace le fichier
        QFile::rename(fileName,"dofus/" + filePath); // On déplace
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

     QDomNode n = docElem.firstChild(); // on récupère le premier noeud
     while(!n.isNull()) { // on le parcourt
         QDomElement element = n.toElement();
         if(!element.isNull()) {
             if(element.tagName() == "version")
             {
                 element.setAttribute("num", newVersion);
                 break; // On quitte si il n'y a qu'une seule balise à modifier
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
    int reponse = QMessageBox::question(this, "Forcer la Mise à Jour","Voulez-vous forcer la mise à jour ? Cela réinstallera la totalité des fichiers du serveur jusqu'à la dernière version.", QMessageBox::Yes | QMessageBox::No);

        if (reponse == QMessageBox::Yes) // S'il a accepté de forcer la MàJ
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
    QMessageBox::critical(this, "Erreur", "Erreur lors du chargement. Vérifiez votre connexion internet ou réessayez plus tard <br /><br /> Code de l'erreur : <br /><em>" + r->errorString() + "</em>");
}

void MainWindow::on_play_clicked()
{
    if(!QProcess::startDetached("../Dofus.exe") && !QProcess::startDetached("\"C:\\Program Files\\Dofus\\Dofus.exe\""))
        QMessageBox::critical(this,"Erreur","Dofus.exe introuvable !");
}


