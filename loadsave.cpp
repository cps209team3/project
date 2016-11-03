/*
 * loadsave.cpp
 */

#include "world.h"
#include "loadsave.h"
#include "object.h"

LoadSave* LoadSave::loadsave = NULL;

LoadSave& LoadSave::instance()
{
	if (loadsave == NULL)
		loadsave = new LoadSave();
	return *loadsave;
}

// loads a save state and configures objects
void LoadSave::load(QString filename)
{
    World::instance().reset();
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		// read lines and configure objects
		QString line = "";
		QTextStream in(&file);

		line = in.readLine();
		int time = line.toInt();

        World::instance().setStartSeconds(time);
		// set up screen object
		line = in.readLine();
		QList<QString> levelDim = line.split(",");
		line = in.readLine();
		QList<QString> screenCoord = line.split(",");
		World::instance().setScreen(new PlayingScreen(
										screenCoord.at(0).toInt(),
										screenCoord.at(1).toInt(),
										0, // don't know the dimensions of the screen here
										0, // will set that in mainwidget.cpp
										levelDim.at(0).toInt(),
										levelDim.at(1).toInt()));

		// player
		line = in.readLine();
		QList<QString> playerCoord = line.split(",");
		int pX = playerCoord.at(1).toInt();
		int pY = playerCoord.at(2).toInt();
		Player* player = new Player(pX, pY, 25, 48, ":/images/maincharacter/stand.png");
        World::instance().setPlayer(player);


		// loop to get platforms and other objects
		size_t numObjs = 0;
		loadObjects(in, numObjs);

		if (World::instance().getObjects().size() != numObjs)
			throw runtime_error("The program did not load all of the objects!");

		file.close();
	}
	else
	{
		throw runtime_error("Failure to open level file");
	}
}

// configures objects from a file <in>
// called at beginning of series of objects in a save file
void LoadSave::loadObjects(QTextStream& in, size_t& num)
{
	num = 0;
	QString line = "";
	Object* obj = NULL;
	while (!in.atEnd())
	{
		line = in.readLine();
		obj = World::instance().createObject(line.split(",").at(0).toStdString());
		if (obj != NULL)
		{
			obj->load(line);
            obj->setVisibility(true);
            World::instance().add(obj);
			num++;
		}
	}
}

// reads object data and saves a save state file
void LoadSave::save(QString filename)
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream out(&file);
		// write lines from Object.save() method
//		for (Object* obj : [the world].objects())
//		{
//			out << obj.save() << "\n"; -- Object::save() method should return a string, a line for the file
//		}
		file.close();
	}
}

void LoadSave::teardown()
{
	delete loadsave;
}
