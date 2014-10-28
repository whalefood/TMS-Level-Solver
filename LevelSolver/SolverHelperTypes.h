//
//  SolverHelperTypes.h
//  LevelSolver
//

#ifndef LevelSolver_SolverHelperTypes_h
#define LevelSolver_SolverHelperTypes_h

#include "Settings.h"
#include <unordered_map>
using namespace std;

enum class OccupantKey : uint8_t
{
    Empty =0 ,// = "_",
    
    Player,// = "@"
    
    EndL,// = ">", --  >
    EndD,// = "n", -- n
    EndR,// = "<", --  <
    EndU,// = "V", -- u
    
    Vertical,// = "|", -- ||
    Horizontal,// = "=", -- =
    BendRD,// = "7", -- 7
    BendRU,// = "J", -- J
    BendDR,// = "L", -- L
    BendUR// = "r", -- r
    
};

enum class Dir
{
    N =1,
    E =2,
    S =3,
    W =4,
    Z =0 //no direction
};




namespace std {
    
    template <> struct hash<pair<Dir,Dir>>
    {
        size_t operator()(const pair<Dir,Dir> & x) const
        {
            return (int)x.first*10+(int)x.second;
        }
    };
    
    template <> struct hash<Dir>
    {
        size_t operator()(const Dir & x) const
        {
            return (int)x;
        }
    };
    
    template <> struct hash<OccupantKey>
    {
        size_t operator()(const OccupantKey & x) const
        {
            return (int)x;
        }
    };
    
    template <> struct hash<pair<OccupantKey,Dir>>
    {
        size_t operator()(const pair<OccupantKey,Dir> & x) const
        {
            return (int)x.first*10+(int)x.second;
        }
    };
}

//   big (even)   small(odd)
//   0000 [0] - 0000 [1]
//   0000 [2] - 0000 [3]

uint8_t evenMask = 240;
uint8_t oddMask = 15;

struct OccupantArray
{
    uint8_t data[LEVEL_SIZE_HALF];
    
    OccupantKey operator [](int i)
    {
        int itemNum = i/2;
        if(i%2 == 0)
        {
            //get even
            return (OccupantKey)(data[itemNum] >> 4);
        }
        
        //get odd
        return (OccupantKey)(data[itemNum] & oddMask);
    }
    
    void setIndex(int i, OccupantKey value)
    {
        int itemNum = i/2;
        uint8_t newVal = data[itemNum];
        
        //setting even
        if(i%2 == 0)
        {
            //clear even
            newVal = newVal & oddMask;
            data[itemNum] = newVal | ((uint8_t)value <<4);
        }
        else
        {
            //setting odd
            
            //clear odd
            newVal =newVal & evenMask;
            data[itemNum] = newVal | (uint8_t)value;
        }
    }
    
    void copyFrom(OccupantArray *arr2)
    {
        for(int i=0; i<LEVEL_SIZE_HALF; i++)
        {
            data[i] =arr2->data[i];
        }
    }
    
    
};



unordered_map<Dir,int> dirToDiffMap;
unordered_map<pair<Dir,Dir>, OccupantKey> dirsToOccMap;
unordered_map<Dir,Dir> oppositeDirsMap;


#endif
