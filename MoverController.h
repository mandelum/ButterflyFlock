//
//  MoverController.h
//  ButterflyFlock
//
//  Created by Robin Brandt on 2013-12-28.
//
//

#pragma once
#include "cinder/Perlin.h"
#include <list>
#include "MoverController.h"
#include "Mover.h"
//#include "Predator.h"

// ---DECLARATION---
class MoverController {
public:
    // CONSTRUCTOR
	MoverController();
    // MOVERS, MAYBE WITH POLYMORPHISM, MAYBE SEP. BUTTEFLIES
    std::list<Mover> mMovers; //Array of Movers
	void applyForceToMovers( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float orientStrength );
    void addMovers( int amt );
    void removeMovers( int amt );
    ci::Vec3f getPos(); // FIRST MOVER POSITION GETTER, BUT WHY?
    ci::Vec3f mMoversCentroid; // UNSURE ABOUT THIS
    int mNumMovers;
    
    // PULL TO CENTER
    void pullToCenter( const ci::Vec3f &center );
    
    // PREDATOR FUNCTIONS
    /*
    std::list<Predator> mPredators;
	void addPredators( int amt );
    void applyForceToPredators( float zoneRadius, float lowerThresh, float higherThresh );
    */
    
    // PERLIN
	ci::Perlin mPerlin;
	
    void update( bool flatten );
	void draw();
};