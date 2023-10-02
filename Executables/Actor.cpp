#include "Actor.h"
#include "StudentWorld.h"
#include <algorithm>
using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


//----------------------------------------------------------------------------//
//                                 TunnelMan                                  //
//----------------------------------------------------------------------------//
void TunnelMan::doSomething()
{
    if(!isAlive()) //checks to see if tunnelMan is alive
        return;
    
    int ch;
    if(getWorld()->getKey(ch))
    {
        switch(ch)
        {
            case KEY_PRESS_LEFT:
                if(getDirection() == left && getX()-1 >= 0 &&
                   !getWorld()->boulderRadius(getX()-1, getY()))
                    moveTo(getX()-1, getY());
                else
                    setDirection(left);
                break;
            case KEY_PRESS_RIGHT:
                if(getDirection() == right && getX()+1 <= 60 &&
                   !getWorld()->boulderRadius(getX()+1, getY()))
                    moveTo(getX()+1, getY());
                else
                    setDirection(right);
                break;
            case KEY_PRESS_UP:
                if(getDirection() == up && getY()+1 <= 60 &&
                   !getWorld()->boulderRadius(getX(), getY()+1))
                    moveTo(getX(), getY()+1);
                else
                    setDirection(up);
                break;
            case KEY_PRESS_DOWN:
                if(getDirection() == down && getY()-1 >= 0 &&
                   !getWorld()->boulderRadius(getX(), getY()-1))
                    moveTo(getX(), getY()-1);
                else
                    setDirection(down);
                break;
            case KEY_PRESS_ESCAPE:
                Dead();
                break;
            case KEY_PRESS_SPACE:
                //Check if tunnelMan has water charge
                if(m_water > 0)
                {
                    --m_water; //remove one water charge
                    getWorld()->playSound(SOUND_PLAYER_SQUIRT);
                    fireSquirt(getDirection(), getX(), getY());
                }
                break;
            case 'Z':
            case 'z':
                if(m_sCharge > 0)
                {
                    --m_sCharge;
                    getWorld()->showActors(getX(), getY());
                }
                break;
            case KEY_PRESS_TAB:
                if(m_gold > 0)
                {
                    --m_gold;
                    GoldNugget *ptr = new GoldNugget(getWorld(), getX(), getY(),
                                                     true, false, false, true);
                    getWorld()->addActor(ptr);
                }
                break;
                
        }
        if(getWorld()->minedEarth(getX(), getY()))
            getWorld()->playSound(SOUND_DIG);
    }
}

void TunnelMan::fireSquirt(Direction dir, int x, int y)
{
    switch(dir)
    {
        case GraphObject::left:
            //Squirt starts 4 squares in front of tunnelMan
            if(x - 4 >= 0 && !getWorld()->containsEarth(x - 4, y) && !getWorld()->boulderRadius(x - 4, y))
            {
                Squirt *squirt = new Squirt(getWorld(), getX()-4, getY(), left);
                getWorld()->addActor(squirt);
            }
            break;
        case GraphObject::right:
            if(x + 4 <= 60 && !getWorld()->containsEarth(x + 4, y) && !getWorld()->boulderRadius(x + 4, y))
            {
                Squirt *squirt = new Squirt(getWorld(), getX()+4, getY(), right);
                getWorld()->addActor(squirt);
            }
            break;
        case GraphObject::up:
            if(y + 4 <= 57 && !getWorld()->containsEarth(x, y + 4) && !getWorld()->boulderRadius(x, y + 4))
            {
                Squirt *squirt = new Squirt(getWorld(), getX(), getY() + 4, up);
                getWorld()->addActor(squirt);
            }
            break;
        case GraphObject::down:
            if(y - 4 >= 0 && !getWorld()->containsEarth(x, y - 4) && !getWorld()->boulderRadius(x, y - 4))
            {
                Squirt *squirt = new Squirt(getWorld(), getX(), getY() - 4, down);
                getWorld()->addActor(squirt);
            }
            break;
        case none:
            break;
    }
}

double Actor::radiusCalc(double x1, double y1, double x2, double y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

//----------------------------------------------------------------------------//
//                                 Boulder                                    //
//----------------------------------------------------------------------------//
void Boulder::doSomething()
{
    //Check if boulder is alive
    if(!isAlive())
        return;
    
    addTick();
    
    //Check if boulder is in stable state
    if(checkStable())
    {
        int counter = 0;
        //Check the earth below it
        for(int i = 0; i < 4; i++)
        {
            //Checks 4 squares below boulder if the earth exisit but is not alive (mined)
            if(getWorld()->getEarth(getX()+i, getY()-1) != NULL &&
               !getWorld()->getEarth(getX()+i, getY()-1)->isAlive())
            {
                counter++;
            }
        }
        if(counter == 4) //no earth is below boulder
        {
            //Set the boulder to waiting state
            m_stable = false;
            m_waiting = true;
            m_falling = false;
            
            //Reset tick
            m_tick = 0;
        }
    }
    if(isWaiting())
    {
        //Must be in waiting period for 30 ticks
        if(getTick() >= 30)
        {
            //Set boulder into falling state
            m_stable = false;
            m_waiting = false;
            m_falling = true;
            
            //Play sound of boulder falling
            getWorld()->playSound(SOUND_FALLING_ROCK);
        }
    }
    if(isFalling())
    {
        //Move boulder down 1 space each tick
        moveTo(getX(), getY()-1);
        
        //Check if boulder has hit the bottom of the earth
        if(getY() == 0)
        {
            //Set boulder to dead
            Dead();
            return; //end tick
        }
        
        //Check if boulder hits another boulder
        if(getWorld()->boulderOnBoulder(getX(), getY()))
        {
            Dead();
            return;
        }
        
        //Check if boulder hits earth
        for(int i = 0; i < 4; i++)
        {
            if(getWorld()->getEarth(getX()+i, getY()-1) != NULL &&
               getWorld()->getEarth(getX()+i, getY()-1)->isAlive())
            {
                Dead();
                return;
            }
        }
        
        //Check if boulder is in a radius of 3 of tunnelMan
        if(radiusCalc(getX(), getY(), getWorld()->getTunnelMan()->getX(),
                      getWorld()->getTunnelMan()->getY()) <= 3.0)
        {
            //TunnelMan instantly loses life
            getWorld()->getTunnelMan()->annoyed(100);
            getWorld()->getTunnelMan()->Dead();
        }
        
        //Check if boudler is in a radius of 3 of protestors
        getWorld()->annoyCloseProtestor(100, getX(), getY());
        
    }
    
}

//----------------------------------------------------------------------------//
//                                 Squirt                                     //
//----------------------------------------------------------------------------//

void Squirt::doSomething()
{
    //If squirt is in radius 3 of protestor, annoy them
    if(getWorld()->annoyCloseProtestor(2, getX(), getY()))
        Dead();
    
    //Travels full distance
    if(m_distance == 0)
        Dead();
    
    //Continue moving squirt
    if(getWorld()->validMove(getDirection(), getX(), getY()))
    {
        switch(getDirection())
        {
            case GraphObject::left:
                if(getWorld()->validCoord(getX()-1, getY()))
                    moveTo(getX()-1, getY());
                break;
            case GraphObject::right:
                if(getWorld()->validCoord(getX()+1, getY()))
                    moveTo(getX()+1, getY());
                break;
            case GraphObject::up:
                if(getWorld()->validCoord(getX(), getY()+1))
                    moveTo(getX(), getY()+1);
                break;
            case GraphObject::down:
                if(getWorld()->validCoord(getX(), getY()-1))
                    moveTo(getX(), getY()-1);
                break;
            case GraphObject::none:
                break;
        }
        --m_distance;
    }
    else //no longer a valid move (hit something like earth)
        Dead();
}

//----------------------------------------------------------------------------//
//                                   Barrel                                   //
//----------------------------------------------------------------------------//
void Barrel::doSomething()
{
    //Check if barrel is alive
    if(!isAlive())
        return;
    
    //Become visible if tunnelMan is within 4 radius
    if(!isVisible() && radiusCalc(getWorld()->getTunnelMan()->getX(),
                                  getWorld()->getTunnelMan()->getY(),
                                  getX(), getY()) <= 4.0)
    {
        setVisible(true);
        return;
    }
    
    //TunnelMan picks up barrel in radius of 3 or less
    if(radiusCalc(getX(), getY(), getWorld()->getTunnelMan()->getX(),
                  getWorld()->getTunnelMan()->getY()) <= 3.0)
    {
        Dead();
        getWorld()->playSound(SOUND_FOUND_OIL);
        getWorld()->increaseScore(1000);
        getWorld()->collectBarrel();
    }
}

//----------------------------------------------------------------------------//
//                                 GoldNugget                                 //
//----------------------------------------------------------------------------//
void GoldNugget::doSomething()
{
    //Check if gold nugget is alive
    if(!isAlive())
        return;
    
    //Add ticks every call
    addTick();
    
    //Make itself visible if tunnelMan is within 4 radius
    if(!isVisible() && radiusCalc(getX(), getY(), getWorld()->getTunnelMan()->getX(),
                                  getWorld()->getTunnelMan()->getY()) <= 4.0)
    {
        setVisible(true);
        return;
    }
    
    //TunnelMan picks up gold nugget if its pickupable and within radius of 3
    if(tmPickUp() && radiusCalc(getX(), getY(), getWorld()->getTunnelMan()->getX(),
                  getWorld()->getTunnelMan()->getY()) <= 3.0)
    {
        Dead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(10);
        getWorld()->getTunnelMan()->collectGold();
    }
    
    //Protestor picks up gold if pickupable and within radius of 3
    if(pPickUp() && getWorld()->closestProtestor(getX(), getY()) != NULL)
    {
        Dead();
        getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
        getWorld()->increaseScore(25);
        getWorld()->closestProtestor(getX(), getY())->pickUpGold();
    }
    
    //Check lifetime if not permanent
    if(!isPermanent())
    {
        //gold can only stay alive for 100 ticks
        if(m_tick >= 100)
            Dead();
    }
}

//----------------------------------------------------------------------------//
//                                   Sonar                                    //
//----------------------------------------------------------------------------//
void Sonar::doSomething()
{
    //Check if sonar is alive/active
    if(!isAlive())
        return;
    
    //Add ticks every call
    addTick();
    
    //Becomes active if tunnelMan is within radius of 3
    if(radiusCalc(getX(), getY(), getWorld()->getTunnelMan()->getX(),
                  getWorld()->getTunnelMan()->getY()) <= 3.0)
    {
        Dead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->getTunnelMan()->collectSonar();
        getWorld()->increaseScore(75);
    }
    
    //Check lifetime of sonar kit
    int level = getWorld()->getLevel();
    int maxLife = max(100, 300-10*level);
    
    if(m_tick >= maxLife)
        Dead();
}

//----------------------------------------------------------------------------//
//                                 WaterPool                                  //
//----------------------------------------------------------------------------//

void WaterPool::doSomething()
{
    //Check id waterPool is alive/active
    if(!isAlive())
        return;
    
    //Add tick every call
    addTick();
    
    //Water pool activates if tunnelMan is within radius of 3
    if(radiusCalc(getX(), getY(), getWorld()->getTunnelMan()->getX(),
                  getWorld()->getTunnelMan()->getY()) <= 3.0)
    {
        Dead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->getTunnelMan()->collectWater();
        getWorld()->increaseScore(100);
    }
    
    //Check lifetime of water
    int level = getWorld()->getLevel();
    int maxLife = max(100, 300-10*level);
    
    if(m_tick >= maxLife)
        Dead();
}

//----------------------------------------------------------------------------//
//                                 Protestor                                  //
//----------------------------------------------------------------------------//

Protestor::Protestor(StudentWorld *studentWorld, int imageID) :
Actor(studentWorld, imageID, 56, 60, left, 1.0, 0)
{
    m_leaveState = false;
    m_restTick = 0;
    m_lastShout = 0;
    m_lastTurn = 0;
    m_hit = 0;
    m_distanceToMove = rand() % 53 + 8;
    int level = getWorld()->getLevel();
    m_ticksToWaitBewtweenMoves = max(0, 3-level/4);
    setVisible(true);
}

void Protestor::stunned()
{
    int level = getWorld()->getLevel();
    m_ticksToWaitBewtweenMoves = max(50, 100-level*10);
}

void Protestor::resetWaitTime()
{
    int level = getWorld()->getLevel();
    m_ticksToWaitBewtweenMoves = max(0, 3-level/4);
}

void Protestor::annoyed(int num)
{
    //Make sure Protestor is not already leaving the game
    if(m_leaveState)
        return;
    
    //Reduce health by stun amount
    m_hit -= num;
    
    //Initial stun but still alive
    if(m_hit > 0)
    {
        getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
        //Start the rest ticks
        stunned();
    }
    //Protestor health dropped below 0
    else
    {
        setLeaveState();
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        //Reset resting tick to 0 since protestor is leaving
        m_ticksToWaitBewtweenMoves = 0;
        //Check if protestor was annoyed by boulder
        if(num == 100)
            getWorld()->increaseScore(500);
        //Check if protestor was annoyed by squirt gun
        if(num == 2)
            getWorld()->increaseScore(100);
    }
}

void Protestor::doSomething()
{
    //Check if alive
    if(!isAlive())
        return;
    
    //Check if protestor is in resting state
    if(m_ticksToWaitBewtweenMoves > 0)
    {
        --m_ticksToWaitBewtweenMoves;
        return;
    }
    else
    {
        //Reset wait time for next move
        if(!m_leaveState)
            resetWaitTime();
        //Increase ticks from last time it shouted
        ++m_lastShout;
        ++m_lastTurn;
    }
    
    //Check if protestor entered leave state
    if(leavingGame())
    {
        //Set protestor to dead once they reach the exit
        if(getX() == 60 && getY() == 60)
        {
            Dead();
            return;
        }
        else
        {
            GraphObject::Direction dir = getWorld()->getExitDirection(getX(), getY());
            moveProtestor(dir);
            return;
        }
    }
    
    //Shout at tunnelMan if near and facing
    if(getWorld()->radiusOfTunnelMan(4.0, getX(), getY()) && facingTM() &&
       m_lastShout > 15)
    {
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        getWorld()->getTunnelMan()->annoyed(2);
        //Reset last shout ticker
        m_lastShout = 0;
        return;
    }
    
    //Harcore protestor is in radius of tunnelman
    if(hardcore())
    {
        //Distance harcore can see tunnelman per level
        int distance = 16 + getWorld()->getLevel() * 2;
        
        if(getWorld()->distanceToTunnelMan(getX(), getY() != -1 &&
                                           getWorld()->distanceToTunnelMan(getX(), getY()) <= distance))
        {
            setDirection(getWorld()->getTunnelManDirection(getX(), getY()));
            moveProtestor(getDirection());
            return;
        }
    }
    
    //Move protestors to tunnelman if there is a direct line of sight
    if(!getWorld()->radiusOfTunnelMan(4.00, getX(), getY()) && clearPath() != none)
    {
        setDirection(clearPath());
        moveProtestor(getDirection());
        m_distanceToMove = 0;
        return;
    }
    
    //Protestor cannot see tunnelman so just move anywhere
    --m_distanceToMove;
    if(m_distanceToMove <= 0)
    {
        setDirection(randomDirection(getX(), getY()));
        m_distanceToMove = rand()%53+8;
    }
    
    //Move protestor
    moveProtestor(getDirection());
    
    //Change directions if stuck/cannot move
    GraphObject::Direction dir = getDirection();
    switch(dir)
    {
        case GraphObject::left:
            if(!getWorld()->validCoord(getX()-1, getY()))
                setDirection(changeDirection());
            break;
        case GraphObject::right:
            if(!getWorld()->validCoord(getX()+1, getY()))
                setDirection(changeDirection());
            break;
        case GraphObject::up:
            if(!getWorld()->validCoord(getX(), getY()+1))
                setDirection(changeDirection());
            break;
        case GraphObject::down:
            if(!getWorld()->validCoord(getX(), getY()-1))
                setDirection(changeDirection());
            break;
        case GraphObject::none:
            break;
    }
     
        
}

void Protestor::moveProtestor(Direction dir)
{
    switch(dir)
    {
        case GraphObject::left:
            if(getWorld()->validCoord(getX()-1, getY()))
                moveTo(getX()-1, getY());
            break;
        case GraphObject::right:
            if(getWorld()->validCoord(getX()+1, getY()))
                moveTo(getX()+1, getY());
            break;
        case GraphObject::up:
            if(getWorld()->validCoord(getX(), getY()+1))
                moveTo(getX(), getY()+1);
            break;
        case GraphObject::down:
            if(getWorld()->validCoord(getX(), getY()-1))
                moveTo(getX(), getY()-1);
            break;
        case GraphObject::none:
            break;
    }
    
}

bool Protestor::facingTM()
{
    GraphObject::Direction dir = getDirection();
    
    bool left = (dir == GraphObject::left && getWorld()->getTunnelMan()->getX() <= getX());
    bool right = (dir == GraphObject::right && getWorld()->getTunnelMan()->getX() >= getX());
    bool up = (dir == GraphObject::up && getWorld()->getTunnelMan()->getY() >= getY());
    bool down = (dir == GraphObject::down && getWorld()->getTunnelMan()->getY() <= getY());
    
    return left || right || up || down;
}

GraphObject::Direction Protestor::clearPath()
{
    int distanceCount = 0;
    //Tunnelman and protestor are on the same x axis
    if((getX() == getWorld()->getTunnelMan()->getX()))
    {
        int distance = abs(getY() - getWorld()->getTunnelMan()->getY());
        //Check down
        if(getY() > getWorld()->getTunnelMan()->getY())
        {
            for(int i = 0; i < distance; i++)
            {
                if(getWorld()->validCoord(getX(), getY()-i))
                    ++distanceCount;
            }
            
            if(distanceCount == distance)
                return GraphObject::Direction::down;
        }
        //Check up
        else
        {
            for(int i = 0; i < distance; i++)
            {
                if(getWorld()->validCoord(getX(), getY()+i))
                    ++distanceCount;
            }
                
            if(distanceCount == distance)
                return GraphObject::Direction::up;
        }
    }
    else if((getY() == getWorld()->getTunnelMan()->getY()))
    {
        int distance = abs(getX() - getWorld()->getTunnelMan()->getX());
        //Check left
        if(getX() > getWorld()->getTunnelMan()->getX())
        {
            for(int i = 0; i < distance; i++)
            {
                if(getWorld()->validCoord(getX()-i, getY()))
                    ++distanceCount;
            }
            
            if(distanceCount == distance)
                return GraphObject::Direction::left;
        }
        //Check right
        else
        {
            for(int i = 0; i < distance; i++)
            {
                if(getWorld()->validCoord(getX()+i, getY()))
                    ++distanceCount;
            }
                
            if(distanceCount == distance)
                return GraphObject::Direction::right;
        }
    }
    return GraphObject::Direction::none;
    
}

GraphObject::Direction Protestor::randomDirection(int x, int y)
{
    for(;;)
    {
        int num = rand() % 4;
        switch(num)
        {
            case 0:
                if(getWorld()->validCoord(getX()-1, getY()))
                    return GraphObject::left;
            case 1:
                if(getWorld()->validCoord(getX()+1, getY()))
                    return GraphObject::right;
            case 2:
                if(getWorld()->validCoord(getX(), getY()+1))
                    return GraphObject::up;
            case 3:
                if(getWorld()->validCoord(getX(), getY()-1))
                    return GraphObject::down;
        }
    }
}

void RegularProtestor::pickUpGold()
{
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(25);
    stunned();
}

void HardcoreProtestor::pickUpGold()
{
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(50);
    stunned();
}

GraphObject::Direction Protestor::changeDirection()
{
    GraphObject::Direction dir = getDirection();
    
    switch(dir)
    {
        case GraphObject::left:
        case GraphObject::right:
            //Both left and right moves are valid
            if(getWorld()->validCoord(getX()-1, getY()) && getWorld()->validCoord(getX()+1, getY()))
            {
                //Randomly select to move left or right
                if(rand() % 2 == 0)
                    return GraphObject::left;
                else
                    return GraphObject::right;
            }
            else if(getWorld()->validCoord(getX()-1, getY()))
                return GraphObject::left;
            else if(getWorld()->validCoord(getX()+1, getY()))
                return GraphObject::right;
            return GraphObject::none;
            break;
        case GraphObject::up:
        case GraphObject::down:
            //Both left and right moves are valid
            if(getWorld()->validCoord(getX(), getY()-1) && getWorld()->validCoord(getX(), getY()+1))
            {
                //Randomly select to move left or right
                if(rand() % 2 == 0)
                    return GraphObject::up;
                else
                    return GraphObject::down;
            }
            else if(getWorld()->validCoord(getX(), getY()-1))
                return GraphObject::down;
            else if(getWorld()->validCoord(getX(), getY()+1))
                return GraphObject::up;
            return GraphObject::none;
            break;
        case GraphObject::none:
            break;
    }
    return GraphObject::none;
}
