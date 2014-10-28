//
//  GameState.h
//  LevelSolver
//

#ifndef __LevelSolver__GameState__
#define __LevelSolver__GameState__

#include <iostream>
#include "Settings.h"
#include "SolverHelperTypes.h"
#include "SpookyHash.h"

using namespace std;


unordered_map<pair<OccupantKey,Dir>,Dir> moveAlongSnakeMap;

class GameState
{
public:
    OccupantArray layout;
    GameState* previousState;
    int numMoves;
    int playerPos;
    int lastMovedIndex;
    Dir lastMovedDir;
    bool newCharToMove;
    
    GameState()
    {
        numMoves=0;
        lastMovedIndex =-1;
        previousState = NULL;
        lastMovedDir = Dir::Z;
    }
    
    GameState(GameState* stateToCopy)
    {
        previousState = stateToCopy;//stateToCopy->previousState;
        layout.copyFrom(&stateToCopy->layout);
        playerPos = stateToCopy->playerPos;
        numMoves = stateToCopy->numMoves;
        lastMovedDir = stateToCopy->lastMovedDir;
        lastMovedIndex = stateToCopy->lastMovedIndex;
    }
    
    void setPlayerPos(int newPos)
    {
        layout.setIndex(playerPos,OccupantKey::Empty);
        playerPos = newPos;
        layout.setIndex(playerPos,OccupantKey::Player);
    }
    
    void print();
    
    bool operator==(const GameState& rhs)const
    {
        for(int i =0; i< LEVEL_SIZE_HALF; i++)
        {
            if(layout.data[i] != rhs.layout.data[i])
                return false;
        }
        return true;
    }
    
    
    
    int getSnakeTail(int snakeIndex )
    {
        Dir currentdir = moveAlongSnakeMap[make_pair(layout[snakeIndex], Dir::Z)];
        
        while(currentdir != Dir::Z)
        {
            snakeIndex+= dirToDiffMap[currentdir];
            currentdir = moveAlongSnakeMap[make_pair(layout[snakeIndex], currentdir)];
        }
        return snakeIndex;
    }
    
    //returns new tail index
    int moveSnake(int snakeIndex,Dir pullDir, int tailIndex)
    {
        //no error checking for speed's sake
        int newHead = snakeIndex + dirToDiffMap[pullDir];
        lastMovedIndex = newHead;
        lastMovedDir = pullDir;
        
        //1. find tail
        if(tailIndex < 0)
            tailIndex = getSnakeTail(snakeIndex);
        
        Dir tailDir = moveAlongSnakeMap[make_pair(layout[tailIndex],Dir::Z)];
        int secondToTail= tailIndex + dirToDiffMap[tailDir];
        
        if(secondToTail == snakeIndex)
        {
            //two seg snake
            layout.setIndex(tailIndex,OccupantKey::Empty);
            layout.setIndex(secondToTail, dirsToOccMap[make_pair(Dir::Z,pullDir)]);
            layout.setIndex(newHead,dirsToOccMap[make_pair(Dir::Z, oppositeDirsMap[pullDir])]);
            return secondToTail;
        }
        
        
        //2. set new tail
        Dir newTailDir =moveAlongSnakeMap[make_pair(layout[secondToTail],tailDir)];
        layout.setIndex(secondToTail,dirsToOccMap[make_pair(Dir::Z, newTailDir)]);
        layout.setIndex(tailIndex,OccupantKey::Empty);
        
        
        //3.move head
        layout.setIndex(newHead,dirsToOccMap[make_pair(Dir::Z, oppositeDirsMap[pullDir])]);
        
        //4. fix old head
        Dir oldHeadDir = moveAlongSnakeMap[make_pair(layout[snakeIndex], Dir::Z)];
        layout.setIndex(snakeIndex,dirsToOccMap[make_pair(oppositeDirsMap[pullDir], oldHeadDir)]);
        
        return secondToTail;
    }
    
    /*  GameState CopyState()
     {
     GameState newState;
     for(int i =0; i< LEVEL_SIZE; i++)
     {
     newState.layout[i] = layout[i];
     }
     
     return newState;
     }*/
};




class GameStateRef
{
public:
    GameState* gameState;
    
    GameStateRef()
    {
        
    }
    
    GameStateRef(GameState* gs)
    {
        gameState = gs;
    }
    
    bool operator==(const GameStateRef& rhs)const
    {
        return *gameState == *rhs.gameState;
    }
};

struct GameStateRefEqu
{
    bool operator()(const GameStateRef s1, const GameStateRef s2) const
    {
        return *s1.gameState == *s2.gameState;
    }
};


namespace std {
    template <> struct hash<GameState>
    {
        size_t operator()(const GameState & x) const
        {
            return SpookyHash::Hash64(x.layout.data, LEVEL_SIZE_HALF, time(NULL));
        }
    };
    
    template <> struct hash<GameState *>
    {
        size_t operator()(const GameState* & x) const
        {
            return SpookyHash::Hash64(x->layout.data, LEVEL_SIZE_HALF, time(NULL));
        }
    };
    
    template <> struct hash<GameStateRef>
    {
        size_t operator()(const GameStateRef & x) const
        {
            return SpookyHash::Hash64(x.gameState->layout.data, LEVEL_SIZE_HALF, time(NULL));
        }
    };
}


#endif /* defined(__LevelSolver__GameState__) */
