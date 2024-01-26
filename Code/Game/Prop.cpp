#include "Prop.hpp"
#include "Engine/Core/VertexUtils.hpp"

Prop::Prop(Game* owner)
	:Entity(owner)
{
}

Prop::~Prop()
{

}

void Prop::Update(float deltaSeconds)
{
	m_orientation.m_yawDegrees += deltaSeconds * m_angularVelocity.m_yawDegrees;
	m_orientation.m_pitchDegrees += deltaSeconds * m_angularVelocity.m_pitchDegrees;
	m_orientation.m_rollDegrees += deltaSeconds * m_angularVelocity.m_rollDegrees;
}

void Prop::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
}

