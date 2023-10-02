#include "StudentWorld.h"
#include "GameController.h"
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

//----------------------------------------------------------------------------//
//                             Game functions                                 //
//----------------------------------------------------------------------------//

//Creates game
GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir);
}

void Ask_all_actors_to_do_something()
{
    
}

int StudentWorld::init()
{
    m_tick = 0; //Starts game at 0 ticks
    
    m_tunnelMan = new TunnelMan(this); //Creates TunnelMan player
    
    //Get ticks for protestors
    int level = getLevel();
    m_addProtestorTick = max(25, 200-level);
    m_numProtestor = min(15, (int)(2+level*1.5));
    
    //Create earth
    vector<vector<Earth*>> earth(64, vector<Earth*>(60)); //2d vector to hold all earth objects
    for(int i = 0; i < 64; i++)
    {
        for(int j = 0; j < 60; j++)
        {
            if(i < 30 || i > 33 || j < 4)
                earth[i][j] = new Earth(this, i, j);
            else
                earth[i][j] = nullptr;
        }
    }
    m_earth = earth;
    
    //add actors into game
    //vector<Actor*> gameActors;
    //m_actors = gameActors;
    
    //Boulders
    m_numBoulder = min(level/2 + 2, 9);
    for(int i = 0; i < m_numBoulder; i++)
    {
        int x, y;
        validBoulderCoord(x, y);
        addActor(new Boulder(this, x, y));
        
        //Remove earth where bouder spawned
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                m_earth[x+i][y+j]->Dead();
            }
        }
    }
    
    //Gold nuggets
    m_numGold = max(5-level/2, 2);
    for(int i = 0; i < m_numGold; i++)
    {
        int x,y;
        validActorCoord(x, y);
        addActor(new GoldNugget(this, x, y, false, true, true, false));
    }
    
    m_barrel = min(2 + level, 21);
    for(int i = 0; i < m_barrel; i++)
    {
        int x,y;
        validActorCoord(x, y);
        addActor(new Barrel(this, x, y));
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::addWaitingActors()
{
    int level = getLevel();
    int probabilityOfHardcore = min(90, level*10+30);
    int goodieChance = level * 25 + 300;
    bool addHardcoreProtestor = (rand() % probabilityOfHardcore) < 1; //1 in probabilty chance of spawning hardcore first
    bool addGoodieChance = (rand() % goodieChance) < 1;
    
    decreaseNumOfTickBeforeAddingProtestor();
    
    //Must add first protestor durng first tick of game
    if(m_tick == 1)
    {
        if(addHardcoreProtestor)
            addActor(new HardcoreProtestor(this));
        else
            addActor(new RegularProtestor(this));
        //Remove number of protestor waiting to be added
        decreaseNumofProtestor();
        //Reset the waiting tick
        resetAddingProtestorTick();
    }
    else if(getTickBeforeAddingProtestor() <= 0 && getNumOfProtestor() > 0)
    {
        if(addHardcoreProtestor)
            addActor(new HardcoreProtestor(this));
        else
            addActor(new RegularProtestor(this));
        
        //Remove number of protestor waiting to be added
        decreaseNumofProtestor();
        //Reset the waiting tick
        resetAddingProtestorTick();
    }
    
    //Add goodies into game
    if(addGoodieChance)
    {
        // 1/5 chance of adding new sonar kit
        if(rand() % 5 < 1)
            addActor(new Sonar(this, 0, 60));
        
        // 4/5 chance of adding water
        if(rand() % 5 < 4)
        {
            int x, y;
            validWaterCoord(x, y);
            addActor(new WaterPool(this,x,y));
        }
            
    }
    
}

void StudentWorld::resetAddingProtestorTick()
{
    int level = getLevel();
    m_addProtestorTick = max(25, 200-level);
}

int StudentWorld::move()
{
    ++m_tick; //add tick every move
    
    //Update game text
    setDisplayText();
    
    //Add new actors
    addWaitingActors();
    
    //TunnelMan doSomething
    m_tunnelMan->doSomething();
    
    //Path for protestor to exit if annoyed fully
    createPath(m_pathToExit, 60, 60);
    
    //Path to tunnelMan for hardcore protestor
    createPath(m_pathToTunnelMan, m_tunnelMan->getX(), m_tunnelMan->getY());
    
    
    //Allow all actors alive to doSomething
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Actor* actor = *it;
        if(actor->isAlive())
        {
            Protestor *protestor = dynamic_cast<Protestor*>(actor);
            if(protestor)
            {
                //nothing
            }
            else
            {
                actor->doSomething();
            }
        }
        ++it;
    }
    
    
    //Remove all dead items
    removeDeadActors();
    
    //Check if tunnelMan is alive
    if(!m_tunnelMan->isAlive())
    {
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    
    //Check if all barrels have been collected
    if(m_barrel == 0)
        return GWSTATUS_FINISHED_LEVEL;
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete m_tunnelMan; //removes tunnelMan from game
    
    //remove all earth objects
    for(int i = 0; i < m_earth.size(); i++)
    {
        for(int j = 0; j < m_earth[i].size(); j++)
        {
            Earth *ptr = m_earth[i][j];
            delete ptr;
        }
    }
    
    //Clear the vector
    for(int i = 0; i < m_earth[i].size(); i++)
    {
        m_earth[i].clear();
    }
    
    //Clear remaining vector
    m_earth.clear();
    
    //Delete all actors
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Actor *ptr = *it;
        delete ptr;
        it = m_actors.erase(it);
    }
}

//----------------------------------------------------------------------------//
//                          Earth functions                                   //
//----------------------------------------------------------------------------//

bool StudentWorld::minedEarth(int x, int y)
{
    bool mined = false;
    
    //Iterate through each earth object
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            int currX = x + i;
            int currY = y + j;
            
            //check if player mining is in game range
            if((currX >= 0 && currX < 64) && (currY >= 0 && currY < 60))
            {
                Earth *ptr = m_earth[currX][currY];
                
                //Check if earth exist and not already mined
                if(ptr != nullptr && ptr->isAlive())
                {
                    m_earth[currX][currY]->Dead();
                    mined = true;
                }
            }
        }
    }
    
    return mined;
}

Earth* StudentWorld::getEarth(int x, int y)
{
    if((x >= 0 &&  x <= 63) && (y >= 0 && y <= 59))
        return m_earth[x][y];
    else
        return NULL;
}

bool StudentWorld::containsEarth(int x, int y)
{
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(getEarth(x +  i, y + j) != NULL && getEarth(x + i, y + j)->isAlive())
                return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------//
//                          Boulder functions                                 //
//----------------------------------------------------------------------------//

bool StudentWorld::boulderOnBoulder(int x, int y)
{
    //Iterate through vector of actor objects in game to find boulder
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Boulder *boulder = dynamic_cast<Boulder*>(*it);
        if(boulder)
        {
            //Check all 4 squares of boulder
            for(int i = 0; i < 4; i++)
            {
                //Store coord of current boulder
                int curBX = x + i;
                int curBY = y;
                for(int j = 0; j< 4; j++)
                {
                    int otherBX = boulder->getX() + j;
                    int otherBY = boulder->getY() + 3; //checking top squares only
                    
                    //If the current boulders bottom squares touches other boulders top squares, then they collided
                    if(curBX == otherBX && curBY == otherBY)
                        return true;
                }
            }
        }
        ++it;
    }
    return false;
}

bool StudentWorld::containsBoulder(int x, int y)
{
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Boulder *boulder = dynamic_cast<Boulder*>(*it);
        if(boulder)
        {
            int bx = boulder->getX();
            int by = boulder->getY();
            
            //Check if location is the same as any coordinates of a boulder
            for(int i = 0; i < 4; i++)
            {
                for(int j = 0; j < 4; j++)
                {
                    if(x == bx + i && y == by + j)
                        return true; //location overlaps with a boulder
                }
            }
        }
        ++it;
    }
    return false;
}

bool StudentWorld::boulderRadius(int x, int y)
{
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Boulder *boulder = dynamic_cast<Boulder*>(*it);
        if(boulder)
        {
            if(radiusCalc(x, y, boulder->getX(), boulder->getY()) <= 3.0)
                return true;
        }
        ++it;
    }
    return false;
}

void StudentWorld::validBoulderCoord(int &x, int &y)
{
    bool valid = false;
    do
    {
        x = rand() % 61; //Must be x = 60 or less
        y = rand() % 37 + 20; //Must be between y = 20 and y = 56
        valid = true;
        
        //Check if boulder is not withing 6 squares of other objects
        vector<Actor*>::iterator it = m_actors.begin();
        while(it != m_actors.end())
        {
            if(radiusCalc(x, y, (*it)->getX(), (*it)->getY()) <= 6.0)
                valid = false;
            ++it;
        }
        
        //Actors must spawn at least 1 square away from middle shaft and never in shaft
        if((26 <= x && x <= 33) && (1 <= y && y <= 59))
            valid = false;
        
    } while(!valid);
}

//----------------------------------------------------------------------------//
//                          Distance functions                                //
//----------------------------------------------------------------------------//

double StudentWorld::radiusCalc(double x1, double y1, double x2, double y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

bool StudentWorld::radiusOfTunnelMan(double radius, int x, int y)
{
    return radiusCalc(x, y, m_tunnelMan->getX(), m_tunnelMan->getY()) <= radius;
}

int StudentWorld::distanceToTunnelMan(int x, int y)
{
    //Make sure protestor is not already at tunnelman
    if(m_tunnelMan->getX() == x && m_tunnelMan->getY() == y)
        return 0;
    
    if(m_pathToTunnelMan[x][y] == Actor::left)
        return 1 + distanceToTunnelMan(x-1, y);
    else if (m_pathToTunnelMan[x][y] == Actor::right)
        return 1 + distanceToTunnelMan(x+1, y);
    else if (m_pathToTunnelMan[x][y] == Actor::up)
        return 1 + distanceToTunnelMan(x, y+1);
    else if (m_pathToTunnelMan[x][y] == Actor::down)
        return 1 + distanceToTunnelMan(x, y-1);
    
    return -1;
}



//----------------------------------------------------------------------------//
//                          Protestor functions                               //
//----------------------------------------------------------------------------//

Protestor* StudentWorld::closestProtestor(int x, int y)
{
    //Iterator to find all protestors in game
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Protestor* ptr = dynamic_cast<Protestor*>(*it);
        if(ptr)
        {
            if(radiusCalc(x, y, ptr->getX(), ptr->getY()) <= 3.0) //Returns protestor that is within a radius of 3
                return ptr;
        }
        it++;
    }
    return NULL;
}

bool StudentWorld::annoyCloseProtestor(int num, int x, int y)
{
    //Has a protestor been annoyed
    bool annoyed = false;
    //Iterator to find all protestors in game
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        Protestor* ptr = dynamic_cast<Protestor*>(*it);
        if(ptr)
        {
            if(radiusCalc(x, y, ptr->getX(), ptr->getY()) <= 3.0) //Returns protestor that is within a radius of 3
            {
                ptr->annoyed(num);
                annoyed = true;
            }
        }
        it++;
    }
    return annoyed;
}

void StudentWorld::createPath(GraphObject::Direction (*game)[64], int x, int y)
{
    //Create map of no directions first
    for(int i = 0; i < VIEW_WIDTH; i++)
    {
        for(int j = 0; j < VIEW_HEIGHT; j++)
        {
            game[i][j] = GraphObject::Direction::none;
        }
    }
    
    queue<BFS> bfsQueue;
    BFS startPosition(x, y);
    bfsQueue.push(startPosition);
    
    BFS nextPosition(0, 0);
    //Begin searching for possible routes
    while(!bfsQueue.empty())
    {
        nextPosition = bfsQueue.front();
        bfsQueue.pop();
        
        int x = startPosition.m_x;
        int y = startPosition.m_y;
        //Check all directions for possible route
        //Left
        if(validCoord(x - 1, y) && game[x - 1][y] == GraphObject::Direction::none)
        {
            bfsQueue.push(BFS(x-1, y));
            game[x - 1][y] = GraphObject::Direction::left;
        }
        //Right
        if(validCoord(x + 1, y) && game[x + 1][y] == GraphObject::Direction::none)
        {
            bfsQueue.push(BFS(x + 1, y));
            game[x + 1][y] = GraphObject::Direction::right;
        }
        //Up
        if(validCoord(x, y + 1) && game[x][y + 1] == GraphObject::Direction::none)
        {
            bfsQueue.push(BFS(x, y + 1));
            game[x][y + 1] = GraphObject::Direction::up;
        }
        //Down
        if(validCoord(x, y - 1) && game[x][y - 1] == GraphObject::Direction::none)
        {
            bfsQueue.push(BFS(x, y - 1));
            game[x][y - 1] = GraphObject::Direction::down;
        }
    }
}

//----------------------------------------------------------------------------//
//                          Move/Coord functions                              //
//----------------------------------------------------------------------------//

bool StudentWorld::validMove(GraphObject::Direction dir, int x, int y)
{
    switch(dir)
    {
        case GraphObject::left:
            --x;
            break;
        case GraphObject::right:
            ++x;
            break;
        case GraphObject::up:
            ++y;
            break;
        case GraphObject::down:
            --y;
            break;
        case GraphObject::none:
            break;
    }
    
    //Make sure the direction does not go out of bounds
    if(x < 0 || y < 0 || x > 60 || y > 60)
        return false;
    
    //Check if there is earth there
    if(containsEarth(x, y))
        return false;
    
    //Check if there is a boulder there
    if(containsBoulder(x, y))
        return false;
    
    //Check if move in in radius of boulder
    if(boulderRadius(x, y))
        return false;
    
    return true;
}

bool StudentWorld::validCoord(int x, int y)
{
    if(x < 0 || y < 0 || x > 60 || y > 60)
        return false;
    
    //Check if there is earth there
    if(containsEarth(x, y))
        return false;
    
    //Check if there is a boulder there
    if(containsBoulder(x, y))
        return false;
    
    //Check if move in in radius of boulder
    if(boulderRadius(x, y))
        return false;
    
    return true;
}

void StudentWorld::validActorCoord(int &x, int &y)
{
    bool valid = false;
    do
    {
        x = rand() % 61; //Must be x = 60 or less
        y = rand() % 57; //Must be y = 56 or less
        valid = true;
        
        //Check if boulder is not withing 6 squares of other objects
        vector<Actor*>::iterator it = m_actors.begin();
        while(it != m_actors.end())
        {
            if(radiusCalc(x, y, (*it)->getX(), (*it)->getY()) <= 6.0)
                valid = false;
            ++it;
        }
        
        //Actors must spawn at least 1 square away from middle shaft and never in shaft
        if((26 <= x && x <= 33) && (1 <= y && y <= 59))
            valid = false;
        
    } while(!valid);
}

void StudentWorld::validWaterCoord(int &x, int &y)
{
    bool valid = false;
    do
    {
        x = rand() % 60;
        y = rand() % 56;
        valid = true;
        
        //Check to see there is no earth where water will spawn
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                if(getEarth(x+i, y+j) != NULL && getEarth(x+i, y+j)->isAlive())
                    valid = false;
            }
        }
        
        //Check there is no other actor within 6 radius
        vector<Actor*>::iterator it = m_actors.begin();
        while(it != m_actors.end())
        {
            if(radiusCalc(x, y, (*it)->getX(), (*it)->getY()) <= 6.0)
                valid = false;
            ++it;
        }
    } while(!valid);
}

//----------------------------------------------------------------------------//
//                              Actor functions                               //
//----------------------------------------------------------------------------//

void StudentWorld::removeDeadActors()
{
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        if(!(*it)->isAlive())
        {
            Actor *ptr = *it;
            delete ptr;
            it = m_actors.erase(it);
        }
        else
            ++it;
    }
}

void StudentWorld::showActors(int x, int y)
{
    vector<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        if(radiusCalc(x, y, (*it)->getX(), (*it)->getY()) <= 12.0)
            (*it)->setVisible(true);
        ++it;
    }
}

//----------------------------------------------------------------------------//
//                            Game Text functions                             //
//----------------------------------------------------------------------------//

string StudentWorld::formatDisplayText(string &text, int level, int lives, int health, int squirts,
                                       int gold, int barrelsLeft, int sonar, int score)
{
    ostringstream ossLevel;
    ossLevel.fill(' ');
    ossLevel<<setw(2)<<level;
    text += "Lvl: ";
    text += ossLevel.str();
    
    text += " ";
    
    text += "Lives: ";
    text += lives+'0';
    text += " ";
    
    ostringstream ossHealth;
    ossHealth.fill(' ');
    ossHealth<<setw(3)<<health;
    text += "Hlth: ";
    text += ossHealth.str();
    text += "% ";
    
    text += " ";
    
    ostringstream ossWater;
    ossWater.fill(' ');
    ossWater<<setw(2)<<squirts;
    text += "Wtr: ";
    text += ossWater.str();
    
    text += " ";
    
    ostringstream ossGold;
    ossGold.fill(' ');
    ossGold<<setw(2)<<gold;
    text += "Gld: ";
    text += ossGold.str();
    
    text += " ";
    
    ostringstream ossOil;
    ossOil.fill(' ');
    ossOil<<setw(2)<<barrelsLeft;
    text += "Oil Left: ";
    text += ossOil.str();
    
    text += " ";
    
    ostringstream ossSonar;
    ossSonar.fill(' ');
    ossSonar<<setw(2)<<sonar;
    text += "Sonar: ";
    text += ossSonar.str();
    
    text += " ";
    
    ostringstream ossScore;
    ossScore.fill('0');
    ossScore<<setw(6)<<score;
    text += "Scr: ";
    text += ossScore.str();
    
    return text;
}

void StudentWorld::setDisplayText()
{
    int level = getCurrentGameLevel();
    int lives = getNumLivesLeft();
    int health = getCurrentHealth();
    int squirts = getSquirtsLeftInSquirtGun();
    int gold = getPlayerGoldCount();
    int barrelsLeft = getNumberOfBarrelsRemainingToBePickedUp();
    int sonar = getPlayerSonarChargeCount();
    int score = getCurrentScore();
    
    string text = formatDisplayText(text, level, lives, health, squirts, gold, barrelsLeft, sonar, score);
    setGameStatText(text);
}
