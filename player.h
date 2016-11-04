#ifndef PLAYER_H
#define PLAYER_H

#include "object.h"

class Player : public Object
{
    bool jumpOnMove;
    int numLives;
    bool canMove;
    bool reachedEndGame;
	int startX, startY;

public:
    Player() : Object(),jumpOnMove(false), numLives(3), reachedEndGame(false) {}
    Player(int x_, int y_, int width_, int height_, QString image_) :
        Object(x_, y_, width_, height_, image_), jumpOnMove(false), numLives(3), canMove(true), reachedEndGame(false), xSpeedLimit(9), standCount(0) {}

	int xSpeedLimit;
	int standCount;

	// powerup booleans.
	bool powerjump;
	bool powerspeed;
	bool powershield;
	bool powerscore;

    bool kicking;

	virtual void load(QString config);
	virtual QString save();

    void jump();
    void moveRight();
    void moveLeft();
    void slowToStop();
    void move();
    virtual void collide(CollisionDetails *details);

    int getNumLives() { return numLives; }
    void setNumLives(int newNumLives) { numLives = newNumLives; }
    bool canJumpOnMove() { return jumpOnMove; }
    void setJumpOnMove(bool initJump) { jumpOnMove = initJump; }

    bool getCanMove() { return canMove; }
    void setCanMove(bool b) { canMove = b; }

    bool getIsAtEndOfLevel() { return reachedEndGame; }
    void setAtEndOfLevel(bool stateOfPlayer) { reachedEndGame = stateOfPlayer; }

	int getStartX() {return startX;}
	int getStartY() {return startY;}
	void setStartX(int newX) {startX = newX;}
	void setStartY(int newY) {startY = newY;}
	void setStartPoint(int newX, int newY) {startX = newX; startY = newY;}

    bool powerJump() { return powerjump; }
    bool powerSpeed() { return powerspeed; }
    bool powerShield() { return powershield; }
    bool powerScore() { return powerscore; }
    void setPower(string, bool);

    bool isKicking() { return kicking; }
    void setKicking(bool b) { kicking = b; }

    void setWalkImage();

    void setXSpeedLimit(int limit) { xSpeedLimit = limit; }
};

#endif // PLAYER_H
