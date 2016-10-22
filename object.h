#ifndef OBJECT_H
#define OBJECT_H

#include<QString>
#include"collisiondetails.h"

class CollisionDetails;

class Object
{
protected:
    int x;
    int y;
    int id;
    int width;
    int height;
    QString image;
    static int nextID;

public:
    Object();
    Object(int x_, int y_, int width_, int height_, QString image_): x(x_), y(y_), width(width_), height(height_), image(image_) {
        id = nextID++;
    }

	CollisionDetails* checkCollision(Object * that);
	void setId(int newId) { id = newId; }
    int getId() {return id;}
    static void resetNextId();

};

#endif // OBJECT_H
