// KlayGE 输入动作映射类  2018年9月16日移植@zhangbei
#include "../Core/predefine.h"
#include "../Intput/Input.hpp"
#include <boost/assert.hpp>

// 增加动作
//////////////////////////////////////////////////////////////////////////////////
void InputActionMap::AddAction(InputActionDefine const & action_define)
{
	actionMap_.emplace(action_define.semantic, action_define.action);
}

// 更新输入动作
//////////////////////////////////////////////////////////////////////////////////
void InputActionMap::UpdateInputActions(InputActionsType& actions, uint16_t key, InputActionParamPtr const & param)
{
	if (this->HasAction(key))
	{
		actions.emplace_back(this->Action(key), param);
	}
}

// 映射中存在这个key
//////////////////////////////////////////////////////////////////////////////////
bool InputActionMap::HasAction(uint16_t key) const
{
	return (actionMap_.find(key) != actionMap_.end());
}

// 从key获取动作
//////////////////////////////////////////////////////////////////////////////////
uint16_t InputActionMap::Action(uint16_t key) const
{
	return actionMap_.find(key)->second;
}
