#ifndef __AGENT__
#define __AGENT__

#include <cassert>
#include <irrlicht.h>
#include <vector>
#include "boost/utility.hpp" // boost::noncopyable

#include "Sensor.hpp"
#include "Action.hpp"
#include "Coordinates.hpp"

namespace cj
{
	using namespace irr;
	using namespace irr::core;
	using namespace irr::scene;
	using namespace irr::video;
#ifndef SWIG
	using std::vector;
#endif

struct pointOfInterest
{   //Simplified struct for now, class later
    f32 heading;
    f32 range;
    IAnimatedMeshSceneNode* object;
};

	// fwd dec
	namespace actions { class ActAgentTurn; }
	namespace actions { class ActAgentMove; }

	// TODO: Change private to protected once subclass interface has been considered properly.
	class Agent : boost::noncopyable, public sensor::SSensors
	{
	public:
		static const f32 MOVEMENT_SLOW;
		static const f32 MOVEMENT_FAST;
		static const f32 TURN_SLOW;
		static const f32 TURN_FAST;

		typedef vector<pointOfInterest> ContactsList;

		//enum Direction
		//{
			//UP,
			//DOWN,
			//RIGHT_ABS,
			//LEFT_ABS,
			//FORWARD,
			//BACKWARD,
			//RIGHT_REL,
			//LEFT_REL
		//};//


		// STATIC ACCESSORS
		static s32 nextAvailableID ;
		static s32 genID()
		{
			assert( nextAvailableID >= 0 );
			return nextAvailableID ++ ;
		}// genID()

		// TODO: setMaterialFlag( /* handoff */ );
		// TODO: setMD2Animation( /* handoff */ );

		// id=Ctor
		// Main ctor:
		Agent(IrrlichtDevice* d, /*PersistentActionsList& pal,*/ stringw mesh, stringw t, stringw h, const absVec& p);
		// Legacy ctor:
		Agent(IrrlichtDevice* d, /*PersistentActionsList& pal,*/ stringw mesh, stringw t, stringw h, const vector3df& p)
		 : /*persistentActionsList(pal),*/ device(d), path(h), texture(t),
		rangefinderVisible(false), radarVisible(false), activationLevelsVisible(false),
		 feelersOutput(), nearbyAgents(), activationLevels(), actionsList()/*, sensorsAction(*this)*/
		{ AgentCtorImpl(mesh, p); }

		 //: [>persistentActionsList(pal),*/ device(d), path(h), texture(t), feelersOutput(), nearbyAgents(), activationLevels()/*, sensorsAction(*this)<]
		//{
			//AgentCtorImpl();
		//}

		// id=DTOR
		virtual ~Agent();

		// Wrapper for ISceneNode#setPosition(); relative coords.
		void setPosition( const relVec& dest )
		{
			body->setPosition( dest.toIrr_vector3df() );
			//body->updateAbsolutePosition();

			assert( getPosition() == dest );
			assert( getBody().getAbsolutePosition() == dest.to_absVec( *(getBody().getParent()) ) );
		}// setPosition()
		// Wrapper for ISceneNode#setAbsolutePosition().
		void setAbsolutePosition( const absVec& dest )
		{	setPosition(dest); }// setAbsolutePosition()
		// Idem; for convenience.
		void setPosition( const absVec& dest )
		{
			setPosition( dest.to_relVec( *(getBody().getParent()) ) );
			//body->updateAbsolutePosition();

			assert( getAbsolutePosition() == dest );
			assert( getBody().getPosition() == dest.to_relVec( *(getBody().getParent()) ).toIrr_vector3df() );
		}// setPosition()

		//const ActionsList& getActionsList() const
		//{	return actionsList;	}//
		//ActionsList& getActionsList()
		//{	return actionsList;	}//

		relAngle getRotation() const
		{ 	return relAngleVec3d::from_vector3df(getBody().getRotation()).to_relAngle();	}// getRotation()
		void setRotation(const relAngle& rot)
		{ 	getBody().setRotation( rot.to_relAngleVec3d().to_vector3df() ); 	}// setRotation()
		// TODO: with absAngleVector3d:
		//absAngle getAbsoluteRotation() const
		//{ 	return absAngle::from_rotation3df(getBody().getAbsoluteTransformation().getRotation());	}// getAbsoluteRotation()
		//void setAbsoluteRotation(const absAngle& rot) const
		//{ 	setRotation( rot.to_relAngle( getBody().getParent() ) );	}// setAbsoluteRotation()

		// Turn by a specified amount.
		void turnAtomic( const relAngle& theta )
		{	setRotation( getRotation() + theta );	}// turnAtomic()
		bool moveAtomic( const relVec& dest );

		// ACTIONS:
		// Turn by specified number of degrees at specified rate, degrees per second.
		void turn( const relAngle& ang, f32 speed/*FIXME: , IActionLoader* const nextAction = NULL */);
		// id=move
		actions::ActAgentMove* const move( const relVec& dest, f32 speed/* FIXME: , IActionLoader* const nextAction = NULL */);
		// TODO: Small optimization by breaking into two overloaded funcs i/s/o one w/ a default parm:
		// id=seek
		actions::ActAgentMove* const seek( const absVec& dest, f32 speed, f32 turnspeed = 0.0/* FIXME: , IActionLoader* const nextAction = NULL*/ );

		//actions::ActAgentSeekWaypoint[>FIXME: <] *
					     //seekWaypoint( Waypoint& pt, f32 speed, f32 turnspeed = 0.0, IActionLoader* const nextAction = NULL );
					     //// FIXME: {	return

		vector<f32> DrawFeelers(bool debug = false);  //wall collision detection
		// TODO: Even better: make these func templates taking const_iterators.)
		template <typename TAgentsIterator> ContactsList DrawCircle(TAgentsIterator begin, const TAgentsIterator& end, bool debug = false);  //shows circular range and agents in range
		template <typename TAgentsIterator> void ClearCircle(TAgentsIterator begin, const TAgentsIterator& end);
        	template <typename TAgentsIterator> vector<f32> DrawPieSlices(TAgentsIterator begin, const TAgentsIterator& end, bool debug = false);  //activation sectors
		void SetResolution(u32 r);  //change resolution
		void SetRange(u32 r);  //change range
		void SetAwareness(u32 a);  //change awareness

		// ACCESSORS
		const IAnimatedMeshSceneNode& getBody(void) const
		{  return *body;  }
		IAnimatedMeshSceneNode& getBody(void)
		{  return *body;  }

		relVec getPosition() const
		{	return relVec::from_position( getBody() );	}// getPosition()
		absPos getAbsolutePosition() const
		{	return absPos::from_vector3df( getBody().getAbsolutePosition() );	}// getAbsolutePosition()

		//absVec getAbsolutePosition() const
		//{	return absVec::from_position( getBody() );	}// getAbsolutePosition()

		bool getRangefinderVisible() const
		{	return rangefinderVisible;	}//
		bool getRadarVisible() const
		{	return radarVisible;	}//
		bool getActivationLevelsVisible() const
		{	return activationLevelsVisible;	}//

		void setRangefinderVisible( bool vis=true )
		{	rangefinderVisible = vis;	}//
		void setRadarVisible( bool vis=true )
		{	radarVisible = vis;	}//
		void setActivationLevelsVisible( bool vis=true )
		{	activationLevelsVisible = vis;	}//

		// Odd--but these seem to need to be exported explicitly to be visible in e.g. the Actions' runTick() methods.
		using sensor::SSensors::setRangefinder;
		using sensor::SSensors::setRadar;
		using sensor::SSensors::setActivation;
		// Idem, but setting visibility simultaneously:
		virtual void setRangefinder( bool mode, bool vis )
		{
			setRangefinderVisible(vis);
			sensor::SSensors::setRangefinder(mode); // chain up
		}// setRangefinder()
		virtual void setRadar( bool mode, bool vis )
		{
			setRadarVisible(vis);
			sensor::SSensors::setRadar(mode); // chain up
		}// setRadar()
		virtual void setActivation( bool mode, bool vis )
		{
			setActivationLevelsVisible(vis);
			sensor::SSensors::setActivation(mode); // chain up
		}// setActivation()

		// Sensor output:
		const vector<f32>& getRangefinderOutput() const
		{
			assert( getRangefinder() );
			return feelersOutput;
		}//
		const ContactsList& getRadarOutput() const
		{
			assert( getRadar() );
			return nearbyAgents;
		}//
		const vector<f32>& getActivationOutput() const
		{
			assert( getActivation() );
			return activationLevels;
		}//

		// OPERATORS
		virtual bool operator== (const Agent& rhs) const;
		virtual bool operator!= (const Agent& rhs) const;

		// Called by Game; not for end-user.  Unfortunately, two of the sensor types require iteration, and the list will be traversed twice.  Coroutines would solve the problem.
		template <typename TAgentsIterator> void updateSensors(const TAgentsIterator& begin, const TAgentsIterator& end);
		void doTickActions( f32 frameDeltaTime )
		{
			for( cj::actions::ActionsList::iterator it = actionsList.begin(); it != actionsList.end(); ++it )
			{	it->runTick( frameDeltaTime );	}// for
		}// doTickActions

		// (used by ActAgentMove::runTick() for debug-line drawing.  TODO: Perh. make this a property of the Action itself?)
		IVideoDriver& getDriver() {	return *driver;	}// getDriver()
	// id=private
	private:
		IrrlichtDevice *device; //get driver and smgr from this
		IVideoDriver* driver; //for drawing
		ISceneManager* smgr; //needed for adding cubes
		ISceneCollisionManager* cmgr; //for collision detection
		IAnimatedMeshSceneNode* body;  //scene node
		IBillboardSceneNode * circle;  //node for circle
		u32 awareness;  //view range
		u32 resolution;  //density of sensors
		u32 range;  //range of sensors
		stringw path;  //directory of textures
		stringw texture;  //path+file of the body's texture

		// These control whether the debug visuals are used:
		bool rangefinderVisible;
		bool radarVisible;
		bool activationLevelsVisible;

		// These eclectically-named lists hold the output from the sensors, as recorded during the most recent tick.  Use the get*Output() accessors, above, instead.
		vector<f32> feelersOutput;
		ContactsList nearbyAgents;
		vector<f32> activationLevels;

		cj::actions::ActionsList actionsList;

		// Ctor body utility function:
		void AgentCtorImpl(stringw mesh, const vector3df& p);

		// Setters for sensor data.
		void setRangefinderOutput(const vector<f32>& vec ) { feelersOutput = vec;	}//
		void setRadarOutput( const ContactsList& vec ) { nearbyAgents = vec;	}//
		void setActivationOutput( const vector<f32>& vec ) { activationLevels = vec;	}//
	};// Agent

// id=act, id=actions
namespace actions
{
	// id=act-agent-turn
	class ActAgentTurn : public ITickAction
	{
	public:
		// CTOR
		ActAgentTurn( Agent& agt, const relAngle& angle, f32 spd ): agent(agt), totalAngle(angle), speed(spd) {}

		// DTOR
		virtual ~ActAgentTurn() {}

		bool runTick( const f32 frameDeltaTime )
		{
			relAngle angle( frameDeltaTime * speed );

			if( angle > totalAngle )
			{	angle = totalAngle;	}// if

			agent.turnAtomic( angle );
			totalAngle -= angle;
			return totalAngle.iszero(); // indicates completion
			//if( angle.iszero() )
			//{
				//agt.getActionsList().erase(*this);
			//}// if
		}// runTick()
	private:
		Agent& agent;
		relAngle totalAngle;
		const f32 speed;
	};// ActAgentTurn

	// id=act-agent-move
	class ActAgentMove : public ITickAction
	{
	public:
		// CTOR
		ActAgentMove( Agent& agt, const relVec& dist, f32 spd ): agent(agt), totalDist(dist), speed(spd), rangefinderPrevState(agt.getRangefinder())
		{
			// We need the rangefinder on, so, if it isn't, make a memo of the state and then enable it temporarily.
			if( !rangefinderPrevState )
			{	agt.setRangefinder(true, false);	}// if
		}// ctor

		// DTOR
		virtual ~ActAgentMove() {}

		bool runTick( const f32 frameDeltaTime )
		{
			relVec dist( frameDeltaTime * speed );

			// If we'd be going to far, scale back
			if( dist.getLengthSQ() > totalDist.getLengthSQ() )
			{	dist = totalDist;	}// if

			// TODO: Optional:
			agent.getDriver().draw3DLine( agent.getAbsolutePosition().toIrr_vector3df(), dist.to_absVec(agent.getBody()).toIrr_vector3df() );

			// Do we avoid hitting a wall on the way?
			bool success = agent.moveAtomic( dist );
			if( success )
			{	totalDist -= dist;	}// if

			assert( relVec() == relVec(0.0,0.0) );// TODO: delete.

			if ( !success || (totalDist == relVec()) ) // i.e., we ran into a wall and had to stop OR we've completed the movement.
			{
				// If we're done, set the rangefinder back to the way it was.  Note that the *visibility* of the rangefinder isn't touched, here.
				agent.setRangefinder( rangefinderPrevState );
				return true;
			}// if
			else
			{	return false;	}// else
		}// runTick()
	private:
		Agent& agent;
		relVec totalDist;
		const f32 speed;
		bool rangefinderPrevState; // Restored to this state upon termination
	};// ActAgentMove
}// actions

//id=draw-circle
template <typename TAgentsIterator>
Agent::ContactsList Agent::DrawCircle(TAgentsIterator it, const TAgentsIterator& end, bool debug)
{   //draw sensing circle and find detected agents in the list
    vector<pointOfInterest> detected;
    //circle for visual debugging
    if(debug)
    {  circle->setVisible(true);  }
    //draw with a clear material so that lines show up
    SMaterial basic = SMaterial();
    basic.setFlag(EMF_LIGHTING, false);
    driver->setMaterial(basic);
    //draw lines in world coordinate system
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    matrix4 transform = body->getAbsoluteTransformation();

    for( ; it != end; ++it)
    {
        pointOfInterest p;
        p.object = &(it->getBody());
        //do not count yourself
        if(p.object == body)
        {  continue; }

        //figure distance to other agent
        matrix4 other = p.object->getAbsoluteTransformation();
        p.range = transform.getTranslation().getDistanceFrom(other.getTranslation());

        if(p.range <= range)
        {   //if other agent is in range
            if(debug)
            {   //visual debug aids
                //draw a line to the other agent
                line3d<f32> line(transform.getTranslation(), other.getTranslation());
                driver->draw3DLine(line.start, line.end, SColor(0,50,50,136));
                //since we have no light source, this makes the agent black
                p.object->setMaterialFlag(EMF_LIGHTING, true);
            }

            //figure the heading relative to the agent
            vector3df relative = other.getTranslation() - transform.getTranslation();
            //normalize the vector for easy computation
            relative.normalize();
            //The active agent faces along the X axis in her local coordinate
            //system, so create an already normalized vector.
            vector3df aheading(1,0,0);
            transform.rotateVect(aheading);
            //Use the cross product to get the direction of the angle.
            vector3df cp = aheading.crossProduct(relative);
            //cross product angle behaves strange, so use dotproduct for the angle
            //FIX ME: use cross product to avoid extra calculation
            p.heading = aheading.dotProduct(relative);
            p.heading = acos(p.heading) * (180/PI);
            //correct rounding messiness
            if((p.heading * p.heading) <= 0.0000005)
            {  p.heading = 0;  }
            //This only works for our particular setup where the agents are in the XZ
            //plane. If that changes, this will need to change to determine the direction
            //of rotation from the agent's heading to the detected agent's location.
            if(cp.Y > 0.0005) //did not use 0 to take rounding errors into account
            {  p.heading = 360 - p.heading;  }
            //add the other agent to the list of interesting points
	    detected.push_back(p);
        }
        else
        {   //reset the lighting and material if agent is not in range
            p.object->setMaterialFlag(EMF_LIGHTING, false);
            p.object->setMaterialTexture(0, driver->getTexture(texture));
        }
    }

    return detected;
}

template <typename TAgentsIterator>
void Agent::ClearCircle(TAgentsIterator begin, const TAgentsIterator& end)
{   //reset the sensing circle
    circle->setVisible(false);
    for( TAgentsIterator it = begin; it != end; ++it)
    {   //reset the lighting and the texture from DrawCircle and DrawPieSlices debug mode
        it->getBody().setMaterialFlag(EMF_LIGHTING, false);
        it->getBody().setMaterialTexture(0, driver->getTexture(texture));
    }
}

template <typename TAgentsIterator>
vector<f32> Agent::DrawPieSlices(TAgentsIterator begin, const TAgentsIterator& end, bool debug)
{   //determines activation levels of different sectors
    vector<f32> activation(4, 0.0f);
    //find distances and heading to agents
    vector<pointOfInterest> detected = DrawCircle(begin, end);
    //draw with a clear material so that lines show up
    SMaterial basic = SMaterial();
    basic.setFlag(EMF_LIGHTING, false);
    basic.Thickness = 3;
    driver->setMaterial(basic);
    //draw lines in world coordinate system
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    matrix4 transform = body->getAbsoluteTransformation();

    if(debug)
    {  //visual debugging aids
        circle->setVisible(true);
        line3d<f32> line;
        //start at the center of the agent
        line.start = transform.getTranslation();
        //end at the edge of the range
        line.end = vector3df(range,0,0);
        //transform it according to the agent's heading
        transform.transformVect(line.end);
        //rotate endpoint to 45 degrees and draw a line
        line.end.rotateXZBy(45, line.start);
        driver->draw3DLine(line.start, line.end, SColor(0,0,0,0));

        for(u32 i = 0; i < 3; i++)
        {   //draw the other 3 lines 90 degrees apart
            line.end.rotateXZBy(90, line.start);
            driver->draw3DLine(line.start, line.end, SColor(0,0,0,0));
        }
    }

    for(u32 i = 0; i < detected.size(); i++)
    {   //for all detected agents, add range to that agent's sector
        if(detected.at(i).heading <= 45  || detected.at(i).heading > 315)
        {   //extra modifier for something that is in the forward sector
            activation.at(0) += ((range - detected.at(i).range) * 2);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "purple.jpg"));
        }
        else if(detected.at(i).heading <= 135  && detected.at(i).heading > 45)
        {   //sector to the left
            activation.at(1) += (range - detected.at(i).range);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "green.jpg"));
        }
        else if(detected.at(i).heading <= 225  && detected.at(i).heading > 135)
        {   //sector to the rear is less noticable
            activation.at(2) += ((range - detected.at(i).range) / 2);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "yellow.jpg"));
        }
        else if(detected.at(i).heading <= 315  && detected.at(i).heading > 225)
        {   //sector to the right
            activation.at(3) += (range - detected.at(i).range);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "blue.jpg"));
        }
    }

    return activation;
}


template <typename TAgentsIterator>
void Agent::updateSensors(const TAgentsIterator& begin, const TAgentsIterator& end)
{
	if( getRangefinder() )
	{ setRangefinderOutput( DrawFeelers( getRangefinderVisible() ) ); }// if
	if( getRadar() )
	{ setRadarOutput( DrawCircle( begin, end, getRadarVisible() ) ); }// if
	if( getActivation() )
	{ setActivationOutput( DrawPieSlices( begin, end, getActivationLevelsVisible() ) ); }// if
}// updateSensors()


}// cj

#endif// inc
