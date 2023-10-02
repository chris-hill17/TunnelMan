#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

//--------------------------------Base Class------------------------------//

class Actor : public GraphObject
{
public:
    //Constructor
    Actor(StudentWorld *studentWorld, int imageID, int startX, int startY, Direction startDirection,
          double size, unsigned int depth)
          : GraphObject(imageID, startX, startY, startDirection, size, depth)
    {
        setVisible(true);
        m_isAlive = true;
        m_studentWorld = studentWorld;
    }
    
    //Destructor
    virtual ~Actor() {}
    
    //pure virtual functions
    virtual void doSomething() = 0;
    virtual void annoyed(int num) = 0;

    //Accessors
    bool isAlive() {return m_isAlive;} //checks if actor is alive/active or dead/inactive
    StudentWorld *getWorld() {return m_studentWorld;} //finds actor in game
    
    //Mutator
    void Dead()
    {
        m_isAlive = false;
        setVisible(false);
    }
    
    double radiusCalc(double x1, double y1, double x2, double y2);

private:
    bool m_isAlive;
    StudentWorld *m_studentWorld;
};

//--------------------------------Tunnel Man--------------------------------//

class TunnelMan : public Actor
{
public:
    //Constructor
    TunnelMan(StudentWorld *studentWorld) :
    Actor(studentWorld, TID_PLAYER, 30, 60, right, 1.0, 0)

    {
        m_hitPts = 10;
        m_water = 5;
        m_sCharge = 1;
        m_gold = 0;
    }
    
    //Destructor
    virtual ~TunnelMan() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {m_hitPts -= num;}
    
    //Mutator function
    void collectGold() { ++m_gold; }
    void collectSonar() { ++m_sCharge; }
    void collectWater() { m_water += 5; }
    
    //Accessor functions
    int getGold() { return m_gold; }
    int getHealth() { return m_hitPts*10; }
    int getSquirt() { return m_water; }
    int getSonarCharge() { return m_sCharge; }
    
    //Helper
    void fireSquirt(Direction dir, int x, int y);
    
private:
    int m_hitPts;
    int m_water;
    int m_sCharge;
    int m_gold;
    
};

//-----------------------------------Earth---------------------------------//

class Earth : public Actor
{
public:
    //Constructor
    Earth(StudentWorld *studentWorld, int x, int y) :
    Actor(studentWorld, TID_EARTH, x, y, right, 0.25, 3)
    {}
    
    //Destructor
    virtual ~Earth() {}
    
    virtual void doSomething() { } //does nothing
    virtual void annoyed(int num) { } //cannot be annoyed so do nothing
};

//-----------------------------------Boulder-------------------------------//

class Boulder : public Actor
{
public:
    //Constructor
    Boulder(StudentWorld *studentWorld, int x, int y) :
    Actor(studentWorld, TID_BOULDER, x, y, down, 1.0, 1)
    {
        m_stable = true;
        m_waiting = false;
        m_falling = false;
    }
    
    //Destructor
    virtual ~Boulder() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {} //cannot be annoyed so do nothing
    
    //Accessor functions
    bool checkStable() { return m_stable; }
    bool isWaiting() { return m_waiting; }
    bool isFalling() { return m_falling; }
    int getTick() { return m_tick; }
    
    //Mutator functions
    
    void addTick() { m_tick += 1; }
    
    
private:
    bool m_stable;
    bool m_waiting;
    bool m_falling;
    int m_tick;
};

//-----------------------------------Squirt---------------------------------//

class Squirt : public Actor
{
public:
    //Constructor
    Squirt(StudentWorld *studentWorld, int x, int y, Direction dir) :
    Actor(studentWorld, TID_WATER_SPURT, x, y, dir, 1.0, 1)
    { m_distance = 4; }
    
    //Destructor
    virtual ~Squirt() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {} //Cannot be annoyed
    
    
private:
    int m_distance;
};

//-----------------------------------Goodies---------------------------------//

class Goodies : public Actor
{
public:
    Goodies(StudentWorld *studentWorld, int imageID, int startX, int startY,
            Direction startDirection, double size, unsigned int depth, bool visible,
            bool permanent, bool tunnelMan, bool protestor) :
    Actor(studentWorld, imageID, startX, startY, startDirection, size, depth)
    {
        m_visible = visible;
        m_tmPickUp = tunnelMan;
        m_pPickUp = protestor;
        m_permanent = permanent;
        setVisible(visible);
    }
    
    //Destructor
    virtual ~Goodies() {}
    
    //pure virtual functions
    virtual void doSomething() = 0;
    virtual void annoyed(int num) = 0;
    
    //Mutator functions
    //void setVisibilty(bool visible) { setVisible(visible); }
    
    //Accessor functions
    bool isVisible() { return m_visible; }
    bool tmPickUp() { return m_tmPickUp; }
    bool pPickUp() { return m_pPickUp; }
    bool isPermanent() { return m_permanent; }

private:
    bool m_visible;
    bool m_tmPickUp;
    bool m_pPickUp;
    bool m_permanent;
};

//-----------------------------------Barrel---------------------------------//

class Barrel : public Goodies
{
public:
    //Constructor
    Barrel(StudentWorld *studentWorld, int x, int y) :
    Goodies(studentWorld, TID_BARREL, x, y, right, 1.0, 2, false, true, true, false)
    { }
    
    //Destructor
    virtual ~Barrel() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {}
    
private:
    
};

//-----------------------------------GoldNugget---------------------------------//

class GoldNugget : public Goodies
{
public:
    //Constructor
    GoldNugget(StudentWorld *studentWorld, int x, int y, bool visible, bool permanent,
               bool tm, bool protestor) :
    Goodies(studentWorld, TID_GOLD, x, y, right, 1.0, 2, visible, permanent, tm, protestor)
    { m_tick = 0; }
    
    //Destructor
    virtual ~GoldNugget() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {}
    
    //mutator function
    void addTick() { ++m_tick; }
    
private:
    int m_tick;
};

//-----------------------------------Sonar Kit---------------------------------//

class Sonar : public Goodies
{
public:
    //Constructor
    Sonar(StudentWorld *studentWorld, int x, int y) :
    Goodies(studentWorld, TID_SONAR, x, y, right, 1.0, 2, true, false, true, false)
    { m_tick = 0; }
    
    //Destructor
    virtual ~Sonar() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {}
    
    //Mutator function
    void addTick() { ++m_tick; }
    
private:
    int m_tick;
};

//-----------------------------------Water Pool---------------------------------//

class WaterPool : public Goodies
{
public:
    //Constructor
    WaterPool(StudentWorld *studentWorld, int x, int y) :
    Goodies(studentWorld, TID_WATER_POOL, x, y, right, 1.0, 2, true, false, true, false)
    { m_tick = 0; }
    
    //Destructor
    virtual ~WaterPool() {}
    
    virtual void doSomething();
    virtual void annoyed(int num) {}
    
    //Mutator function
    void addTick() { ++m_tick; }
    
private:
    int m_tick;
};

//-----------------------------------Protestor---------------------------------//

class Protestor : public Actor
{
public:
    //Constructor
    Protestor(StudentWorld *studentWorld, int imageID);
    
    //Destructor
    virtual ~Protestor() {}
    
    virtual void doSomething();
    virtual void annoyed(int num);
    virtual void pickUpGold() = 0;
    virtual bool hardcore() = 0;
    
    //Mutator
    void setLeaveState() { m_leaveState = true; }
    void stunned();
    void setHealth(int health) { m_hit = health; }
    void resetWaitTime();
    
    
    //Accessor functions
    int getHealth() { return m_hit * 10; }
    bool leavingGame() { return m_leaveState; }
    
    //Helper
    void moveProtestor(Direction dir);
    bool facingTM();
    GraphObject::Direction clearPath();
    GraphObject::Direction randomDirection(int x, int y);
    GraphObject::Direction changeDirection();
    
    
    
private:
    bool m_leaveState;
    int m_hit;
    int m_ticksToWaitBewtweenMoves;
    int m_restTick;
    int m_lastShout;
    int m_lastTurn;
    int m_distanceToMove;
};

class RegularProtestor : public Protestor
{
public:
    //Constructor
    RegularProtestor(StudentWorld *studentWorld) :
    Protestor(studentWorld, TID_PROTESTER)
    { setHealth(5); }
    
    //Destructor
    virtual ~RegularProtestor() {}
    
    virtual void pickUpGold();
    virtual bool hardcore() { return false; }
};

class HardcoreProtestor : public Protestor
{
public:
    //Constructor
    HardcoreProtestor(StudentWorld *studentWorld) :
    Protestor(studentWorld, TID_HARD_CORE_PROTESTER)
    { setHealth(20); }
    
    //Destructor
    virtual ~HardcoreProtestor() {}
    
    virtual void pickUpGold();
    virtual bool hardcore() { return true; }
};

#endif// ACTOR_H_
