#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QTabWidget>
#include <QWebEngineFullScreenRequest>
#include <QPushButton>
#include <QLineEdit>
#include <QIcon>
#include <QWebEngineView>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include<QApplication>
#include <QMessageBox>
#include <QWebEnginePage>
#include <QWebEngineNavigationRequest>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineDownloadRequest>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QSysInfo>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QDir>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(0);

    ui->verticalLayout->setSpacing(0);
    ui->verticalLayout->setContentsMargins(0,0,0,0);
    setContentsMargins(0,0,0,0);
    ui->centralwidget->setContentsMargins(0,0,0,0);
    //setting the window icon
    const QIcon *window_icon = new QIcon(":/img/img/darb.png");
    setWindowIcon(*window_icon);
    setWindowTitle(u8"متصفح درب");
    // add the startup tab
    MainWindow::addNewTab();

    //new tab button
    QPushButton *addTabButton = new QPushButton("+", this);
    QFont font = addTabButton->font();
    font.setPixelSize(25);
    addTabButton->setFont(font);
    addTabButton->setFixedSize(35, 35);
    addTabButton->setStyleSheet("color:white;"
                                "background:rgb(15,15,15);"
                                "margin:0px;");
    addTabButton->setToolTip("إضافة تبويبة جديدة");
    QWidget *cornerWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(cornerWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addWidget(addTabButton);
    layout->setAlignment(addTabButton, Qt::AlignRight);

    ui->tabWidget->setCornerWidget(cornerWidget, Qt::TopRightCorner);
    connect(addTabButton, &QPushButton::clicked, this, &MainWindow::addNewTab);
}
void MainWindow::addNewTab() {
    QWidget *newTab = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout(newTab);
    layout->setAlignment(Qt::AlignTop);

    // search text box element
    QLineEdit *search_input = new QLineEdit();
    search_input->setStyleSheet("border-radius:10px;"
                         "background:#303030;"
                         "font-size:18px;"
                         "padding:5px;"
                         "color:white;");
    // load buttons icons
    const QIcon* search_icon = new QIcon(":/img/img/search.png");
    const QIcon* refresh_icon = new QIcon(":/img/img/refresh.png");
    const QIcon* settings_icon = new QIcon(":img/img/setting.png");
    const QIcon* home_icon = new QIcon(":img/img/home.png");
    //buttons
    QPushButton* search_btn = new QPushButton();
    search_btn->setIcon(*search_icon);

    QPushButton* refresh_btn = new QPushButton();
    refresh_btn->setIcon(*refresh_icon);

    QPushButton* settings_btn = new QPushButton();
    settings_btn->setIcon(*settings_icon);

    QPushButton* home_btn = new QPushButton();
    home_btn->setIcon(*home_icon);

    // the web view
    QWebEngineView* web_view = new QWebEngineView();
    web_view->page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    //profile->setPersistentStoragePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/myBrowserData");
    profile->setCachePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    web_view->setPage(new QWebEnginePage(profile, web_view));

    web_view->setUrl(QUrl("https://duckduckgo.com/?q=&t=chromentp"));
    web_view->reload();

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addWidget(refresh_btn);
    hbox->addWidget(search_input);
    hbox->addWidget(search_btn);
    hbox->addWidget(home_btn);
    hbox->addWidget(settings_btn);

    layout->setContentsMargins(0,9,0,9);

    layout->addLayout(hbox);
    layout->addWidget(web_view);

    web_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //config the web view setting
    web_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,true);
    web_view->settings()->setAttribute(QWebEngineSettings::WebGLEnabled,true);
    web_view->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled,true);
    //config cookies
    QWebEngineCookieStore *cookieStore = web_view->page()->profile()->cookieStore();
    connect(cookieStore, &QWebEngineCookieStore::cookieAdded,this, [](const QNetworkCookie &cookie) {
        qDebug() << "Cookie added:" << cookie.name();
        qDebug() << cookie.path();
        qDebug() << cookie.domain();
    });
    cookieStore->loadAllCookies();
    qDebug() << web_view->page()->profile()->persistentStoragePath();

    // add new Tab
    newTab->setLayout(layout);
    ui->tabWidget->addTab(newTab, QString("رئيسية"));

    int last_tab_index = ui->tabWidget->count();
    ui->tabWidget->setCurrentIndex(last_tab_index - 1);

    connect(search_input,&QLineEdit::returnPressed,this,&MainWindow::onSearchButtonClicked);

    // Connect button signals to slots
    connect(search_btn, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);
    connect(refresh_btn, &QPushButton::clicked, this, &MainWindow::onRefreshButtonClicked);
    connect(home_btn, &QPushButton::clicked, this, &MainWindow::onHomeButtonClicked);
    connect(settings_btn, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);

    connect(web_view->page(),&QWebEnginePage::newWindowRequested,this,&MainWindow::on_newWindow);
    connect(web_view->page(),&QWebEnginePage::fullScreenRequested,this,&MainWindow::on_web_full_screen);

    connect(web_view->page()->profile(),&QWebEngineProfile::downloadRequested,this,&MainWindow::on_download);
    connect(web_view,&QWebEngineView::titleChanged,this,&MainWindow::onTitleChanged);

    connect(web_view,&QWebEngineView::urlChanged,this,&MainWindow::onUrlChanged);
    connect(web_view,&QWebEngineView::iconChanged,this,&MainWindow::onIconChanged);
}
void MainWindow::on_download(const QWebEngineDownloadRequest *download){
    QString file_name = download->downloadDirectory();
    if( QSysInfo::prettyProductName().startsWith("Windows")){
        // dos file System :-)
        file_name += "\\";
    }else{
        // unix file system B-)
        file_name += "/";
    }
    file_name += download->downloadFileName();
    qDebug() << download->url() << "\n";
    // remove last tab of download

    // download the file
    this->downloadFile(download->url(),file_name);
}
void MainWindow::downloadFile(const QUrl &url, const QString &filePath)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                QMessageBox* msgbox = new QMessageBox();
                msgbox->setText("تم تحميل الملف بنجاح !");
                msgbox->show();
            } else {
                QMessageBox* msgbox = new QMessageBox();
                msgbox->setText("فشل فتح الملف ياصاح هناك خطأ ما ._.");
                msgbox->show();
            }
        } else {
            QMessageBox* msgbox = new QMessageBox();
            msgbox->setText("فشل التحميل");
            msgbox->show();
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}
void MainWindow::on_web_full_screen(const QWebEngineFullScreenRequest &request){
    ui->tabWidget->tabBarAutoHide();
    this->SetFullScreen();
}
void MainWindow::SetFullScreen()
{
    this->showFullScreen();
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_newWindow(const QWebEngineNewWindowRequest &request){
    QUrl url = request.requestedUrl();
    MainWindow::addNewTab();
    QWidget *currentTab = ui->tabWidget->currentWidget();
    QWebEngineView *webView = currentTab->findChild<QWebEngineView *>();
    webView->load(url);
}
void MainWindow::onIconChanged(const QIcon &icon){
    int currentIndex = ui->tabWidget->currentIndex();
    ui->tabWidget->setTabIcon(currentIndex,icon);
}
void MainWindow::onUrlChanged(const QUrl &url){
    QWidget *currentTab = ui->tabWidget->currentWidget();
    QLineEdit *searchInput = currentTab->findChild<QLineEdit *>();
    QString str_url = url.toEncoded();
    searchInput->setText(str_url);
}
void MainWindow::onTitleChanged(const QString &title){
    int currentIndex = ui->tabWidget->currentIndex();
    ui->tabWidget->setTabText(currentIndex, title);
}
void MainWindow::onSearchButtonClicked(){
    QWidget *currentTab = ui->tabWidget->currentWidget();
    QLineEdit *searchInput = currentTab->findChild<QLineEdit *>();
    QWebEngineView *webView = currentTab->findChild<QWebEngineView *>();
    QString search_query = searchInput->text();
    if (!search_query.startsWith("http")){
        search_query.replace(" ","+");
        search_query = "https://duckduckgo.com/?q=" + search_query;
    }
    webView->load(QUrl(search_query));
}
void MainWindow::onRefreshButtonClicked(){
    QWidget *currentTab = ui->tabWidget->currentWidget();
    QWebEngineView *webView = currentTab->findChild<QWebEngineView *>();
    webView->reload();
}
void MainWindow::onHomeButtonClicked(){
    QWidget *currentTab = ui->tabWidget->currentWidget();
    QWebEngineView *webView = currentTab->findChild<QWebEngineView *>();
    webView->load(QUrl("https://duckduckgo.com/"));
}

void MainWindow::onSettingsButtonClicked(){

}
void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if (ui->tabWidget->count() == 1){
        ui->tabWidget->setCurrentIndex(index);
        QWidget *currentTab = ui->tabWidget->currentWidget();
        QWebEngineView *webView = currentTab->findChild<QWebEngineView *>();
        webView->close();
        ui->tabWidget->removeTab(index);
        MainWindow::close();
    }
    ui->tabWidget->setCurrentIndex(index);
    QWidget *currentTab = ui->tabWidget->currentWidget();
    QWebEngineView *webView = currentTab->findChild<QWebEngineView *>();
    webView->close();
    ui->tabWidget->removeTab(index);
}

