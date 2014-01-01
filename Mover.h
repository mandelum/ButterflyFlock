//
//  Mover.h
//  ButterflyFlock
//
//  Created by Robin Brandt on 2013-12-28.
//
//  BASE CLASS THAT BUTTEFLIES AND MAYBE PREDATORS AND OTHER FLYING THINGS SHALL INHERIT?
//
#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include <vector>

#include "cinder/gl/gl.h"
//#include "cinder/gl/GlslProg.h"
#include "cinder/ObjLoader.h"
#include "cinder/gl/Vbo.h"
//#include "MyCamUI.h"
#include "cinder/Perlin.h"

using namespace ci;
using namespace ci::app;


class Mover {
public:
    // CONSTRUCTOR
    Mover();
    Mover( ci::Vec3f pos, ci::Vec3f vel, bool followed );
	void pullToCenter( const ci::Vec3f &center );
	void update( bool flatten );
	void draw();
	void drawTail();
	void limitSpeed();
	void addNeighborPos( ci::Vec3f pos );
    
	ci::Vec3f	mPos;
	ci::Vec3f	mTailPos;
	ci::Vec3f	mVel;
	ci::Vec3f	mVelNormal;
	ci::Vec3f	mAcc;
	
	ci::Vec3f	mNeighborPos;
	int			mNumNeighbors;
    
	ci::Color	mColor;
    
	float		mDecay;
	float		mRadius;
	float		mLength;
	float		mMaxSpeed, mMaxSpeedSqrd;
	float		mMinSpeed, mMinSpeedSqrd;
	float		mFear;
	float		mCrowdFactor;
    
	bool		mIsDead;
	bool		mFollowed;
    
    
    void xformGeo();
    
    //params::InterfaceGlRef        mParams;
    
    //gl::GlslProg        mShader;
    TriMesh                        mMesh;
    gl::VboMesh                mVBO;
    //MyCamUI         mMyCam;
    Perlin                        mPerlin;
    
    float                        mBendAmount;
    int                                mVertexNumber;
};
