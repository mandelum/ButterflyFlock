//
//  MoverController.cpp
//  ButterflyFlock
//
//  Created by Robin Brandt on 2013-12-28.
//
//
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "MoverController.h"

using namespace ci;
using std::list;

MoverController::MoverController()
{
    mPerlin = Perlin( 4 );

}

// POLYMORPHISM with added array of pointers to different arrays of movers, then an extra for loop to go thorugh them, like, butterflies and predators?
void MoverController::applyForceToMovers( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float alignStrength  )
{
	float twoPI = M_PI * 2.0f; // Save value of 2pi for performance
	mMoversCentroid = Vec3f::zero();
	mNumMovers = mMovers.size();
	
    // Nested iteration through every mover, and its interations
	for( list<Mover>::iterator p1 = mMovers.begin(); p1 != mMovers.end(); ++p1 )
    {
        list<Mover>::iterator p2 = p1; // Start where you mean to go on
    
        for( ++p2; p2 != mMovers.end(); ++p2 )// Dont interact with yourself
        {
        
            Vec3f dir = p1->mPos - p2->mPos; // Get direction vector from p1 to p2
            float distSqrd = dir.lengthSquared(); // Save square to be proportional to other squares
            // Vary the Zone of Percetion based on how crowded it feels?
			float zoneRadiusSqrd = zoneRadius * p1->mCrowdFactor * zoneRadius * p2->mCrowdFactor;
            // Check if the neighbor is in the Zone of Perception
			if( distSqrd < zoneRadiusSqrd ){
                // How far inside the Zone are we?
				float per = distSqrd/zoneRadiusSqrd;
				p1->addNeighborPos( p2->mPos ); //Exchange Positions
				p2->addNeighborPos( p1->mPos );
                
				if( per < lowerThresh )         // Separation / Repel
                {
                    // Proportional force calculation, to be better grasped
                    float F = ( lowerThresh/per - 1.0f ) * repelStrength;
					dir.normalize();
					dir *= F;
                    
					p1->mAcc += dir;
					p2->mAcc -= dir;
				}
                else if( per < higherThresh )   // Alignment / Orient
                {
                    // Proportional force calculation, to be better grasped
					float threshDelta	= higherThresh - lowerThresh;
					float adjPer		= ( per - lowerThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * twoPI ) * -0.5f + 0.5f ) ) * alignStrength;
                    
					p1->mAcc += p2->mVelNormal * F;
					p2->mAcc += p1->mVelNormal * F;
					
				}
                else                            // Cohesion (prep) / Attract
                {
                    // Proportional force calculation, to be better grasped
					float threshDelta	= 1.0f - higherThresh;
					float adjPer		= ( per - higherThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * twoPI ) * -0.5f + 0.5f ) ) * attractStrength;
                    
					dir.normalize();
					dir *= F;
                    
					p1->mAcc -= dir;
					p2->mAcc += dir;
				}
			}
		}
		
		mMoversCentroid += p1->mPos;
    
        /* //Hodgin had this commented out, not sure about it yet?
         if( p1->mNumNeighbors > 0 ){ // Cohesion
         Vec3f neighborAveragePos = ( p1->mNeighborPos/(float)p1->mNumNeighbors );
         p1->mAcc += ( neighborAveragePos - p1->mPos ) * attractStrength;
         }
         */
		
		// ADD PERLIN NOISE INFLUENCE - Make separate function
        float scale = 0.002f; // Magic numbers inside code..
		float multi = 0.01f;
		Vec3f perlin = mPerlin.dfBm( p1->mPos * scale ) * multi;
		p1->mAcc += perlin;
		
		
		// CHECK WHETHER THERE IS ANY PARTICLE/PREDATOR INTERACTION
        ///*
    
		float eatDistSqrd = 50.0f;
		float predatorZoneRadiusSqrd = zoneRadius * zoneRadius * 5.0f;
		for( list<Predator>::iterator predator = mPredators.begin(); predator != mPredators.end(); ++predator ) {
            
			Vec3f dir = p1->mPos - predator->mPos[0];
			float distSqrd = dir.lengthSquared();
			
			if( distSqrd < predatorZoneRadiusSqrd ){
				if( distSqrd > eatDistSqrd ){
					float F = ( predatorZoneRadiusSqrd/distSqrd - 1.0f ) * 0.1f;
					p1->mFear += F * 0.1f;
					dir = dir.normalized() * F;
					p1->mAcc += dir;
					if( predator->mIsHungry )
						predator->mAcc += dir * 0.04f * predator->mHunger;
				} else {
					p1->mIsDead = true;
					predator->mHunger = 0.0f;
					predator->mIsHungry = false;
				}
			}
		}
    
		//*/
	}
	mMoversCentroid /= (float)mNumMovers;
}

// PREDATOR FLOCK, SEEMS LIKE THIS SHOULD BE SOLVABLE THROUGH POLYMORPHISM!?! NOT THIS DOUBLING...
void MoverController::applyForceToPredators( float zoneRadius, float lowerThresh, float higherThresh )
{
    float twoPI = M_PI * 2.0f;
	for( list<Predator>::iterator P1 = mPredators.begin(); P1 != mPredators.end(); ++P1 ){
        
		list<Predator>::iterator P2 = P1;
		for( ++P2; P2 != mPredators.end(); ++P2 ) {
			Vec3f dir = P1->mPos[0] - P2->mPos[0];
			float distSqrd = dir.lengthSquared();
			float zoneRadiusSqrd = zoneRadius * zoneRadius * 4.0f;
			
			if( distSqrd < zoneRadiusSqrd ){		// Neighbor is in the zone
				float per = distSqrd/zoneRadiusSqrd;
				if( per < lowerThresh ){			// Separation
					float F = ( lowerThresh/per - 1.0f ) * 0.01f;
					dir.normalize();
					dir *= F;
                    
					P1->mAcc += dir;
					P2->mAcc -= dir;
				} else if( per < higherThresh ){	// Alignment
					float threshDelta	= higherThresh - lowerThresh;
					float adjPer		= ( per - lowerThresh )/threshDelta;
					float F				= ( 1.0f - cos( adjPer * twoPI ) * -0.5f + 0.5f ) * 0.3f;
                    
					P1->mAcc += P2->mVelNormal * F;
					P2->mAcc += P1->mVelNormal * F;
					
				} else {							// Cohesion
					float threshDelta	= 1.0f - higherThresh;
					float adjPer		= ( per - higherThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * twoPI ) * -0.5f + 0.5f ) ) * 0.1f;
                    
					dir.normalize();
					dir *= F;
                    
					P1->mAcc -= dir;
					P2->mAcc += dir;
				}
			}
		}
	}
}
 void MoverController::addPredators( int amt, const Vec2f &pos )
 {
 for( int i=0; i<amt; i++ )
 {
 //Vec3f pos = Rand::randVec3f() * Rand::randFloat( 500.0f, 750.0f );
 Vec3f vel = Rand::randVec3f();
 mPredators.push_back( Predator( Vec3f(pos, 0.0f), vel ) );
 }
 }


void MoverController::pullToCenter( const ci::Vec3f &center )
{
    // MOVER GRAVITY
	for( list<Mover>::iterator p = mMovers.begin(); p != mMovers.end(); ++p ){
		p->pullToCenter( center );
	}
	// PREDATOR GRAVITY
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->pullToCenter( center );
	}
    
}

void MoverController::update( bool flatten )
{
    // MOVER UPDATE
	for( list<Mover>::iterator p = mMovers.begin(); p != mMovers.end(); ){
		if( p->mIsDead ){
			p = mMovers.erase( p );
		} else {
			p->update( flatten );
			++p;
		}
	}
    
    
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->update( flatten );
	}
    
}

void MoverController::draw()
{
	// DRAW PREDATOR ARROWS

	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		float hungerColor = 1.0f - p->mHunger;
		gl::color( ColorA( 1.0f, hungerColor, hungerColor, 1.0f ) );
		p->draw();
	} 
	
	// DRAW MOVER ((ARROWS))
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	//glBegin( GL_LINES );
	for( list<Mover>::iterator p = mMovers.begin(); p != mMovers.end(); ++p ){
		p->draw();
	}
	//glEnd();
}



void MoverController::addMovers( int amt )
{
	for( int i=0; i<amt; i++ )
        {
		Vec3f pos = Rand::randVec3f() * Rand::randFloat( 100.0f, 200.0f );
		Vec3f vel = Rand::randVec3f();
		
		bool followed = false;
		if( mMovers.size() == 0 ) followed = true; //Eaten?
		
		mMovers.push_back( Mover( pos, vel, followed ) );
        }
}

void MoverController::removeMovers( int amt )
{
	for( int i=0; i<amt; i++ )
        {
		mMovers.pop_back();
        }
}

Vec3f MoverController::getPos()
{
	return mMovers.begin()->mPos;
}
