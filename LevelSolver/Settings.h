//
//  Settings.h
//  LevelSolver
//
//  Created by Jonah Wallerstein on 7/10/14.
//  Copyright (c) 2014 Whale Food Software. All rights reserved.
//

#ifndef LevelSolver_Settings_h
#define LevelSolver_Settings_h

//#define SIXBYSIX

#ifdef SIXBYSIX

#define LEVEL_DIM 6 // the level dimentions
#define LEVEL_SIZE 36  // level dimentions ^ 2
#define LEVEL_SIZE_HALF  18 // half size, round up to even

#else

#define LEVEL_DIM 5 // the level dimentions
#define LEVEL_SIZE 25  // level dimentions ^ 2
#define LEVEL_SIZE_HALF  13 // half size, round up to even


#endif

#define hash_table unordered_map
#define hash_set unordered_set


typedef uint8_t sint;


#endif
