//
//  Mover.cpp
//  ButterflyFlock
//
//  Created by Robin Brandt on 2013-12-28.
//
//
#include "cinder/app/AppNative.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

#include "Mover.h"

using namespace ci;
using std::list;

// Make mover more general, and move stuff to butterfly and other objects...
// Use Virtual functions, but est performance
// maybe try templates
// Abstrct class

// CONSTRUCTORS
Mover::Mover()
{
    
}

Mover::Mover( Vec3f pos, Vec3f vel, bool followed )
{ // INITIALIZATION
    mPos			= pos;
	mTailPos		= pos;
	mVel			= vel;
	mVelNormal		= Vec3f::yAxis();
	mAcc			= Vec3f::zero();
	
	mNeighborPos	= Vec3f::zero();
	mNumNeighbors	= 0;
	mMaxSpeed		= Rand::randFloat( 2.5f, 4.0f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;
	
	mColor			= ColorA( 1.0f, 1.0f, 1.0f, 1.0f );
    
	mDecay			= 0.99f;
	mRadius			= 1.0f;
	mLength			= 5.0f;
	mFear			= 1.0f; // Other mental states to follow in butterfly
	mCrowdFactor	= 1.0f; // Affects Zone of Perception & max speed
	
	mIsDead			= false;
	mFollowed		= followed;
}

void Mover::pullToCenter( const Vec3f &center )
{
	Vec3f dirToCenter = mPos - center;
	float distToCenter = dirToCenter.length();
	float distThresh = 200.0f;
	
	if( distToCenter > distThresh ){ // Pull to a center until the given threshold
		dirToCenter.normalize();
		float pullStrength = 0.00025f;
		mVel -= dirToCenter * ( ( distToCenter - distThresh ) * pullStrength );
	}
}

void Mover::update( bool flatten )
{
    // Calculate the Crowd Factor in some way proportional to the Number of
	mCrowdFactor -= ( mCrowdFactor - ( 1.0f - mNumNeighbors * 0.02f ) ) * 0.1f;
	mCrowdFactor = constrain( mCrowdFactor, 0.5f, 1.0f );
	
	mFear -= ( mFear - 0.0f ) * 0.2f; // Diminishing fear
	
	if( flatten ) mAcc.z = 0.0f;
	
	mVel += mAcc;
    
	mVelNormal = mVel.normalized();
	
	limitSpeed();
	
	mPos += mVel;
    
	if( flatten ) mPos.z = 0.0f;
	
	mTailPos = mPos - ( mVelNormal * mLength );
	mVel *= mDecay;
	
    // Calculates color based on density
	float c = math<float>::min( mNumNeighbors/50.0f, 1.0f );
	mColor = ColorA( CM_HSV, 1.0f - c, c, c * 0.5f + 0.5f, 1.0f );
	
    // Resets
	mAcc = Vec3f::zero();
	mNeighborPos = Vec3f::zero();
	mNumNeighbors = 0;
}

void Mover::limitSpeed()
{
	float maxSpeed = mMaxSpeed + mCrowdFactor;
	float maxSpeedSqrd = maxSpeed * maxSpeed;
	
	float vLengthSqrd = mVel.lengthSquared();
	if( vLengthSqrd > maxSpeedSqrd ){
		mVel = mVelNormal * maxSpeed;
		
	} else if( vLengthSqrd < mMinSpeedSqrd ){
		mVel = mVelNormal * mMinSpeed;
	}
	mVel *= (1.0 + mFear ); // Running faster when scared
}

void Mover::draw()
{
	glColor4f( mColor );
    // Draws an arrow
	gl::drawVector( mPos - mVelNormal * mLength, mPos - mVelNormal * mLength * 0.75f, mLength * 0.7f, mRadius );
}

void Mover::drawTail()
{
	glColor4f( mColor );
	gl::vertex( mPos );
	glColor4f( ColorA( mColor.r, mColor.g, mColor.b, 0.01f ) );
	gl::vertex( mTailPos );
}

void Mover::addNeighborPos( Vec3f pos )
{   // A bit usure why this method is needed?
	mNeighborPos += pos;
	mNumNeighbors ++;
}
