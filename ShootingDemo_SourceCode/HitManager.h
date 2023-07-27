#pragma once
#include <DirectXMath.h>
#include <list>
#include "GameObject.h"

using namespace DirectX;
using namespace std;

class GameComponent;

enum class HIT_AREA_TYPE	
{
	HIT_PLAYER_BODY = 0x00000001UL,		
	HIT_PLAYER_ATTACK = 0x00000002UL,		
	HIT_PLAYER_SHIELD = 0x00000004UL,		
	HIT_ENEMY_BODY = 0x00000010UL,		
	HIT_ENEMY_ATTACK = 0x00000020UL,		
	HIT_ENEMY_SHIELD = 0x00000040UL,		
};

class HitBoxClass
{
private:
	HIT_AREA_TYPE	m_hitType;					
	int				m_hitPower;					
	XMFLOAT2		m_hitCenter;				
	XMFLOAT2		m_hitSize;					
	XMFLOAT4		m_hitRect;					

	void updateHitRect();						 

public:
	void setHitSize(float width, float height)	
	{
		m_hitSize.x = width;
		m_hitSize.y = height;
		updateHitRect();						
	}

	void setPosition(float x, float y)			
	{
		m_hitCenter.x = x;
		m_hitCenter.y = y;
		updateHitRect();						
	}

	void setAttackType(HIT_AREA_TYPE area, int pow)	
	{
		m_hitType = area;
		m_hitPower = pow;
	}

	HIT_AREA_TYPE getHitType()				
	{
		return m_hitType;
	}

	int getHitPower()						
	{
		return m_hitPower;
	}

	XMFLOAT4 getHitRect()					
	{
		return m_hitRect;
	}
};

class HitCircleClass
{
private:
	float radius;							
	XMFLOAT2 center;						
	float doubledR;							

public:
	void setCenter(float x, float y)		
	{
		center.x = x;
		center.y = y;
	}

	void setRadius(float r)					
	{
		radius = r;
		doubledR = r * r;					
	}

	float getRadius()						　
	{
		return radius;
	}

	float getDoubledRadius()				
	{
		return doubledR;
	}

	XMFLOAT2 getCenterPosition()
	{
		return center;
	}
};

class HitManager
{
private:
	class HitBoxStructure;	　
	class HitCircleStructure;	            
    class HitBoxStructure	　
    {
    private:
        HitBoxClass* m_pHitbox;             
        GameComponent* m_pGameComponent;          
    public:
        HitBoxStructure(GameComponent* cmp, HitBoxClass* box)
        {
            m_pHitbox = box;
            m_pGameComponent = cmp;
        }

        bool isHit(HitBoxStructure* target);   　
		bool isHit(HitCircleStructure* target);   　
		GameComponent* getGameComponent();        
        HitBoxClass* getHitBox();           
    };

    class HitCircleStructure	            
    {
    private:
        HitCircleClass* m_pHitCircle;             
        GameComponent* m_pGameComponent;          
    public:
        HitCircleStructure(GameComponent* cmp, HitCircleClass* circle)
        {
            m_pGameComponent = cmp;
            m_pHitCircle = circle;
        }

		bool isHit(HitBoxStructure* target);   　
		bool isHit(HitCircleStructure* target);   　
        GameComponent* getGameComponent();             
        HitCircleClass* getHitCircle();           
    };

    list<HitBoxStructure*> playerBodyHit;		
    list<HitBoxStructure*> enemyBodyHit;		
    list<HitBoxStructure*> playerWeaponHit;	
    list<HitBoxStructure*> enemyWeaponHit;		
	HitManager();
	void flushHitList(list<HitBoxStructure*>* p_hitlist);  　

public:
    void refreshHitSystem();                            　
    void setBoxHit(GameComponent * cmp, HitBoxClass* box);  

    void hitFrameAction();                              　

	static HitManager* getInstance()
	{
		static HitManager* hitMng = nullptr;

		if (hitMng == nullptr)
		{
			hitMng = new HitManager();
		}

		return hitMng;
	}
};
