#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Actor.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include <algorithm>
#include <string>
#include <vector>


// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir)
    : GameWorld(assetDir), m_tunnelMan(NULL), m_earth(NULL), m_actors(NULL),
    m_tick(0), m_barrel(0) {}
    
    //virtual destructor
    virtual ~StudentWorld() {}

    //Required methods
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    //Actor functions to place objects in game
    void addActor(Actor *actor) { m_actors.push_back(actor); }
    std::vector<Actor*> getActors() {return m_actors;}
    void addWaitingActors();
    
    //Mutator functions
    void resetTick() {m_tick = 0;}
    void addTick() {++m_tick;}
    void collectBarrel() { --m_barrel; }
    void removeDeadActors();
    void setDisplayText();
    void decreaseNumofProtestor() { --m_numProtestor; }
    void decreaseNumOfTickBeforeAddingProtestor() { --m_addProtestorTick; }
    void resetAddingProtestorTick();
    
    //Accessor functions
    TunnelMan* getTunnelMan() {return m_tunnelMan;}
    Earth* getEarth(int x, int y);
    Protestor* closestProtestor(int x, int y);
    int getTick() { return m_tick; }
    int getTickBeforeAddingProtestor() { return m_addProtestorTick; }
    int getNumOfProtestor() { return m_numProtestor; }
    int getCurrentGameLevel() { return getLevel(); }
    int getNumLivesLeft() { return getLives(); }
    int getCurrentHealth() { return getTunnelMan()->getHealth(); }
    int getSquirtsLeftInSquirtGun() { return getTunnelMan()->getSquirt(); }
    int getPlayerGoldCount() { return getTunnelMan()->getGold(); }
    int getNumberOfBarrelsRemainingToBePickedUp() { return m_barrel; }
    int getPlayerSonarChargeCount() {return getTunnelMan()->getSonarCharge(); }
    int getCurrentScore() { return getScore(); }
    
    
    //Helper functions
    bool boulderOnBoulder(int x, int y);
    bool containsBoulder(int x, int y);
    bool boulderRadius(int x, int y);
    void validBoulderCoord(int &x, int &y);
    double radiusCalc(double x1, double y1, double x2, double y2);
    bool annoyCloseProtestor(int num, int x, int y);
    bool validMove(GraphObject::Direction dir, int x, int y);
    bool validCoord(int x, int y);
    void validActorCoord(int &x, int &y);
    bool minedEarth(int x, int y);
    bool containsEarth(int x, int y);
    std::string formatDisplayText(std::string &text, int level, int lives, int health, int squirts, int gold,
                                  int barrelsLeft, int sonar, int score);
    void showActors(int x, int y);
    void createPath(GraphObject::Direction game[][VIEW_HEIGHT], int x, int y);
    GraphObject::Direction getExitDirection(int x, int y) { return m_pathToExit[x][y]; }
    GraphObject::Direction getTunnelManDirection(int x, int y) { return m_pathToTunnelMan[x][y]; }
    void validWaterCoord(int &x, int &y);
    bool radiusOfTunnelMan(double radius, int x, int y);
    int distanceToTunnelMan(int x, int y);
    

private:
    TunnelMan *m_tunnelMan; //tunnelMan pointer
    std::vector<Actor*> m_actors; //vector of all actors
    std::vector<std::vector<Earth*>> m_earth; //2d earth vector
    int m_tick; //tick counter
    int m_barrel; //number of barrels in game
    int m_numBoulder; //Number of boulders in game
    int m_numGold; //Number of gold in game
    int m_addProtestorTick; //Number of ticks before adding protestors
    int m_numProtestor; //number of protestors in game
    GraphObject::Direction m_pathToExit[VIEW_WIDTH][VIEW_HEIGHT];
    GraphObject::Direction m_pathToTunnelMan[VIEW_WIDTH][VIEW_HEIGHT];
    
    struct BFS
    {
        BFS(int x, int y)
        {
            m_x = x;
            m_y = y;
        }
        int m_x, m_y;
    };
};

#endif // STUDENTWORLD_H_

