#include "user_functions.h"

FunctionType getSelectedFunction(tgui::ListBox::Ptr listbox)
{
	const auto selected_label = listbox->getSelectedItem();

	const auto match = std::find_if(function_info.begin(), function_info.end(),
		[&selected_label](auto function)
		{
			return function.label == selected_label;
		});

	if (match == function_info.end())
		return FunctionType::NO_FUNCTION;

	return match->type;
}

void fillFunctionGUIDropdown(tgui::ListBox::Ptr listbox)
{
	for (const auto& function : function_info)
		listbox->addItem(function.label);
}

void setFunctionGUIFromHotkeys(tgui::ListBox::Ptr listbox)
{
	for (const auto& function : function_info)
	{
		if (sf::Keyboard::isKeyPressed(function.hotkey))
			listbox->setSelectedItem(function.label);
	}
}