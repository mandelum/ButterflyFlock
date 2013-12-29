
// ------------------------------
// BUTTERFLY FLOCK SIMULATION
// CENTRALEN, STOCKHOLM, WINTER 2013-2014
// *STORY BY MACHINE*, ADAM WITTSELL, ROBIN BRANDT
// THANKS TO ROBERT HODGIN, DANIEL SHIFFMAN & CRAIG REYNOLDS
// ------------------------------

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"

// INCLUDES FOR OTHER CLASSES
#include "MoverController.h"

// QUICKTIME MOVIE RECORDING INCLUDES
#include "cinder/qtime/MovieWriter.h"
#include "cinder/Utilities.h"

#define NUM_INITIAL_PARTICLES 1000
#define NUM_PARTICLES_TO_SPAWN 15
#define NUM_INITIAL_PREDATORS 1

using namespace ci;
using namespace ci::app;
using namespace std;

// ---DECLARATION---
class ButterflyFlockApp : public AppNative {
  public:
    // SETUP AND INITIALIZSATION FUNCTIONS
    void prepareSettings(Settings *settings);
	void setup();
    
    // UPDATE & DRAW
	void update();
	void draw();
    
	// MOUSE & KEY EVENTS & DATA FOR TEMPORARY INTERACTIVITY
    Vec2i mMouseLoc;
    void mouseDown( MouseEvent event );
    void mouseMove( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void keyUp( KeyEvent event);
    void keyDown( KeyEvent event );
    
    // QUICKTIME RECORDING FUNCTION & OBJECT
    qtime::MovieWriter mMovieWriter;
    void initMovieWriter();
    
    // PARAMS, INTERFACE FOR TWEAKING
	params::InterfaceGlRef	mParams;
	
	// 3D CAMERA
	CameraPersp			mCam;
	Quatf				mSceneRotation; //THIS IS FASCINATING
	Vec3f				mEye, mCenter, mUp;
	float				mCameraDistance;
    
    // MOVER & FLOCK CONTROLLER & VARS
    MoverController mMoverController;
    float				mZoneRadius;
	float				mLowerThresh, mHigherThresh;
	float				mAttractStrength, mRepelStrength, mOrientStrength;
    
    bool				mCentralGravity;
	bool				mFlatten;
    //Could theese not be added to MoverController *also* as mVars instead of arguments to every func call

    
    bool				mSaveFrames;
    
    
};

void ButterflyFlockApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize( 800, 600 );
    settings->setFrameRate( 30.0f );
}

void ButterflyFlockApp::setup()
{
    Rand::randomize(); // RESET RANDOM GENERATOR BASED ON CLOCK
    
    // INITIALIZE VARIABLES, TO BE ADDED TO PARAMS INTERFACE
    mCenter			= Vec3f( getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f );
	mCentralGravity = true;
	mFlatten		= false; // 2D FOR TWEAKING
    
	mZoneRadius		= 80.0f; // ZONE OF PERCEPTION & ATTRACTION / COHESION
	mAttractStrength= 0.004f;
    
    mHigherThresh	= 0.8f;  // ALIGNMENT TRESHOLD
	mOrientStrength	= 0.01f; // CHANGE NAME TO ALIGN?
    
    mLowerThresh	= 0.5f;  // SEPARATE TRESHOLD
    mRepelStrength	= 0.01f;
	
	// SETUP CAMERA
	mCameraDistance = 350.0f;
	mEye			= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter			= Vec3f::zero();
	mUp				= Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 5000.0f );
    
    mSaveFrames		= false;
    
	// SETUP PARAMS
	mParams = params::InterfaceGl::create( "Flocking", Vec2i( 200, 310 ) );
	mParams->addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams->addSeparator();
	mParams->addParam( "Eye Distance", &mCameraDistance, "min=100.0 max=2000.0 step=50.0 keyIncr=s keyDecr=w" );
	mParams->addParam( "Center Gravity", &mCentralGravity, "keyIncr=g" );
	mParams->addParam( "Flatten", &mFlatten, "keyIncr=f" );
	mParams->addSeparator();
	mParams->addParam( "Zone Radius", &mZoneRadius, "min=10.0 max=100.0 step=1.0 keyIncr=z keyDecr=Z" );
	mParams->addParam( "Lower Thresh", &mLowerThresh, "min=0.025 max=1.0 step=0.025 keyIncr=l keyDecr=L" );
	mParams->addParam( "Higher Thresh", &mHigherThresh, "min=0.025 max=1.0 step=0.025 keyIncr=h keyDecr=H" );
	mParams->addSeparator();
	mParams->addParam( "Attract Strength", &mAttractStrength, "min=0.001 max=0.1 step=0.001 keyIncr=a keyDecr=A" );
	mParams->addParam( "Repel Strength", &mRepelStrength, "min=0.001 max=0.1 step=0.001 keyIncr=r keyDecr=R" );
	mParams->addParam( "Orient Strength", &mOrientStrength, "min=0.001 max=0.1 step=0.001 keyIncr=o keyDecr=O" );
	
	// CREATE MOVER CONTROLLER
	mMoverController.addMovers( NUM_INITIAL_PARTICLES );
}

void ButterflyFlockApp::update()
{
    if( mLowerThresh > mHigherThresh ) mHigherThresh = mLowerThresh;
    
	mMoverController.applyForceToPredators( mZoneRadius, 0.4f, 0.7f );
    
	mMoverController.applyForceToMovers( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength );
	if( mCentralGravity ) mMoverController.pullToCenter( mCenter );
    
	mMoverController.update( mFlatten );
	
    // UPDATE CAMERA
	mEye	= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
	gl::rotate( mSceneRotation );
}

void ButterflyFlockApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ), true );
    
    gl::enableDepthRead();
	gl::enableDepthWrite();
    
    // DRAW MOVERS
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) ); // WHITE
	mMoverController.draw();
	
	if( mSaveFrames ){
		writeImage( getHomeDirectory() / "flocking" / ("image_" + toString( getElapsedFrames() ) + ".png"), copyWindowSurface() );
	}
	
	// DRAW PARAMS WINDOW INTERFACE
	mParams->draw();
    
    // DRAW ZONE DIAGRAM
    // Make color intensity based on strength of force in mAttractStrength, mRepelStrength, mOrientStrength
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::setMatricesWindow( getWindowSize() );
	gl::pushModelView();
        gl::translate( Vec3f( 117.0f, getWindowHeight() - 117.0f, 0.0f ) );
        
        gl::color( ColorA( 1.0f, 0.25f, 0.25f, 1.0f ) );
        gl::drawSolidCircle( Vec2f::zero(), mZoneRadius );
        
        gl::color( ColorA( 0.25f, 1.0f, 0.25f, 1.0f ) );
        gl::drawSolidCircle( Vec2f::zero(), mZoneRadius * mHigherThresh );
        
        gl::color( ColorA( 0.25f, 0.25f, 1.0f, 1.0f ) );
        gl::drawSolidCircle( Vec2f::zero(), mZoneRadius * mLowerThresh );
        
        gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.25f ) );
        gl::drawStrokedCircle( Vec2f::zero(), 100.0f );
	gl::popModelView();
    
    // WRITE QUICKTIME MOVIE
    if( mMovieWriter ){
        mMovieWriter.addFrame( copyWindowSurface());
    }
}

void ButterflyFlockApp::mouseMove( MouseEvent event ) {
    
    mMouseLoc = event.getPos();
    
}

void ButterflyFlockApp::mouseDrag( MouseEvent event ) {
    
    mouseMove( event );
    
}

void ButterflyFlockApp::mouseDown( MouseEvent event )
{
    mMoverController.addPredators( NUM_INITIAL_PREDATORS, mMouseLoc );
}

void ButterflyFlockApp::keyUp( KeyEvent event)
{
    if ( event.getChar() == 'm') {
        if (mMovieWriter) {
            mMovieWriter = qtime::MovieWriter();
        }
        else {
            initMovieWriter();
        }
    }
}

void ButterflyFlockApp::keyDown( KeyEvent event )
{
	if( event.getChar() == '1' ){
		mMoverController.addMovers( NUM_PARTICLES_TO_SPAWN );
	} else if( event.getChar() == ' ' ){
		mSaveFrames = !mSaveFrames;
	}
    
}


void ButterflyFlockApp::initMovieWriter()
{
    fs::path path = getSaveFilePath();
    if( path.empty() == false)
        {
        //console() << path;
        qtime::MovieWriter::Format format;
        format.setCodec( qtime::MovieWriter::CODEC_H264);
        //format.setQuality(0.5f);
        format.setDefaultDuration( 1.0f / 30.0f);
        mMovieWriter = qtime::MovieWriter( path, getWindowWidth(),getWindowHeight(), format );
        }
}


CINDER_APP_NATIVE( ButterflyFlockApp, RendererGl )
