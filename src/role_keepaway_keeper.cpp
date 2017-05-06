// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "role_keepaway_keeper.h"

#include "strategy.h"

#include "bhv_chain_action.h"

#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/body_turn_to_ball.h>
#include <rcsc/action/body_advance_ball.h>
#include <rcsc/action/body_dribble.h>
#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_pass.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include <rcsc/action/body_clear_ball.h>

#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_scan_field.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

using namespace rcsc;

const std::string RoleKeepawayKeeper::NAME( "KeepawayKeeper" );

/*-------------------------------------------------------------------*/
/*!

 */
namespace {
rcss::RegHolder role = SoccerRole::creators().autoReg( &RoleKeepawayKeeper::create,
                                                       RoleKeepawayKeeper::NAME );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
RoleKeepawayKeeper::execute( PlayerAgent * agent )
{

    bool kickable = agent->world().self().isKickable();
    if ( agent->world().existKickableTeammate()
         && agent->world().teammatesFromBall().front()->distFromBall()
         < agent->world().ball().distFromSelf() )
    {
        kickable = false;
    }

    if ( kickable )
    {
        doKick( agent );
    }
    else
    {
        doMove( agent );
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
void
RoleKeepawayKeeper::doKick( PlayerAgent * agent )
{
//     if ( Bhv_ChainAction().execute( agent ) )
//     {
//         dlog.addText( Logger::TEAM,
//                       __FILE__": (execute) do chain action" );
//         agent->debugClient().addMessage( "ChainAction" );
//     }
// if the take get close I pass 
  //if I can get the ball ,I chase ball
    const WorldModel & wm = agent->world();
    bool can_pass=false;
    bool can_drrible=false;
    bool hold_ball=false;
    //---------------------------------------------------------
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    
    
    const Vector2D M_pos = wm.self().pos();
    
    
    const PlayerPtrCont & opps = wm.opponentsFromSelf(); //get the sorted player from self
    const PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< PlayerObject * >( 0 )
            : opps.front() );
    
	
    const PlayerPtrCont & teams =wm.teammatesFromSelf();
    const PlayerObject * nearest_team
       =(
	 teams.empty()
	 ? static_cast<PlayerObject *>(0)
	  :teams.front()
      );
    
	
    Vector2D pass_target = nearest_team->pos();       
    if(nearest_opp!=NULL)
    {
      if(nearest_opp->pos().dist(M_pos)<5)
      {
	if(Body_Pass::get_best_pass(wm,&pass_target,NULL,NULL))
	{
	  agent->doPointto(pass_target.x,pass_target.y);
	  Body_Pass().execute(agent);
	  agent->setNeckAction(new Neck_TurnToLowConfTeammate());
	}
      }

	
    }
    
    
    
    
    
    
    
    
    
    
    
  
  
  
}

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleKeepawayKeeper::doMove( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();

    if ( ! wm.existKickableTeammate()
         && self_min <= mate_min + 1 )
    {
        Vector2D face_point( 0.0, 0.0 );
        Body_Intercept( true, face_point ).execute( agent );
        agent->setNeckAction( new Neck_TurnToBallOrScan() );
        return;
    }

    const Vector2D home_pos = Strategy::i().getPosition( wm.self().unum() );


    if ( ! Body_GoToPoint( home_pos,
                           0.5,
                           ServerParam::i().maxDashPower() ).execute( agent ) )
    {
        Body_TurnToBall().execute( agent );
    }

    agent->setNeckAction( new Neck_TurnToBallOrScan() );
}
