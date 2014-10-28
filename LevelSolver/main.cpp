//
//  main.cpp
//  LevelSolver
//

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include "SpookyHash.h"
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <set>
#include <ctime>

#include <google/sparse_hash_map>
#include <google/sparse_hash_set>
#include <google/dense_hash_map>
#include <google/dense_hash_set>

#include "Settings.h"
#include "SolverHelperTypes.h"
#include "GameState.h"

using google::sparse_hash_map;
using google::sparse_hash_set;
using namespace std;


hash_table<char,OccupantKey> charToKeyMap;
hash_table<OccupantKey,char> keyToCharMap;


void GameState::print()
{
    for(int y=0; y< LEVEL_DIM; y++)
    {
        for(int x=0; x<LEVEL_DIM; x++)
        {
            cout<< keyToCharMap[layout[y*LEVEL_DIM +x]];
        }
        cout<<'\n';
    }
}


void populateSnakeHelpers()
{
    dirToDiffMap[Dir::N] =-1*LEVEL_DIM;
    dirToDiffMap[Dir::S] =LEVEL_DIM;
    dirToDiffMap[Dir::W] =-1;
    dirToDiffMap[Dir::E] =1;
    dirToDiffMap[Dir::Z] =0;
    
    
    moveAlongSnakeMap[make_pair(OccupantKey::BendUR, Dir::N)] = Dir::E;
    moveAlongSnakeMap[make_pair(OccupantKey::BendUR, Dir::W)] = Dir::S;
    
    moveAlongSnakeMap[make_pair(OccupantKey::BendRU, Dir::E)] = Dir::N;
    moveAlongSnakeMap[make_pair(OccupantKey::BendRU, Dir::S)]= Dir::W;
    
    moveAlongSnakeMap[make_pair(OccupantKey::BendRD, Dir::E)] = Dir::S;
    moveAlongSnakeMap[make_pair(OccupantKey::BendRD, Dir::N)]=Dir::W;
    
    moveAlongSnakeMap[make_pair(OccupantKey::BendDR, Dir::S)] = Dir::E;
    moveAlongSnakeMap[make_pair(OccupantKey::BendDR, Dir::W)] = Dir::N;
    
    moveAlongSnakeMap[make_pair(OccupantKey::EndD,Dir::Z)] = Dir::S;
    moveAlongSnakeMap[make_pair(OccupantKey::EndD, Dir::N)] = Dir::Z;
    
    moveAlongSnakeMap[make_pair(OccupantKey::EndU, Dir::Z)] = Dir::N;
    moveAlongSnakeMap[make_pair(OccupantKey::EndU, Dir::S)] = Dir::Z;
    
    moveAlongSnakeMap[make_pair(OccupantKey::EndL, Dir::Z)] = Dir::W;
    moveAlongSnakeMap[make_pair(OccupantKey::EndL, Dir::E)] = Dir::Z;
    
    moveAlongSnakeMap[make_pair(OccupantKey::EndR, Dir::Z)] = Dir::E;
    moveAlongSnakeMap[make_pair(OccupantKey::EndR, Dir::W)] = Dir::Z;
    
    moveAlongSnakeMap[make_pair(OccupantKey::Horizontal, Dir::E)] = Dir::E;
    moveAlongSnakeMap[make_pair(OccupantKey::Horizontal, Dir::W)] = Dir::W;
    
    moveAlongSnakeMap[make_pair(OccupantKey::Vertical, Dir::N)] = Dir::N;
    moveAlongSnakeMap[make_pair(OccupantKey::Vertical, Dir::S)] = Dir::S;
    
    //ugh.  I bet I could have made this easier on myself...
    dirsToOccMap[make_pair(Dir::N, Dir::N)] = OccupantKey::Vertical;
    dirsToOccMap[make_pair(Dir::S, Dir::S)] = OccupantKey::Vertical;
    
    dirsToOccMap[make_pair(Dir::E, Dir::E)] = OccupantKey::Horizontal;
    dirsToOccMap[make_pair(Dir::W, Dir::W)] = OccupantKey::Horizontal;
    
    dirsToOccMap[make_pair(Dir::E, Dir::N)] = OccupantKey::BendRU;
    dirsToOccMap[make_pair(Dir::E, Dir::S)] = OccupantKey::BendRD;
    
    dirsToOccMap[make_pair(Dir::S, Dir::E)] = OccupantKey::BendDR;
    dirsToOccMap[make_pair(Dir::S, Dir::W)] = OccupantKey::BendRU;
    
    dirsToOccMap[make_pair(Dir::N, Dir::E)] = OccupantKey::BendUR;
    dirsToOccMap[make_pair(Dir::N, Dir::W)] = OccupantKey::BendRD;
    
    dirsToOccMap[make_pair(Dir::W, Dir::N)] = OccupantKey::BendDR;
    dirsToOccMap[make_pair(Dir::W, Dir::S)] = OccupantKey::BendUR;
    
    dirsToOccMap[make_pair(Dir::Z, Dir::N)]  = OccupantKey::EndU;
    dirsToOccMap[make_pair(Dir::Z, Dir::S)]  = OccupantKey::EndD;
    dirsToOccMap[make_pair(Dir::Z, Dir::E)]  = OccupantKey::EndR;
    dirsToOccMap[make_pair(Dir::Z, Dir::W)]  = OccupantKey::EndL;
    
    //sigh
    oppositeDirsMap[Dir::Z] = Dir::Z;
    oppositeDirsMap[Dir::N] = Dir::S;
    oppositeDirsMap[Dir::S] = Dir::N;
    oppositeDirsMap[Dir::E] = Dir::W;
    oppositeDirsMap[Dir::W] = Dir::E;
}



//populate maps
void addKeyMap(char c,OccupantKey k )
{
    charToKeyMap[c]=k;
    keyToCharMap[k]= c;
}

int newCounter =0;

uint8_t endIndex;

hash_set<GameStateRef> stateIndex;
queue<GameState*> stateQueue;

GameState* solvedState = NULL;


//used to order states by how far the player is from the goal
struct GameStateRefComp {
    bool operator() (const GameStateRef& lhs, const GameStateRef& rhs) const
    {
        int lhsDist = LEVEL_DIM - lhs.gameState->playerPos % LEVEL_DIM;
        int rhsDist = LEVEL_DIM - rhs.gameState->playerPos % LEVEL_DIM;
        
        lhsDist+=abs(int(lhs.gameState->playerPos/LEVEL_DIM) - endIndex/LEVEL_DIM);
        rhsDist+=abs(int(rhs.gameState->playerPos/LEVEL_DIM) - endIndex/LEVEL_DIM);
        
        if(lhsDist == rhsDist)
        {
            return lhs.gameState->playerPos < rhs.gameState->playerPos;
        }
        
        return lhsDist<rhsDist;
    }
};



GameState* stringToGameState(string code)
{
    GameState* rtnval = new GameState();
    rtnval->numMoves = 0;
    for(int i=0; i<LEVEL_SIZE; i++)
    {
        if(code[i] == 'e')
            endIndex = i;
        else if(code[i] == '@')
            rtnval->playerPos = i;
        rtnval->layout.setIndex(i,charToKeyMap[code[i]]);
    }
    rtnval->lastMovedDir = Dir::Z;
    rtnval->lastMovedIndex = -1;
    return rtnval;
}

    
    hash_set<size_t> collitions;
    int colitionCount = 0;
    
    
bool tryAddToStateQueue(GameState * state)
{
    if(!stateIndex.insert(GameStateRef(state)).second){
        return false;
    }
    
   /* if(!collitions.insert(SpookyHash::Hash64(state->layout.data, LEVEL_SIZE_HALF, time(NULL))).second)
    {
        colitionCount++;
        cout<<colitionCount<<" out of "<<stateIndex.size()<<endl;
    }*/
    
    stateQueue.push(state);
    return true;
}

GameState * tryAddToStateQueueOrGetPrev(GameState * state)
{
    auto result =stateIndex.insert(GameStateRef(state));
    if(!result.second){
        return result.first->gameState;
    }
    
    stateQueue.push(state);
    return NULL;
}

GameState * tryMovePlayerInDir(GameState * startState, Dir direction)
{
    int playerPos = startState->playerPos;
    int newPos = playerPos;
    switch (direction) {
        case Dir::E:
            if( (playerPos+1) % LEVEL_DIM == 0 || startState->layout[playerPos+1] != OccupantKey::Empty)
                return NULL;
            newPos = playerPos+1;
            break;
        case Dir::W:
            if( playerPos % LEVEL_DIM == 0 || startState->layout[playerPos-1] != OccupantKey::Empty)
                return NULL;
            newPos = playerPos-1;
            break;
        case Dir::N:
            if(playerPos-LEVEL_DIM < 0 || startState->layout[playerPos-LEVEL_DIM] != OccupantKey::Empty)
                return NULL;
            newPos =playerPos-LEVEL_DIM;
            break;
        case Dir::S:
            if(playerPos+LEVEL_DIM >= LEVEL_SIZE || startState->layout[playerPos+LEVEL_DIM] != OccupantKey::Empty)
                return NULL;
            newPos =playerPos+LEVEL_DIM;
            break;
        default:
            break;
    }
    
    GameState * rtnval = new GameState(startState);
    rtnval->previousState = startState;
    rtnval->lastMovedDir = direction;
    rtnval->setPlayerPos(newPos);
    rtnval->lastMovedIndex = newPos;
    return rtnval;
}

#pragma mark playerMovements


// add all player movements to the state queue
void addAllPlayerMovements(GameState * rootState)
{
    bool triedSpots[LEVEL_SIZE] = {};
    vector<GameState *> newStates;
    queue<GameState *> fillQueue;
    fillQueue.push(rootState);
    
    int newMoveNum = rootState->numMoves+1;
    
    while(fillQueue.size() > 0)
    {
        GameState * startState = fillQueue.front();
        if(startState->playerPos == endIndex)
        {
            solvedState = startState;
            return;
        }
        fillQueue.pop();
        
        GameState * newState;
        
        //add up and down
        if((newState = tryMovePlayerInDir(startState, Dir::S))!=NULL)
        {
            newState->numMoves = newMoveNum;
            if(!triedSpots[newState->playerPos])
            {
                triedSpots[newState->playerPos]=true;
                newStates.push_back(newState);
                fillQueue.push(newState);
            }
        }
        
        if((newState = tryMovePlayerInDir(startState, Dir::N))!=NULL)
        {
            newState->numMoves = newMoveNum;
            if(!triedSpots[newState->playerPos])
            {
                triedSpots[newState->playerPos]=true;
                newStates.push_back(newState);
                fillQueue.push(newState);
            }
        }
        
        if((newState = tryMovePlayerInDir(startState, Dir::W))!=NULL)
        {
            newState->numMoves = newMoveNum;
            if(!triedSpots[newState->playerPos])
            {
                triedSpots[newState->playerPos]=true;
                newStates.push_back(newState);
                fillQueue.push(newState);
            }
        }
        if((newState = tryMovePlayerInDir(startState, Dir::E))!=NULL)
        {
            newState->numMoves = newMoveNum;
            if(!triedSpots[newState->playerPos])
            {
                triedSpots[newState->playerPos]=true;
                newStates.push_back(newState);
                fillQueue.push(newState);
            }
        }
    }
    
    for (const auto& state: newStates) {
        tryAddToStateQueue(state);
    }
}


unordered_map<GameStateRef,bool> snakeMoveIndex;
queue<pair<GameState*,int>> snakeMoveQueue;

void newaddAllSnakeMovements(GameState * rootState, int snakeIndex)
{
    snakeMoveIndex.clear();
    GameState* newRoot = new GameState(rootState);
    newRoot->lastMovedIndex = snakeIndex;
    newRoot->numMoves++;
    newRoot->previousState = rootState;
    newRoot->lastMovedDir = Dir::Z;
    snakeMoveIndex[GameStateRef(newRoot)]=true;
    
    //get tail
    int tail = newRoot->getSnakeTail(snakeIndex);
    snakeMoveQueue.push(pair<GameState*,int>(newRoot,tail));
    
    auto helper = [&] (GameState* root, int snakeIndex, int indexToCheck, int tail, Dir dir)
    {
        bool isLoop = false;
        if(root->layout[indexToCheck] != OccupantKey::Empty)
        {
            if(indexToCheck == tail)
            {
                isLoop = true;
            }
            else
            {
                return;
            }
        }
        
        GameState * moved = new GameState(root);
        moved->previousState = root;
        int newTail = moved->moveSnake(snakeIndex,dir, tail);
        
        
        if(snakeMoveIndex.insert(pair<GameStateRef,bool>(GameStateRef(moved),isLoop)).second)
        {
            snakeMoveQueue.push(pair<GameState*,int>(moved, newTail));
        }
        else
        {
            delete moved;
        }
    };
    
    while(snakeMoveQueue.size()>0)
    {
        newRoot = snakeMoveQueue.front().first;
        int tail =snakeMoveQueue.front().second;
        snakeMoveQueue.pop();
        
        snakeIndex = newRoot->lastMovedIndex;
        OccupantKey headType = newRoot->layout[snakeIndex];
        
        //get tail
        //int tail = newRoot->getSnakeTail(snakeIndex);
        
        //if move onto empty space, new state
        //south
        if(headType != OccupantKey::EndD &&
           snakeIndex + LEVEL_DIM < LEVEL_SIZE )
        {
            helper(newRoot, snakeIndex,snakeIndex+LEVEL_DIM, tail,Dir::S);
        }
        //north
        if(headType != OccupantKey::EndU &&
           snakeIndex - LEVEL_DIM >= 0)
        {
            helper(newRoot, snakeIndex,snakeIndex-LEVEL_DIM, tail,Dir::N);
        }
        
        //east
        if(headType != OccupantKey::EndR &&
           (snakeIndex +1) % LEVEL_DIM != 0)
        {
            helper(newRoot, snakeIndex,snakeIndex+1, tail,Dir::E);
        }
        
        //west
        if(headType != OccupantKey::EndL &&
           snakeIndex % LEVEL_DIM != 0)
        {
            helper(newRoot, snakeIndex,snakeIndex-1, tail,Dir::W);
        }
        
    }
    
    for (const auto& stateref: snakeMoveIndex) {
        if(!stateref.second)
            tryAddToStateQueue(stateref.first.gameState);
     }
    
    /*clean up loops
     for (const auto& state: loops) {
     delete state.gameState;
     }*/
    //cout <<"Snakes revisited: " << snakeVistedCount << endl;
}

void addAllSnakeMovements(GameState * rootState, int snakeIndex)
{
    
    // make a queue of all new states
    //unordered_set<GameStateRef> newStates;
    hash_set<GameStateRef> throughStates;
    queue<pair<GameState*,int>>  statesToExplore;
    GameState* newRoot = new GameState(rootState);
    newRoot->lastMovedIndex = snakeIndex;
    newRoot->numMoves++;
    newRoot->previousState = rootState;
    newRoot->lastMovedDir = Dir::Z;
    
    //get tail
    int tail = newRoot->getSnakeTail(snakeIndex);
    statesToExplore.push(make_pair(newRoot, tail));
    
    auto helper = [&] (GameState* root, int snakeIndex, int indexToCheck, int tail, Dir dir)
    {
        if(root->layout[indexToCheck] == OccupantKey::Empty )
        {
            GameState * moved = new GameState(root);
            moved->previousState = root;
            int newTail = moved->moveSnake(snakeIndex,dir,tail);
            
            GameState * prevState = tryAddToStateQueueOrGetPrev(moved);
            
            if(prevState == NULL)
            {
                statesToExplore.push(make_pair(moved, newTail));
            }
            else
            {
                // in case pulling head through a prev state pulled tail through (or visa-versa)
                if( moved->lastMovedIndex != prevState->lastMovedIndex)
                {
                    if(throughStates.insert(GameStateRef(moved)).second)
                    {
                        statesToExplore.push(make_pair(moved, newTail));
                    }
                    else
                    {
                        delete moved;
                    }
                }
                else
                {
                    delete moved;
                }
            }
        }
        else if(indexToCheck == tail)
        {
            // if looping, don't count as unique state
            GameState * moved = new GameState(root);
            moved->previousState = root;
            int newTail = moved->moveSnake(snakeIndex,dir,tail);
            if(throughStates.insert(GameStateRef(moved)).second)
            {
                statesToExplore.push(make_pair(moved, newTail));
            }
            else
            {
                delete moved;
            }
        }
    };
    
    while(statesToExplore.size()>0)
    {
        newRoot = statesToExplore.front().first;
        int tail =statesToExplore.front().second;
        statesToExplore.pop();
        
        snakeIndex = newRoot->lastMovedIndex;
        OccupantKey headType = newRoot->layout[snakeIndex];
        
        //get tail
        //int tail = newRoot->getSnakeTail(snakeIndex);
        
        //if move onto empty space, new state
        //south
        if(headType != OccupantKey::EndD &&
           snakeIndex + LEVEL_DIM < LEVEL_SIZE )
        {
            helper(newRoot, snakeIndex,snakeIndex+LEVEL_DIM, tail,Dir::S);
        }
        //north
        if(headType != OccupantKey::EndU &&
           snakeIndex - LEVEL_DIM >= 0)
        {
            helper(newRoot, snakeIndex,snakeIndex-LEVEL_DIM, tail,Dir::N);
        }
        
        //east
        if(headType != OccupantKey::EndR &&
           (snakeIndex +1) % LEVEL_DIM != 0)
        {
            helper(newRoot, snakeIndex,snakeIndex+1, tail,Dir::E);
        }
        
        //west
        if(headType != OccupantKey::EndL &&
           snakeIndex % LEVEL_DIM != 0)
        {
            helper(newRoot, snakeIndex,snakeIndex-1, tail,Dir::W);
        }
    }
    
}

int main(int argc, const char * argv[])
{
    
    //setup
    
    addKeyMap('e',OccupantKey::Empty);
    addKeyMap('_',OccupantKey::Empty);
    addKeyMap('>',OccupantKey::EndL);
    addKeyMap('n',OccupantKey::EndD);
    addKeyMap('<',OccupantKey::EndR);
    addKeyMap('V',OccupantKey::EndU);
    
    addKeyMap('|',OccupantKey::Vertical);
    addKeyMap('=',OccupantKey::Horizontal);
    
    addKeyMap('7',OccupantKey::BendRD);
    addKeyMap('J',OccupantKey::BendRU);
    addKeyMap('L',OccupantKey::BendDR);
    addKeyMap('r',OccupantKey::BendUR);
    
    addKeyMap('@',OccupantKey::Player);
    
    populateSnakeHelpers();
    
    //check for level file
    
    
    string levelFileStr;
    
    if(argc == 1)
    {
        cout<<"Please enter a level file location:\n";
        std::cin >>levelFileStr;
    }
    else
    {
        levelFileStr =argv[1];
    }
    
    string levelString = "";
    ifstream levelFile(levelFileStr);
    if (levelFile.is_open())
    {
        string line;
        getline (levelFile,line);
        endIndex = atoi(line.c_str());
        
        while ( getline (levelFile,line) )
        {
            levelString+=line;
        }
        levelFile.close();
    }
    else
    {
        cout << "Unable to open file"<<endl;
        return 0;
    }
    
    
    GameState *startState = stringToGameState(levelString);
    
    
    stateQueue.push(startState);
    stateIndex.insert(GameStateRef(startState));
    
    int moveNum=-1;
    
    
    time_t startTime = time(0);
    
    //keeping a queue of states to explore
    while(stateQueue.size() > 0)
    {
        GameState * originState = stateQueue.front();
        stateQueue.pop();
        
        if(originState->numMoves > moveNum)
        {
            cout<<"evaluating move num "<<originState->numMoves<<" with this many new states "<<stateQueue.size()<<endl;
            moveNum =originState->numMoves;
            
            time_t endTime = time(0);
            
            cout<<"time taken "<<difftime (endTime, startTime)<<endl;
            
        }
        
        
        if(originState->layout[endIndex] == OccupantKey::Player)
        {
            solvedState = originState;
            break;
        }
        
        //try all moves
        
        //first try to move player
        if(originState->lastMovedIndex != originState->playerPos)
            addAllPlayerMovements(originState);

        
        //if found a solution, get the eff outa hea!
        if(solvedState != NULL)
        {
            break;
        }
        
        // then try all snake moves
        for(int i=0; i<LEVEL_SIZE; i++)
        {
            if(originState -> lastMovedIndex == i)
                continue;
            
            OccupantKey occ = originState->layout[i];
            if(occ == OccupantKey::EndD ||
               occ == OccupantKey::EndL ||
               occ == OccupantKey::EndR ||
               occ == OccupantKey::EndU)
            {
                addAllSnakeMovements(originState, i);
            }
        }
     
    }
    
    if(solvedState != NULL)
    {
        
        //reverse the order
        vector<GameState*> reverseList;
        int numMoves = 0;
        
        while(solvedState != NULL)
        {
            switch(solvedState->lastMovedDir)
            {
                case Dir::N:
                    reverseList.push_back(solvedState);
                    break;
                case Dir::S:
                    reverseList.push_back(solvedState);
                    break;
                case Dir::E:
                    reverseList.push_back(solvedState);
                    break;
                case Dir::W:
                    reverseList.push_back(solvedState);
                    break;
                default:
                    break;
            }
            solvedState = solvedState->previousState;
        }
        
        cout<<"rtnval.goldPath = {";
        int currentlyMovingIndx = -1;
        for(int cntr = (int)reverseList.size()-1; cntr>=0; cntr--)
        {
            GameState* thisStep= reverseList[cntr];
            
            
            //figure out if new char or still moving old
            int movedIndex = thisStep->lastMovedIndex;
            switch(thisStep->lastMovedDir)
            {
                case Dir::N:
                    movedIndex += LEVEL_DIM;
                    cout<<"{dir=Direction.Up";
                    break;
                case Dir::S:
                    movedIndex -= LEVEL_DIM;
                    cout<<"{dir=Direction.Down";
                    break;
                case Dir::E:
                    movedIndex -= 1;
                    cout<<"{dir=Direction.Right";
                    break;
                case Dir::W:
                    movedIndex += 1;
                    cout<<"{dir=Direction.Left";
                    break;
                default:
                    break;
            }
            
            if(movedIndex != currentlyMovingIndx)
            {
                //print coords cause new char
                
                numMoves++;
                cout<<", coords=Vector.new("<<movedIndex%LEVEL_DIM +1<<","<<(int)(movedIndex/LEVEL_DIM) +1<<")";
            }
            currentlyMovingIndx =thisStep->lastMovedIndex;
            cout<<"}";
            if(cntr != 0)
            {
                cout<<",";
            }
            
        }
        
        cout<<"};\n";
        cout<<"solved in "<<numMoves<<" moves\n";
    }
    return 0;
}

