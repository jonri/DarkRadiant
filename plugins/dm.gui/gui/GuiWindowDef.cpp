#include "GuiWindowDef.h"

#include "parser/DefTokeniser.h"
#include "string/convert.h"
#include "itextstream.h"

#include <limits>
#include "string/split.h"
#include "string/trim.h"
#include "string/case_conv.h"
#include "string/replace.h"
#include "string/predicate.h"

#include "GuiScript.h"

namespace gui
{

namespace
{

void skipBlock(parser::DefTokeniser& tokeniser)
{
	tokeniser.assertNextToken("{");

	std::size_t level = 1;

	while (tokeniser.hasMoreTokens())
	{
		std::string token = tokeniser.nextToken();

		if (token == "{") level++;
		if (token == "}") level--;

		if (level == 0) return;
	}
}

}

GuiWindowDef::GuiWindowDef(IGui& owner) :
	_owner(owner),
	_renderableText(*this)
{
	visible.setValue(true);
	forecolor = Vector4(1, 1, 1, 1);
	hovercolor = Vector4(1, 1, 1, 1);
	backcolor = Vector4(0, 0, 0, 0);
	bordercolor = Vector4(0, 0, 0, 0);
	bordersize.setValue(0.0f);
	matcolor = Vector4(1, 1, 1, 1);
	rotate.setValue(0.0f);
	textscale.setValue(1.0f);
	textalign.setValue(0);
	textalignx.setValue(0.0f);
	textaligny.setValue(0.0f);
	forceaspectwidth.setValue(640.0f);
	forceaspectheight.setValue(480.0f);
	noclip.setValue(false);
	notime.setValue(false);
	nocursor.setValue(false);
	nowrap.setValue(false);
	time = 0;

	_textChanged = true;
	text.signal_variableChanged().connect([this]() { _textChanged = true; });
}

IGui& GuiWindowDef::getGui() const
{
	return _owner;
}

// Returns a GUI expression, which can be a number, a string or a formula ("gui::objVisible" == 1).
GuiExpressionPtr GuiWindowDef::getExpression(parser::DefTokeniser& tokeniser)
{
	return GuiExpression::createFromTokens(tokeniser);
}

Vector4 GuiWindowDef::parseVector4(parser::DefTokeniser& tokeniser)
{
	// Collect tokens until all four components are parsed
	std::vector<GuiExpressionPtr> comp;

	while (comp.size() < 4 && tokeniser.hasMoreTokens())
	{
		std::string token = tokeniser.peek();

		if (token == ",")
		{
			tokeniser.nextToken();
			continue;
		}

		GuiExpressionPtr expr = getExpression(tokeniser);

		comp.push_back(expr);
	}

	if (comp.size() != 4)
	{
		throw parser::ParseException("Couldn't parse Vector4, not enough components found.");
	}

	return Vector4(comp[0]->getFloatValue(),
                   comp[1]->getFloatValue(),
                   comp[2]->getFloatValue(),
                   comp[3]->getFloatValue());
}

std::shared_ptr<IGuiExpression<float>> GuiWindowDef::parseFloat(parser::DefTokeniser& tokeniser)
{
	GuiExpressionPtr expr = getExpression(tokeniser);

	if (!expr)
	{
		throw parser::ParseException("Failed to parse float expression.");
	}

	return std::make_shared<TypedExpression<float>>(expr);
}

std::shared_ptr<IGuiExpression<int>> GuiWindowDef::parseInt(parser::DefTokeniser& tokeniser)
{
	GuiExpressionPtr expr = getExpression(tokeniser);

	if (!expr)
	{
		throw parser::ParseException("Failed to parse integer expression.");
	}

	return std::make_shared<TypedExpression<int>>(expr);
}

std::shared_ptr<IGuiExpression<std::string>> GuiWindowDef::parseString(parser::DefTokeniser& tokeniser)
{
	std::string token = tokeniser.peek();
	GuiExpressionPtr expr;

	if (string::starts_with(token, "gui::"))
	{
		expr = std::make_shared<GuiStateVariableExpression>(tokeniser.nextToken().substr(5));
	}
	else
	{
		expr = std::make_shared<StringExpression>(tokeniser.nextToken());
	}

	return std::make_shared<TypedExpression<std::string>>(expr);
}

std::shared_ptr<IGuiExpression<bool>> GuiWindowDef::parseBool(parser::DefTokeniser& tokeniser)
{
	GuiExpressionPtr expr = getExpression(tokeniser);

	if (!expr)
	{
		throw parser::ParseException("Failed to parse integer expression.");
	}

	return std::make_shared<TypedExpression<bool>>(expr);
}

void GuiWindowDef::addWindow(const IGuiWindowDefPtr& window)
{
	children.push_back(window);
}

void GuiWindowDef::constructFromTokens(parser::DefTokeniser& tokeniser)
{
	// The windowDef keyword has already been parsed, so expect a name plus an opening brace here
	name = tokeniser.nextToken();

	tokeniser.assertNextToken("{");

	while (tokeniser.hasMoreTokens())
	{
		std::string token = tokeniser.nextToken();
		string::to_lower(token);

		if (token == ";") 
		{
			continue; // ignore stray semicolons
		}

		if (token == "rect")
		{
			rect = parseVector4(tokeniser);
		}
		else if (token == "visible")
		{
			visible.setValue(parseBool(tokeniser));
		}
		else if (token == "notime")
		{
			notime.setValue(parseBool(tokeniser));
		}
		else if (token == "forecolor")
		{
			forecolor = parseVector4(tokeniser);
		}
		else if (token == "hovercolor")
		{
			hovercolor = parseVector4(tokeniser);
		}
		else if (token == "backcolor")
		{
			backcolor = parseVector4(tokeniser);
		}
		else if (token == "bordercolor")
		{
			bordercolor = parseVector4(tokeniser);
		}
		else if (token == "matcolor")
		{
			matcolor = parseVector4(tokeniser);
		}
		else if (token == "rotate")
		{
			rotate.setValue(parseFloat(tokeniser));
		}
		else if (token == "text")
		{
			text.setValue(parseString(tokeniser));
		}
		else if (token == "font")
		{
			font.setValue(parseString(tokeniser));
		}
		else if (token == "textscale")
		{
			textscale.setValue(parseFloat(tokeniser));
		}
		else if (token == "textalign")
		{
			textalign.setValue(parseInt(tokeniser));
		}
		else if (token == "textalignx")
		{
			textalignx.setValue(parseFloat(tokeniser));
		}
		else if (token == "textaligny")
		{
			textaligny.setValue(parseFloat(tokeniser));
		}
		else if (token == "forceaspectwidth")
		{
			forceaspectwidth.setValue(parseFloat(tokeniser));
		}
		else if (token == "forceaspectheight")
		{
			forceaspectheight.setValue(parseFloat(tokeniser));
		}
		else if (token == "background")
		{
			background.setValue(parseString(tokeniser));
		}
		else if (token == "noevents")
		{
			noevents.setValue(parseBool(tokeniser));
		}
		else if (token == "nocursor")
		{
			nocursor.setValue(parseBool(tokeniser));
		}
		else if (token == "noclip")
		{
			noclip.setValue(parseBool(tokeniser));
		}
		else if (token == "nowrap")
		{
			nowrap.setValue(parseBool(tokeniser));
		}
		else if (token == "modal")
		{
			noevents.setValue(parseBool(tokeniser));
		}
		else if (token == "menugui")
		{
			menugui.setValue(parseBool(tokeniser));
		}
		else if (token == "windowdef" || token == "indowdef") // yes, there's a syntax error in the TDM GUI
		{
			// Child windowdef
			GuiWindowDefPtr window(new GuiWindowDef(_owner));
			window->constructFromTokens(tokeniser);

			addWindow(window);
		}
		else if (token == "ontime")
		{
			std::string timeStr = tokeniser.nextToken();

			// Check the time for validity
			std::size_t onTime = string::convert<std::size_t>(
                timeStr, std::numeric_limits<std::size_t>::max()
            );

			if (onTime == std::numeric_limits<std::size_t>::max())
			{
				rWarning() << "Invalid time encountered in onTime event in "
					<< name << ": " << timeStr << std::endl;
			}

			// Allocate a new GuiScript
			GuiScriptPtr script(new GuiScript(*this));

			script->constructFromTokens(tokeniser);

			_timedEvents.insert(TimedEventMap::value_type(onTime, script));
		}
		else if (token == "onnamedevent")
		{
			std::string eventName = tokeniser.nextToken();

			// Parse the script
			GuiScriptPtr script(new GuiScript(*this));
			script->constructFromTokens(tokeniser);

			// TODO: Save event
		}
		else if (token == "onevent")
		{
			GuiScriptPtr script(new GuiScript(*this));
			script->constructFromTokens(tokeniser);

			// TODO
		}
		else if (token == "onesc")
		{
			GuiScriptPtr script(new GuiScript(*this));
			script->constructFromTokens(tokeniser);

			// TODO
		}
		else if (token == "onmouseenter" || token == "onmouseexit")
		{
			GuiScriptPtr script(new GuiScript(*this));
			script->constructFromTokens(tokeniser);

			// TODO
		}
		else if (token == "onaction")
		{
			GuiScriptPtr script(new GuiScript(*this));
			script->constructFromTokens(tokeniser);

			// TODO
		}
		else if (token == "onactionrelease")
		{
			GuiScriptPtr script(new GuiScript(*this));
			script->constructFromTokens(tokeniser);

			// TODO
		}
		else if (token == "float" || token == "definefloat")
		{
			// TODO: Add variable
			std::string variableName = tokeniser.nextToken();

			// try to check if the next token is a numeric initialisation value
			try
			{
				float value = std::stof(tokeniser.peek());

				// Success, load the value
				tokeniser.nextToken();
			}
			catch (std::invalid_argument&)
			{
				// Not a number, ignore and continue
			}
		}
		else if (token == "definevec4")
		{
			// TODO: Add variable
			std::string variableName = tokeniser.nextToken();
			parseVector4(tokeniser);
		}
		else if (token == "listdef" || token == "choicedef" || token == "binddef" ||
				 token == "editdef" || token == "sliderdef" || token == "renderdef")
		{
			tokeniser.nextToken(); // def name
			skipBlock(tokeniser);
		}
		else if (token == "bordersize")
		{
			bordersize.setValue(parseFloat(tokeniser));
		}
		else if (token == "choicetype")
		{
			rWarning() << "'choiceType' token encountered in windowDef block in " << name << std::endl;
			tokeniser.nextToken(); // the choicetype value
		}
		else if (token == "}")
		{
			break;
		}
		else
		{
			rWarning() << "Unknown token encountered in GUI: " << token << std::endl;
		}
	}
}

IRenderableText& GuiWindowDef::getRenderableText()
{
	if (_textChanged)
	{
		// Text has changed, refresh the renderable
		_textChanged = false;

		// (Re-)compile the renderable
		_renderableText.recompile();
	}

	return _renderableText;
}

void GuiWindowDef::update(const std::size_t timeStep, bool updateChildren)
{
	if (!notime)
	{
		std::size_t oldTime = time;

		// Update this windowDef's time
		time += timeStep;

		// Be sure to include the ontime 0 event the first time
		if (oldTime > 0)
		{
			oldTime++;
		}

		// Check events whose time is within [oldTime..time]
		for (TimedEventMap::const_iterator i = _timedEvents.lower_bound(oldTime);
			 i != _timedEvents.end() && i != _timedEvents.upper_bound(time); ++i)
		{
			i->second->execute();
		}
	}

	// Update children regardless of this windowDef's notime setting
	if (updateChildren)
	{
		for (ChildWindows::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			(*i)->update(timeStep, updateChildren);
		}
	}
}

void GuiWindowDef::initTime(const std::size_t toTime, bool updateChildren)
{
	this->time = toTime;

	if (updateChildren)
	{
		for (ChildWindows::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			(*i)->initTime(toTime, updateChildren);
		}
	}
}

IGuiWindowDefPtr GuiWindowDef::findWindowDef(const std::string& windowName)
{
	// First look at all direct children
	for (ChildWindows::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if ((*i)->name == windowName)
		{
			return (*i);
		}
	}

	// Not found, ask each child to search for the windowDef
	for (ChildWindows::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		IGuiWindowDefPtr window = (*i)->findWindowDef(windowName);

		if (window != NULL) return window;
	}

	// Not found
	return IGuiWindowDefPtr();
}

void GuiWindowDef::pepareRendering(bool prepareChildren)
{
	// Triggers a re-compilation of the text VBOs, if necessary
	getRenderableText();

	if (!prepareChildren) return;

	for (ChildWindows::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		(*i)->pepareRendering(prepareChildren);
	}
}

} // namespace
