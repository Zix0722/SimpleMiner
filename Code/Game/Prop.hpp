#pragma once
#include "Entity.hpp"

class Prop : public Entity
{
public:
	Prop(Game* owner);
	virtual ~Prop();

	void Update(float deltaSeconds);
	void Render() const;
public:
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_texture = nullptr;
};