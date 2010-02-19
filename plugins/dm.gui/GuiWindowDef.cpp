#include "GuiWindowDef.h"

#include "parser/DefTokeniser.h"
#include "string/string.h"
#include "itextstream.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace gui
{

// Returns a GUI expression, which can be a number, a string or a formula ("gui::objVisible" == 1).
std::string GuiWindowDef::getExpression(parser::DefTokeniser& tokeniser)
{
	std::string returnValue = tokeniser.nextToken();

	if (returnValue == "(")
	{
		// Assemble token until closing brace found
		std::size_t depth = 1;
		
		while (depth > 0 && tokeniser.hasMoreTokens())
		{
			std::string token = tokeniser.nextToken();

			if (token == ")") depth--;

			returnValue += token;
		}
	}

	//  Strip quotes
	boost::algorithm::trim_if(returnValue, boost::algorithm::is_any_of("\""));

	return returnValue;
}

Vector4 GuiWindowDef::parseVector4(parser::DefTokeniser& tokeniser)
{
	// Collect tokens until all four components are parsed
	std::vector<std::string> comp;

	while (comp.size() < 4 && tokeniser.hasMoreTokens())
	{
		std::string token = getExpression(tokeniser);

		if (token == ",") continue;

		if (token.find(',') != std::string::npos)
		{
			std::vector<std::string> parts;
			boost::algorithm::split(parts, token, boost::algorithm::is_any_of(","));

			for (std::size_t i = 0; i < parts.size(); ++i)
			{
				comp.push_back(boost::algorithm::trim_copy(parts[i]));
			}

			continue;
		}

		// TODO: Catch GUI expressions

		comp.push_back(token);
	}

	if (comp.size() != 4)
	{
		throw parser::ParseException("Couldn't parse Vector4, not enough components found.");
	}

	return Vector4(strToDouble(comp[0]), strToDouble(comp[1]), strToDouble(comp[2]), strToDouble(comp[3]));
}

float GuiWindowDef::parseFloat(parser::DefTokeniser& tokeniser)
{
	// TODO: Catch GUI expressions

	return strToFloat(getExpression(tokeniser));
}

int GuiWindowDef::parseInt(parser::DefTokeniser& tokeniser)
{
	// TODO: Catch GUI expressions

	return strToInt(getExpression(tokeniser));
}

std::string GuiWindowDef::parseString(parser::DefTokeniser& tokeniser)
{
	// TODO: Catch GUI expressions

	return getExpression(tokeniser);
}

bool GuiWindowDef::parseBool(parser::DefTokeniser& tokeniser)
{
	// TODO: Catch GUI expressions

	return strToInt(getExpression(tokeniser)) != 0;
}

void GuiWindowDef::addWindow(const GuiWindowDefPtr& window)
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
		boost::algorithm::to_lower(token);

		if (token == "rect")
		{
			rect = parseVector4(tokeniser);
		}
		else if (token == "visible")
		{
			visible = parseBool(tokeniser);
		}
		else if (token == "notime")
		{
			notime = parseBool(tokeniser);
		}
		else if (token == "forecolor")
		{
			forecolor = parseVector4(tokeniser);
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
			rotate = parseFloat(tokeniser);
		}
		else if (token == "text")
		{
			setText(parseString(tokeniser));
		}
		else if (token == "font")
		{
			font = parseString(tokeniser);

			// Cut off the "fonts/" part
			boost::algorithm::replace_first(font, "fonts/", "");
		}
		else if (token == "textscale")
		{
			textscale = parseFloat(tokeniser);
		}
		else if (token == "textalign")
		{
			textalign = parseInt(tokeniser);
		}
		else if (token == "background")
		{
			background = parseString(tokeniser);
		}
		else if (token == "noevents")
		{
			noevents = parseBool(tokeniser);
		}
		else if (token == "modal")
		{
			noevents = parseBool(tokeniser);
		}
		else if (token == "menugui")
		{
			menugui = parseBool(tokeniser);
		}
		else if (token == "windowdef")
		{
			// Child windowdef
			GuiWindowDefPtr window(new GuiWindowDef);
			window->constructFromTokens(tokeniser);

			addWindow(window);
		}
		else if (token == "ontime")
		{
			// TODO
			std::string time = tokeniser.nextToken();
			tokeniser.assertNextToken("{");

			std::size_t depth = 1;

			while (tokeniser.hasMoreTokens() && depth > 0)
			{
				std::string token = tokeniser.nextToken();
				if (token == "}") depth--;
				if (token == "{") depth++;
			}
		}
		else if (token == "onnamedevent")
		{
			// TODO
			std::string eventName = tokeniser.nextToken();
			tokeniser.assertNextToken("{");

			std::size_t depth = 1;

			while (tokeniser.hasMoreTokens() && depth > 0)
			{
				std::string token = tokeniser.nextToken();
				if (token == "}") depth--;
				if (token == "{") depth++;
			}
		}
		else if (token == "onesc")
		{
			// TODO
			tokeniser.assertNextToken("{");

			std::size_t depth = 1;

			while (tokeniser.hasMoreTokens() && depth > 0)
			{
				std::string token = tokeniser.nextToken();
				if (token == "}") depth--;
				if (token == "{") depth++;
			}
		}
		else if (token == "onmouseenter" || token == "onmouseexit")
		{
			// TODO
			tokeniser.assertNextToken("{");

			std::size_t depth = 1;

			while (tokeniser.hasMoreTokens() && depth > 0)
			{
				std::string token = tokeniser.nextToken();
				if (token == "}") depth--;
				if (token == "{") depth++;
			}
		}
		else if (token == "onaction")
		{
			// TODO
			tokeniser.assertNextToken("{");

			std::size_t depth = 1;

			while (tokeniser.hasMoreTokens() && depth > 0)
			{
				std::string token = tokeniser.nextToken();
				if (token == "}") depth--;
				if (token == "{") depth++;
			}
		}
		else if (token == "float" || token == "definefloat")
		{
			// TODO: Add variable
			std::string variableName = tokeniser.nextToken();
		}
		else if (token == "definevec4")
		{
			// TODO: Add variable
			std::string variableName = tokeniser.nextToken();
			parseVector4(tokeniser);
		}
		else if (token == "}")
		{
			break;
		}
		else
		{
			globalWarningStream() << "Unknown token encountered in GUI: " << token << std::endl;
		}
	}
}

const std::string& GuiWindowDef::getText() const
{
	return _text;
}

void GuiWindowDef::setText(const std::string& newText)
{
	_text = newText;
	_textChanged = true;
}

RenderableText& GuiWindowDef::getRenderableText()
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

}
