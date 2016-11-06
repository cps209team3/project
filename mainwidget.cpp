#include <QLabel>
#include <QDebug>
#include <QDesktopWidget>
#include <QTimer>
#include <QKeyEvent>
#include <cmath>
#include <QTime>

#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "world.h"
#include "titlescreen.h"
#include "endgame.h"
#include "highscore.h"
#include "highscorepage.h"
#include "loadsave.h"
#include "pausescreen.h"
#include <sstream>


MainWidget::MainWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MainWidget)
{
	ui->setupUi(this);
	ui->lblLife1->raise();
	ui->lblLife2->raise();
    ui->lblLife3->raise();
	ui->lblScore->raise(); // these components should not be under the world objects
	ui->lblTimeLeft->raise();


	timer = new QTimer(this);
    timer->setInterval(50);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerHit()));


	clock = new QTimer(this);
	clock->setInterval(1000);
	connect(clock, SIGNAL(timeout()), this, SLOT(clockHit()));

	right = false;
    left = false;
	TitleScreen* titleScrn = new TitleScreen(this);
	titleScrn->show();
    titleScrn->raise();
    connectCount = 0;
    connect(&server,SIGNAL(newConnection()),this,SLOT(clientConnected()));
    socket = NULL;
}

void MainWidget::loadLevel(QString filename)
{
	ObjectLabel* lblPlayer = NULL;

	//Deletes all objects from the last game
	for (int i = 0; i < ui->worldWidget->children().size(); ++i) {
		if (dynamic_cast<ObjectLabel*>(ui->worldWidget->children().at(i)) != NULL){
			ui->worldWidget->children().at(i)->deleteLater();
		}
	}

	LoadSave::instance().load(filename);
	HighScore::instance().LoadScore(World::instance().getLevelName());
	World::instance().getScreen()->setScreenSize(ui->worldWidget->geometry().width(), ui->worldWidget->geometry().height());
	ui->lblBackground->setPixmap(QPixmap(World::instance().getBackgroundPath()));

    Player* player = World::instance().getPlayer();
	lblPlayer = new ObjectLabel(ui->worldWidget);
	lblPlayer->setObject(player);
	lblPlayer->setPixmap(QPixmap(player->getImage()));
	lblPlayer->setScaledContents(true);
	lblPlayer->show();
	lblPlayer->updateLabelPosition();

	for (Object* worldObj : World::instance().getObjects())
	{
		ObjectLabel* label = new ObjectLabel(ui->worldWidget);
		label->setObject(worldObj);
		label->updateLabelPosition();
		label->setScaledContents(true);
		label->setPixmap(QPixmap(worldObj->getImage()));
		if (label->getObject()->getVisibility())
			label->show();
	}

	if (lblPlayer != NULL)
	{
		lblPlayer->raise();
		labelPlayer = lblPlayer;
	}
	ui->lblLife1->show();
	ui->lblLife2->show();
	ui->lblLife3->show();

    ui->lblLife1->raise();
    ui->lblLife2->raise();
    ui->lblLife3->raise();
    ui->lblScore->raise();
    ui->lblTimeLeft->raise();

    ui->lblCheat->hide();

	World::instance().setSeconds(World::instance().getStartSeconds());
	World::instance().setCurrentLevel(filename);

	ui->lblTimeLeft->setText(QDateTime::fromTime_t(World::instance().getSeconds()).toUTC().toString("m:ss"));
}

void MainWidget::timerHit(){

	//program 4 code below (for reference)
	World& world = World::instance();
    Player* player = world.getPlayer();

    if ((right && left) || (!right && !left)) {
		// if both right and left arrows are held down or both are released slow the player to a stop
		player->slowToStop();
        player->setCount(0);
	} else if (right) {
		// if the right arrow is pressed the player goes right
        if (player->getCanMove())
            player->moveRight();
	} else if (left) {
		// if the left arrow is pressed the player goes left
        if (player->getCanMove())
            player->moveLeft();
	}

	// updates player's position in the model
    player->move();

	if (player->getX() < 0)
	{
		player->setX(0);
		player->setXSpeed(0);
	}
	if (player->getRightPoint() > World::instance().getScreen()->getLevelWidth())
	{
		player->setX(World::instance().getScreen()->getLevelWidth() - player->getWidth());
		player->setXSpeed(0);
	}

	ui->lblPowerJump->setVisible(player->powerJump());
	ui->lblPowerSpeed->setVisible(player->powerSpeed());
	ui->lblPowerShield->setVisible(player->powerShield());
	ui->lblPowerScore->setVisible(player->powerScore());

    if (!player->getCanMove())
    {
        QTimer::singleShot(500,this,SLOT(enableMove()));
        QString img = ":/images/maincharacter/hurt";
        if (player->powerShield())
        {
            img += "shield";
            player->setWidth(48);
        }
        if (player->isRight() == false)
            img += "left";
        img += ".png";
        player->setImage(img);
    }

	// update screen location based on player location
	PlayingScreen* screen = World::instance().getScreen();
	if (player->getX() - screen->getX() > screen->getCenterX(player->getWidth())
			&& (screen->getX() + screen->getScreenWidth()) < screen->getLevelWidth())
	{
		screen->setX(min(player->getX() - screen->getCenterX((player->getWidth())), screen->getLevelWidth() - screen->getScreenWidth()));
	}
	else if (player->getX() - screen->getX() < screen->getCenterX(player->getWidth())
			 && screen->getX() > 0)
	{
		screen->setX(max(player->getX() - screen->getCenterX((player->getWidth())), 0));
	}

	if (player->getY() - screen->getY() > screen->getCenterY(player->getHeight())
			&& (screen->getY() + screen->getScreenHeight()) < screen->getLevelHeight())
	{
		screen->setY(min(player->getY() - screen->getCenterY((player->getHeight())), screen->getLevelHeight() - screen->getScreenHeight()));
	}
	else if (player->getY() - screen->getY() < screen->getCenterY(player->getHeight())
			 && screen->getY() > 0)
	{
		screen->setY(max(player->getY() - screen->getCenterY((player->getHeight())), 0));
    }

    vector<MoveThread*> moveThreads;
    for (size_t i = 0; i < world.getObjects().size(); i ++)
    {
        QCoreApplication::processEvents();
        MoveThread* currentThread = new MoveThread(world.getObjects().at(i));
        moveThreads.push_back(currentThread);
        currentThread->start();
    }

    for (size_t i = 0; i < moveThreads.size(); ++i) {
        QCoreApplication::processEvents();
        MoveThread* currentThread = moveThreads.at(i);
        currentThread->wait();
        delete currentThread;
    }
    CheckPlayerCollisionThread* playerCollide = new CheckPlayerCollisionThread();
    playerCollide->start();

    for (int i = 0; i < ui->worldWidget->children().length(); i++)
    {
        QCoreApplication::processEvents();
        ObjectLabel * guiObject = dynamic_cast<ObjectLabel*>(ui->worldWidget->children().at(i));
        if (guiObject != NULL) {
            // updates the position of each label to the position of its object in the model
            guiObject->updateLabelPosition();
            // showCoin method replacement
            if (guiObject->getObject()->getVisibility() == true) {
                guiObject->show();
            } else {
				guiObject->hide();
            }
        }
    }
    playerCollide->wait();
    if (playerCollide->getDeath()) {
        death(player);
    }
    delete playerCollide;

    ui->lblScore->setText(QString::number(World::instance().getScore()));

    if (player->getIsAtEndOfLevel()) {
        death(player);
    }

    if (player->getBottomPoint() > World::instance().getScreen()->getLevelHeight() || World::instance().getSeconds() == 0 )
    {
        if (world.getCheat())
            return;
        death(player);
        resetPlayer(player);
    }

    labelPlayer->setPixmap(player->getImage());
    if (socket != NULL) {
        stringstream line;
        line << player->getX() << "," << player->getY() << "," << player->getImage().toStdString();
        string sline;
        getline(line,sline);
        socket->write(QString::fromStdString(sline).toLocal8Bit());
    }
}

void MainWidget::clockHit()
{
	if (!World::instance().getCheat())
		World::instance().setSeconds(World::instance().getSeconds() - 1);
	ui->lblTimeLeft->setText(QDateTime::fromTime_t(World::instance().getSeconds()).toUTC().toString("m:ss"));
    ui->lblCheat->hide();
    if (World::instance().getCheat())
    {
        ui->lblTimeLeft->setText(QString(""));
        ui->lblTimeLeft->setPixmap(QString(":/images/infinity.png"));
        ui->lblTimeLeft->setScaledContents(true);

        ui->lblCheat->show();
    }
	if (World::instance().getSeconds() == 0)
	{
		death(World::instance().getPlayer());
		resetPlayer(World::instance().getPlayer());
	}

	// decrement powerup times
	if (!World::instance().getCheat())
	{
		Player* player = World::instance().getPlayer();
		if (player->powerJump())
		{
			player->getPowerTime("jump") -= 1;
			if (player->getPowerTime("jump") <= 0)
				player->setPower("jump", false);
		}
		if (player->powerSpeed())
		{
			player->getPowerTime("speed") -= 1;
			if (player->getPowerTime("speed") <= 0)
				player->setPower("speed", false);
		}
		if (player->powerShield())
		{
			player->getPowerTime("shield") -= 1;
			if (player->getPowerTime("shield") <= 0)
				player->setPower("shield", false);
		}
		if (player->powerScore())
		{
			player->getPowerTime("score") -= 1;
			if (player->getPowerTime("score") <= 0)
				player->setPower("score", false);
		}
	}
}

void MainWidget::resetPlayer(Player* player)
{
	player->setX(player->getStartX());
	player->setY(player->getStartY());
	if (player->getNumLives() > 0)
		World::instance().setScore(0);
	ui->lblScore->setText("0");
	World::instance().getScreen()->setLocation(0, 0);
	clock->stop();
	World::instance().setSeconds(World::instance().getStartSeconds() + 1);
	clockHit();
	clock->start();

	for (Object* worldObj : World::instance().getObjects()) {

		Coin * coin = dynamic_cast<Coin*>(worldObj);
		if (coin != NULL) {
			coin->setVisibility(true);
			coin->setisCollectible(true);
		}
	}
}

void MainWidget::death(Player* player)
{
	player->setNumLives(player->getNumLives() - 1);

	if (player->getNumLives() > 0 && !player->getIsAtEndOfLevel()) {
		if (player->getNumLives() == 2){
			ui->lblLife3->hide();
		} else if (player->getNumLives() == 1){
			ui->lblLife2->hide();
		}
		//will need to split this to display different screens
	} else {
		ui->lblLife1->hide();
		timer->stop();
		clock->stop();
		EndGame * e = new EndGame(this);
		e->show();
		e->raise();
	}
}

//displays all the coins in the world if the player has lives left
void MainWidget::showCoin() {
	for (Object* worldObj : World::instance().getObjects()) {

		Coin * coin = dynamic_cast<Coin*>(worldObj);
		if (coin != NULL) {

			int coinId = worldObj->getId();
			ObjectLabel * lbl;

			for (int i = 0; i < ui->worldWidget->children().length(); i++ ) {
				QCoreApplication::processEvents();
				lbl = dynamic_cast<ObjectLabel*>(ui->worldWidget->children().at(i));

				if (lbl != NULL) {
					if (lbl->getId() == coinId){
						if (coin->getVisibility() == true) {
							lbl->show();
						} else {
							lbl->hide();
						}
					}
				}
			}
		}
	}
}

MainWidget::~MainWidget() {
	ui->worldWidget->deleteLater();
	delete ui;
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
	Player* player = World::instance().getPlayer();

    if (event->key() == Qt::Key_Left) {
		this->left = true;
        player->setRight(false);
	} else if (event->key() == Qt::Key_Right) {
		this->right = true;
        player->setRight(true);
    } else if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Up) {
        player->setJumping(true);
        player->setJumpOnMove(true);
    }
    else if (event->key() == Qt::Key_A)
    {
        if (player->canKick())
        {
            player->setKicking(true);
            player->setCanKick(false);
            QTimer::singleShot(500,this,SLOT(stopKicking()));
            QTimer::singleShot(1000,this,SLOT(enableKicking()));
        }
    }
}

void MainWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Left) {
		this->left = false;
	} else if (event->key() == Qt::Key_Right) {
		this->right = false;
	}
}

void MoveThread::run()
{
    object->move();
}


void CheckPlayerCollisionThread::run()
{
    for(size_t i = 0; i < World::instance().getObjects().size(); ++i) {
        // checks to see if player the player collides with each object
        CollisionDetails* collision = World::instance().getPlayer()->checkCollision(World::instance().getObjects().at(i));
        if (collision != NULL) {
            World::instance().getPlayer()->collide(collision);
            if (dynamic_cast<Enemy*>(collision->getCollided()))
                if (collision->getCollided()->getVisibility() && World::instance().getPlayer()->powerShield() == false && World::instance().getCheat() == false)
                    death = true;
        }
        delete collision;
    }
}

void MainWidget::on_PBpause_clicked()
{
	if (!timer->isActive())
		// animation timer stopped means already paused
		return;
    timer->stop();
    clock->stop();
	PauseScreen* pause = new PauseScreen(this);
	connect(pause, &PauseScreen::resumeClicked, this, &MainWidget::on_resumeFromPause);
	connect(pause, &PauseScreen::restartClicked, this, &MainWidget::on_restartFromPause);
    pause->show();
    pause->raise();
}

void MainWidget::on_resumeFromPause()
{
	timer->start();
	clock->start();
}

void MainWidget::on_restartFromPause()
{
	loadLevel(World::instance().getCurrentLevel());
	timer->start();
	clock->start();
}

void MainWidget::on_loadState(QString filename)
{
	loadLevel(filename);
}

void MainWidget::enableMove()
{
    World::instance().getPlayer()->setCanMove(true);
    World::instance().getPlayer()->setWidth(29);
}
void MainWidget::stopKicking()
{
    World::instance().getPlayer()->setKicking(false);
}
void MainWidget::enableKicking()
{
    World::instance().getPlayer()->setCanKick(true);
}

void MainWidget::clientConnected()
{
    if(connectCount >= 1) {
        return;
    }
    connectCount += 1;
    socket = server.nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
}

void MainWidget::dataReceived()
{
    QTcpSocket *sock = dynamic_cast<QTcpSocket*>(sender());
    while (sock->canReadLine()) {
        QString line = sock->readLine();

    }
}

void MainWidget::clientDisconnected()
{
    QTcpSocket *sock = dynamic_cast<QTcpSocket*>(sender());
    sock->deleteLater();
    --connectCount;
    socket = NULL;
}
