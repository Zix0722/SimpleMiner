#include "Entity.hpp"
#include "Engine/Math/MathUtils.hpp"

Entity::Entity(Game* owner)
	:m_game(owner)
{

}

Entity::~Entity()
{

}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 result = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	result.SetTranslation3D(m_position);
	return result;
}
