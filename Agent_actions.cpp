#include "Agent.hpp"

namespace cj
{
// id=action
namespace actions
{

MoveAction::MoveAction( Agent& agt, const vector3df dest, f32 spd ): agent(agt), destination(dest), speed(spd)
{
dpr("MoveAction ctor");
}
MoveAction::~MoveAction()
{
	//assert( started() );
}// d

void MoveAction::start()
{
dpr("Agent " << agent.getID() << " began MoveAction.");
	assert( !started() );
	agent.animationRun();
	agent.getBody().setRotation( (destination - agent.getBody().getAbsolutePosition()).getHorizontalAngle() + vector3df(0,-90.00,0) );
	agent.getBody().updateAbsolutePosition();
	assert(!started());
	actions::IAction::start();// chain up
}// start()

bool MoveAction::runTick( const f32 deltaTime )
{
	assert( started() );

//dpr("ActAgentMove tick to " << destination);
	vector3df remaining = destination - agent.getBody().getAbsolutePosition();
	f32 dist = deltaTime * speed;
	if( dist >= remaining.getLength() )
	{ 	dist = remaining.getLength(); 	}// if

	vector3df distVec = (destination - agent.getBody().getPosition());
	distVec.normalize() *= dist;
//dpr("dist " << dist );

	// TODO: Optional:
	//agent.getDriver().draw3DLine( agent.getPosition().toIrr_vector3df(), dist.to_absVec(*agent.getBody().getParent()).toIrr_vector3df() );

	agent.getBody().setPosition( agent.getBody().getPosition() + distVec );
	agent.getBody().updateAbsolutePosition();

	if( iszero(remaining.getLength()) )
	{
dpr("Arrived at " << destination);
		//waypoint.setFancyGraphic(false);
		return true;
	}// if
	else
	{	return false;	}// else
}// runTick()

// id=die
void DieAction::start()
{
//dpr("Agent " << agent.getID() << " began to Die.");
	assert(!started());
	agent.animationDie();
	assert(!started());
	IAction::start();
}// start()
bool DieAction::runTick( f32 frameDeltaTime )
{
	return true;// one-shot
}// runTick()

// id=ATTACK
AttackAction::AttackAction( Agent& atk, Agent& targ )
: Timed( f32(atk.getAttackMultiplier()) ), attacker(atk), target(targ)
{}// ctor

AttackAction::~AttackAction()
{}

void AttackAction::start()
{
	assert( !started()	);
dpr( "Agent " << attacker.getID() << " attacks Agent " << target.getID() );
	attacker.animationStand();//TODO: attacker.animationAttack();
	// TODO: Set facing target
	const bool hit = true;//TODO: calc hit chance based on ACC & dist
	if( hit )
	{
		const s32 damage = 1;//TODO: calculate damage based on random val & STR
		target.TakeDamage(damage);
	}// if
	assert(!started());
	IAction::start();// chain up
}// start()

bool AttackAction::runTick( const f32 deltaTime )
{
	assert( started() );
	if( periodMet(deltaTime) )
	{
		//...
		return true; // TODO: Actual retval should be true only when the entire attack sequence is complete.
	}// if
	// else
	return false;
}// runTick()

}// actions
}// cj

