#pragma once
#include "Entity.hpp"

class Player : public Entity
{
public:
	Player(Game* owner);
	virtual ~Player();

	void Update(float deltaSeconds);
	void Render() const;
public:
	Camera* m_camera = nullptr;
	float m_movementSpeed = 4.f;
	float m_turnRatePerSec = 90.f;
};