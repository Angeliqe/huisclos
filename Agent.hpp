#ifndef __AGENT__
#define __AGENT__

#include "assert_swig.hpp"
#include <irrlicht.h>
#include <vector>
#include <math.h>
#include "boost/utility.hpp" // boost::noncopyable

#include "Wall.hpp"
//#include "State.hpp"
#include "Sensor.hpp"
#include "Action.hpp"
#include "Coordinates.hpp"

namespace cj
{
using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;
using std::vector;

struct pointOfInterest
{   //Simplified struct for now, class later
    f32 heading;
    f32 range;
    IAnimatedMeshSceneNode* object;
};

// fwd dec
class Agent;
namespace actions
{
	class MoveAction;
	class FollowPathAction;
	class DieAction;
	class AttackAction;
	//class ActAgentTurn;
}// fwd

class IState : public boost::noncopyable
{
public:
	IState();
	virtual ~IState();
	//virtual void start() = 0;
	virtual void start();
	virtual void stop();
	virtual void runTick( f32 deltaTime ) = 0;

	bool started() const;
private:
	bool hasStarted;
};// State

//class State
//{
//};// State

// id=manual-state
class ManualState : public IState
{
public:
	ManualState( Agent& ag ): agent(ag) {}
	virtual ~ManualState() {}
	virtual void start();
	virtual void stop();
	void runTick( f32 frameDeltaTime );
private:
	Agent& agent;
};// ManualState


// id=wander
class WanderState : public IState
{
public:
	WanderState( Agent& ag ): agent(ag), wander(NULL) {}
	virtual ~WanderState() {}
	virtual void resetWander();
	virtual void start();
	virtual void stop();
	void runTick( f32 deltaTime );
private:
	Agent& agent;
	cj::actions::FollowPathAction* wander;
};// WanderState

//  id=death, id=dead
class DeathState : public IState
{
public:
	DeathState(Agent& ag);
	virtual ~DeathState() {}
	virtual void start();
	void runTick( f32 deltaTime );
private:
	virtual void stop() {}// shouldn't be called

	Agent& agent;
	actions::DieAction* playdead;
};// DeathState

// id=fight
class FightState : public IState
{
public:
	FightState(Agent& ag): agent(ag), attack(NULL) {}
	virtual ~FightState() {}
	virtual void start();
	virtual void resetAttack();
	virtual void stop();
	virtual void runTick( f32 frameDeltaTime );
private:
	Agent& agent;
	actions::AttackAction* attack;
};// Fight

// TODO: Change private to protected once subclass interface has been considered properly.
// id=agent
class Agent : public boost::noncopyable, public sensor::SSensors, public Timed
{
public:
	static const f32 DEFAULT_GIRTH;
	static const f32 MOVEMENT_SLOW;
	static const f32 MOVEMENT_FAST;
	static const f32 TURN_SLOW;
	static const f32 TURN_FAST;

	typedef vector<pointOfInterest> ContactsList;
	typedef IState* AgentState;
	//enum AgentState { MANUAL, DEAD, ATTACK, MOVE };

	ManualState* const Manual;
	WanderState* const Wander;
	DeathState* const Dead;
	FightState* const Fight;

	// TODO: Polymorphism would be superior.  Would mean changing addAgent() yet again.
	enum MOB { FAIRY };// MOB


	typedef vector<Agent*> (* agentsListCallback)();
	//static agentsListCallback getList();
	static agentsListCallback getAgentsList;

	static s32 nextAvailableID;
	static s32 genID(); // Returns the next unused Agent ID int.
	s32 getID() const;
	static bool getLineOfSightExists( const Agent& a1, const Agent& a2 );// true if l-o-s exists between the named Agents.

	// id=Ctor
	Agent(IrrlichtDevice* d, stringw mesh, stringw texture, stringw path, const vector3df& position, const s32 HP, const s32 Str, const s32 Spd, const s32 Acc);
	// id=DTOR
	virtual ~Agent();

	// Wrapper for ISceneNode#setPosition(); relative coords.
	void setPosition( const relVec& dest );
	// Wrapper for ISceneNode#setAbsolutePosition().
	void setAbsolutePosition( const absVec& dest );
	void setPosition( const absVec& dest );

	relAngle getRotation() const;
	void setRotation(const relAngle& rot);
	// TODO: with absAngleVector3d:
	//absAngle getAbsoluteRotation() const
	//{ 	return absAngle::from_rotation3df(getBody().getAbsoluteTransformation().getRotation());	}// getAbsoluteRotation()
	//void setAbsoluteRotation(const absAngle& rot) const
	//{ 	setRotation( rot.to_relAngle( getBody().getParent() ) );	}// setAbsoluteRotation()

	// Turn by a specified amount.
	void turnAtomic( const relAngle& theta );
	bool moveAtomic( const relVec& dest );

	// id=ACTIONS:
	// Also look in protected: section.
	//actions::ActAgentSeekPosition* const Goto( const vector3df& dest, f32 speed );// Go straight to the destination.
	actions::MoveAction* const MoveTo( const vector3df dest, f32 speed );// Go straight to the destination.
	template <typename TWaypointsList> actions::FollowPathAction* visitWaypoints( const TWaypointsList& pointsList, f32 speed );
	// id=seek
	actions::FollowPathAction* const Seek( const vector3df dest, /*const cj::Wall& w, */ f32 speed=0.00, bool debug=true );// Pathfind to the destination with A*.  Leaving speed=0 will cause that Agent's default speed to be used.
	void clearAllActions();
	void runTick( f32 frameDeltaTime );// Used by Game.

	vector<f32> DrawFeelers(bool debug = false);  //wall collision detection
	// TODO: Even better: make these func templates taking const_iterators.)
	template <typename TAgentsIterator> ContactsList DrawCircle(TAgentsIterator begin, const TAgentsIterator& end, bool debug = false);  //shows circular range and agents in range
	template <typename TAgentsIterator> void ClearCircle(TAgentsIterator begin, const TAgentsIterator& end);
	template <typename TAgentsIterator> vector<f32> DrawPieSlices(TAgentsIterator begin, const TAgentsIterator& end, bool debug = false);  //activation sectors
	void SetResolution(u32 r);  //change resolution
	void SetRange(u32 r);  //change range
	void SetAwareness(u32 a);  //change awareness

	// ACCESSORS
	const IAnimatedMeshSceneNode& getBody(void) const;
	IAnimatedMeshSceneNode& getBody(void);
	relVec getPosition() const;
	absPos getAbsolutePosition() const;

	bool getRangefinderVisible() const {	return rangefinderVisible;	}//
	bool getRadarVisible() const {	return radarVisible;	}//
	bool getActivationLevelsVisible() const {	return activationLevelsVisible;	}//

	void setRangefinderVisible( bool vis=true ) {	rangefinderVisible = vis;	}//
	void setRadarVisible( bool vis=true ) {	radarVisible = vis;	}//
	void setActivationLevelsVisible( bool vis=true ) {	activationLevelsVisible = vis;	}//

	AgentState getState() const;
	void setState( AgentState s );
	bool isDead() const;// predicate

	using sensor::SSensors::setRangefinder;
	using sensor::SSensors::setRadar;
	using sensor::SSensors::setActivation;
	// Idem, but setting visibility simultaneously:
	virtual void setRangefinder( bool mode, bool vis );
	virtual void setRadar( bool mode, bool vis );
	virtual void setActivation( bool mode, bool vis );

	// Sensor output:
	const vector<f32>& getRangefinderOutput() const { assert( getRangefinder() ); return feelersOutput; }//
	const ContactsList& getRadarOutput() const { assert( getRadar() ); return nearbyAgents; }//
	const vector<f32>& getActivationOutput() const { assert( getActivation() ); return activationLevels; }//

	// id=op
	virtual bool operator== (const Agent& rhs) const;
	virtual bool operator!= (const Agent& rhs) const;

	// Called by Game; not for end-user.  Unfortunately, two of the sensor types require iteration, and the list will be traversed twice.  Coroutines would solve the problem.
	template <typename TAgentsIterator> void updateSensors(const TAgentsIterator& begin, const TAgentsIterator& end);

	//bool getHasMoveTarget() const;
	//void setHasMoveTarget( const bool b );
	//const Agent* getAttackTarget() const;
	//Agent* getAttackTarget();
	//void setAttackTarget( Agent* const targ );
	vector<Agent*> getVisibleAgents( bool countIfDead=false ) const; // Returns list of all Agents visible to the caller.
	//template <typename TAgentsList> vector<Agent*> getVisibleAgents( TAgentsList&, bool countIfDead=false ); // Returns list of all Agents visible to the caller.
	bool isEnemyVisible( bool countIfDead=false ) const;
	//template <typename TAgentsList> bool isEnemyVisible( TAgentsList&, bool countIfDead=false ) const;
	//template <typename TAgentsList> vector<Agent*> getVisibleAgents( typename TAgentsList::iterator it, const typename TAgentsList::iterator end ); // Returns list of all Agents visible to the caller.
	//bool isEnemyVisible(); // True if there is a line-of-sight to any other agent.

	// (used by ActAgentMove::runTick() for debug-line drawing.  TODO: Perh. make this a property of the Action itself?)
	IVideoDriver& getDriver() {	return *driver;	}// getDriver()

	virtual u32 getAttackMultiplier() const {	return 1;	}
	virtual s32 getHP() const { return HitPoints;	}//
	virtual void setHP( const s32 hp ) {	HitPoints = hp;	}//
	virtual s32 getStr() const { return Strength;	}//
	virtual void setStr( const s32 str ) {	Strength = str;	}//
	virtual s32 getSpd() const { return Speed;	}//
	virtual void setSpd( const s32 spd ) {	Speed = spd;	}//
	virtual s32 getAcc() const { return Accuracy;	}//
	virtual void setAcc( const s32 acc ) {	Accuracy = acc;	}//

	virtual void Die();// Induces Die action.  Surprise.  Unlike Attack() and Seek(), this method is compatible ⁅I think!⁆ with explicitly calling Agent.setState(Agent.Dead).
	virtual void TakeDamage( const s32 damage );
	actions::AttackAction* const Attack( Agent& target );// Initiate attack; will continue until target is dead or interrupted by the calling State.
	actions::AttackAction* const Attack();// Just shoot straight ahead

	//actions::ActAtkMelee* AttackMelee( Agent& target );
	//ActAtkRanged* AttackRanged( Agent& target );


	// Animation adjustment:
	virtual void animationStand();
	virtual void animationAttack();
	virtual void animationRun();
	virtual void animationDie();

	//virtual void onStateBegin();
	//virtual void onStateEnd();
	//virtual bool onTick( u32 frameDeltaTime_ms );

	const actions::IAction* getCurrentAction() const;
	actions::IAction* getCurrentAction();
	void clearCurrentAction();
	void setCurrentAction( actions::IAction* const newact );

	//Wall& wall; // ⁅Dammit, I don't want to have to store this⁆

	// <TAG> CA - NOTE: Do not add public functions to Agent beyond this line.
	// This will be my section.
	bool MoveVector(vector3df distance);  //COLLISON MOVEMENT

	vector<IBillboardSceneNode*> circles;

// id=private, id=priv
private:

	//agentsListCallback getAgentsList;// TODO: Accessors.

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

	IState* currentState;
	actions::IAction* currentAction;
	actions::ActionsList actionsList;

	Agent* attackTarget;
	bool hasMoveTarget;

	s32 HitPoints;
	s32 Strength;
	s32 Speed;
	s32 Accuracy;

	// Controlling the rate at which AI states are updated:
	//u32 period_ms;
	//u32 tickDeltaTime_ms;
	f32 period;
	f32 tickDeltaTime;

	// Ctor body utility function:
	void AgentCtorImpl(stringw mesh, const vector3df& p);

	// Setters for sensor data.
	void setRangefinderOutput(const vector<f32>& vec ) { feelersOutput = vec;	}//
	void setRadarOutput( const ContactsList& vec ) { nearbyAgents = vec;	}//
	void setActivationOutput( const vector<f32>& vec ) { activationLevels = vec;	}//
};// Agent
}// cj

#include "Agent_templates.hpp"

#endif// inc

