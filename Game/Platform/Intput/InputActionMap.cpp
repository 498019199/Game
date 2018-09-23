// KlayGE ���붯��ӳ����  2018��9��16����ֲ@zhangbei
#include "../Core/predefine.h"
#include "../Intput/Input.hpp"
#include <boost/assert.hpp>

// ���Ӷ���
//////////////////////////////////////////////////////////////////////////////////
void InputActionMap::AddAction(InputActionDefine const & action_define)
{
	actionMap_.emplace(action_define.semantic, action_define.action);
}

// �������붯��
//////////////////////////////////////////////////////////////////////////////////
void InputActionMap::UpdateInputActions(InputActionsType& actions, uint16_t key, InputActionParamPtr const & param)
{
	if (this->HasAction(key))
	{
		actions.emplace_back(this->Action(key), param);
	}
}

// ӳ���д������key
//////////////////////////////////////////////////////////////////////////////////
bool InputActionMap::HasAction(uint16_t key) const
{
	return (actionMap_.find(key) != actionMap_.end());
}

// ��key��ȡ����
//////////////////////////////////////////////////////////////////////////////////
uint16_t InputActionMap::Action(uint16_t key) const
{
	return actionMap_.find(key)->second;
}
