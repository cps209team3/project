#include "enemy.h"
#include "world.h"
#include "platform.h"
#include <QApplication>
#include "QRect"

void Enemy::load(QString config)
{
	try
	{
		QList<QString> params = config.split(",");
		Object::load(config);
		this->setVisibility(Object::getQListElement(params, 6) == "true");
		this->setDamage(10);
		this->setXSpeed(2);
		this->setRight(true);
	}
	catch (exception& ex)
	{
		throw invalid_argument(ex.what());
	}
}

QString Enemy::save()
{
	QString out = Object::save();
	out += "," + QString::fromStdString(this->getVisibility() ? "true" : "false");
	return out;
}

void Enemy::move()
{
    if(!QRect(x,y,width,height).intersects(World::instance().getCurrentScreen())) {
        return;
    }
    if (getRightPoint() + xSpeed >= currentPlatform->getRightPoint() || x - xSpeed <= currentPlatform->getX())
        right = !right;

    if (right)
    {
        x += xSpeed;
    }
    else
    {
        x -= xSpeed;
    }

    y += ySpeed;

    for (size_t i = 0; i < World::instance().getObjects().size(); i ++)
    {
        CollisionDetails* col = checkCollision(World::instance().getObjects().at(i));
        if (col != NULL)
        {
            collide(col);
            delete col;
        }
    }
}

void Enemy::collide(CollisionDetails *details)
{
    if (dynamic_cast<Platform*>(details->getCollided()) != NULL || dynamic_cast<Player*>(details->getCollided()) != NULL) {
        if (details->getXStopCollide() != 0) {
            x += details->getXStopCollide();
            if (details->getXStopCollide() > 0)
            {
                right = true;
                if (dynamic_cast<Player*>(details->getCollided()) != NULL)
                    x += 5;
            }
            if (details->getXStopCollide() < 0)
            {
                right = false;
                if (dynamic_cast<Player*>(details->getCollided()) != NULL)
                    x -= 5;
            }
        }
        if (details->getYStopCollide() != 0) {
            y += details->getYStopCollide();
            currentPlatform = dynamic_cast<Platform*>(details->getCollided());
        }
    }
}

// ======FlyingEnemy=====

void FlyingEnemy::load(QString config)
{
	try
	{
		QList<QString> params = config.split(",");
		Enemy::load(config);
		this->setStartX(Object::getQListElement(params, 7).toInt());
		this->setStartY(Object::getQListElement(params, 8).toInt());
	}
	catch (exception& ex)
	{
		throw invalid_argument(ex.what());
	}
}

QString FlyingEnemy::save()
{
	QString out = Enemy::save();
	out += "," + QString::number(this->getStartX());
	out += "," + QString::number(this->getStartY());
	return out;
}

void FlyingEnemy::move()
{
    if(!QRect(x,y,width,height).intersects(World::instance().getCurrentScreen())) {
        return;
    }
    if (isRight())
    {
        x += xSpeed;
    }
    else
    {
        x -= xSpeed;
    }

    if (up)
        y -= ySpeed;
    else
        y += ySpeed;

    count ++;
    xCount ++;
    if (count == 10)
    {
        up = !up;
        count = 0;
    }
    if (xCount == 40)
    {
        right = !right;
        xCount = 0;
    }

    for (size_t i = 0; i < World::instance().getObjects().size(); i ++)
    {
        CollisionDetails* col = checkCollision(World::instance().getObjects().at(i));
        if (col != NULL)
        {
            collide(col);
            delete col;
        }
    }
}

void FlyingEnemy::collide(CollisionDetails *details)
{
    if (dynamic_cast<Platform*>(details->getCollided()) != NULL || dynamic_cast<Player*>(details->getCollided()) != NULL) {
        if (details->getXStopCollide() != 0) {
            x += details->getXStopCollide();
            if (details->getXStopCollide() > 0)
            {
                setRight(true);
                if (dynamic_cast<Player*>(details->getCollided()) != NULL)
                    setX(getX() + 5);
            }
            if (details->getXStopCollide() < 0)
            {
                setRight(false);
                if (dynamic_cast<Player*>(details->getCollided()) != NULL)
                    setX(getX() - 5);
            }
        }
        if (details->getYStopCollide() != 0) {
            y += details->getYStopCollide();
            if (details->getYStopCollide() < 0)
            {
                setY(getY() + 5);
                up = true;
            }
            if (details->getYStopCollide() > 0)
            {
                setY(getY() + 5);
                up = false;
            }
        }
    }
}
