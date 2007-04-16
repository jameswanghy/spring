#include "StdAfx.h"
// ExplosiveProjectile.cpp: implementation of the CExplosiveProjectile class.
//
//////////////////////////////////////////////////////////////////////

#include "ExplosiveProjectile.h"
#include "Game/GameHelper.h"
#include "Map/Ground.h"
#include "Game/Camera.h"
#include "Rendering/GL/VertexArray.h"
#include "Rendering/Textures/ColorMap.h"
#include "Sim/Weapons/WeaponDefHandler.h"
#include "Sim/Misc/InterceptHandler.h"
#include "ProjectileHandler.h"
#include "mmgr.h"

CR_BIND_DERIVED(CExplosiveProjectile, CWeaponProjectile, (float3(0,0,0),float3(0,0,0),NULL,NULL,0,0));

CR_REG_METADATA(CExplosiveProjectile, (
	CR_MEMBER(ttl),
	CR_MEMBER(areaOfEffect)
	));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExplosiveProjectile::CExplosiveProjectile(const float3& pos,const float3& speed,CUnit* owner, WeaponDef *weaponDef, int ttl,float areaOfEffect)
: CWeaponProjectile(pos,speed,owner, 0,ZeroVector,weaponDef,0, true),
	ttl(ttl),
	areaOfEffect(areaOfEffect),
	curTime(0)
{
	useAirLos=true;

	SetRadius(weaponDef->collisionSize);
	drawRadius=weaponDef->size;
	invttl=1.0f/ttl;

#ifdef TRACE_SYNC
	tracefile << "New explosive: ";
	tracefile << pos.x << " " << pos.y << " " << pos.z << " " << speed.x << " " << speed.y << " " << speed.z << "\n";
#endif
}

CExplosiveProjectile::~CExplosiveProjectile()
{

}

void CExplosiveProjectile::Update()
{
	pos+=speed;
	speed.y+=gs->gravity;

	if(!--ttl)
		Collision();

	if(weaponDef->noExplode) {
		if(TraveledRange())
			CProjectile::Collision();
	}
	curTime+=invttl;
	if(curTime>1)
		curTime=1;
}

void CExplosiveProjectile::Collision()
{
	if (!weaponDef->noExplode) {
		float h=ground->GetHeight2(pos.x,pos.z);
		if(h>pos.y){
			float3 n=ground->GetNormal(pos.x,pos.z);
			pos-=speed*max(0.0f,min(1.0f,float((h-pos.y)*n.y/n.dot(speed)+0.1f)));
		}
	}
//	helper->Explosion(pos,damages,areaOfEffect,owner);
	CWeaponProjectile::Collision();
}

void CExplosiveProjectile::Collision(CUnit *unit)
{
//	unit->DoDamage(damages,owner);
//	helper->Explosion(pos,damages,areaOfEffect,owner);

	CWeaponProjectile::Collision(unit);
}

void CExplosiveProjectile::Draw(void)
{
	if(s3domodel)	//dont draw if a 3d model has been defined for us
		return;

	inArray=true;
	unsigned char col[4];
	if(weaponDef->visuals.colorMap) {
		weaponDef->visuals.colorMap->GetColor(col, curTime);
	} else {
		col[0]=int(weaponDef->visuals.color.x*255);
		col[1]=int(weaponDef->visuals.color.y*255);
		col[2]=int(weaponDef->visuals.color.z*255);
		col[3]=int(weaponDef->intensity*255);
	}

	float3 dir=speed;
	dir.Normalize();

	for(int a=0;a<5;++a){
		col[0]=int((5-a)*0.2f*col[0]);
		col[1]=int((5-a)*0.2f*col[1]);
		col[2]=int((5-a)*0.2f*col[2]);
		col[3]=int((5-a)*0.2f*col[3]);
		float3 interPos=pos+speed*gu->timeOffset-dir*drawRadius*0.6f*a;
		va->AddVertexTC(interPos-camera->right*drawRadius-camera->up*drawRadius,weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->ystart,col);
		va->AddVertexTC(interPos+camera->right*drawRadius-camera->up*drawRadius,weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->ystart,col);
		va->AddVertexTC(interPos+camera->right*drawRadius+camera->up*drawRadius,weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->yend,col);
		va->AddVertexTC(interPos-camera->right*drawRadius+camera->up*drawRadius,weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->yend,col);
	}
}

int CExplosiveProjectile::ShieldRepulse(CPlasmaRepulser* shield,float3 shieldPos, float shieldForce, float shieldMaxSpeed)
{
	float3 dir=pos-shieldPos;
	dir.Normalize();
	if(dir.dot(speed)<shieldMaxSpeed){
		speed+=dir*shieldForce;
		return 2;
	}
	return 0;
}
