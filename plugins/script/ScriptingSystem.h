#pragma once

#include <map>
#include <vector>
#include <pybind11/pybind11.h>

#include "iscript.h"
#include "iscriptinterface.h"
#include "PythonConsoleWriter.h"
#include "icommandsystem.h"

#include "ScriptCommand.h"

namespace py = pybind11;

namespace script 
{

// Forward declaration
class StartupListener;
typedef std::shared_ptr<StartupListener> StartupListenerPtr;

class ScriptingSystem :
	public IScriptingSystem
{
private:
	std::string _outputBuffer;
	std::string _errorBuffer;

	PythonConsoleWriter _outputWriter;
	PythonConsoleWriter _errorWriter;

	bool _initialised;

	typedef std::pair<std::string, IScriptInterfacePtr> NamedInterface;
	typedef std::vector<NamedInterface> Interfaces;
	Interfaces _interfaces;

	// The path where the script files are hosted
	std::string _scriptPath;

	// All named script commands (pointing to .py files)
	ScriptCommandMap _commands;

	sigc::signal<void> _sigScriptsReloaded;

public:
	ScriptingSystem();

	// Adds a script interface to this system
	void addInterface(const std::string& name, const IScriptInterfacePtr& iface) override;

	// (Re)loads all scripts from the scripts/ folder
	void reloadScriptsCmd(const cmd::ArgumentList& args);

	/**
	 * This actually initialises the Scripting System, adding all
	 * registered interfaces to the Python context. After this call
	 * the scripting system is ready for use.
	 *
	 * This method also invokes "scripts/init.py" when done.
	 */
	void initialise();

	// Runs a specific script file (command target)
	void runScriptFile(const cmd::ArgumentList& args);

	// Runs a named script command (command target)
	void runScriptCommand(const cmd::ArgumentList& args);

	// Executes a script file
	void executeScriptFile(const std::string& filename) override;

	// Execute the given python script string
	ExecutionResultPtr executeString(const std::string& scriptString) override;

	void foreachScriptCommand(const std::function<void(const IScriptCommand&)>& functor) override;

	sigc::signal<void>& signal_onScriptsReloaded() override;

	// Runs the named command (or rather the .py file behind it)
	void executeCommand(const std::string& name);

	// RegisterableModule implementation
	const std::string& getName() const override;
	const StringSet& getDependencies() const override;
	void initialiseModule(const IApplicationContext& ctx) override;
	void shutdownModule() override;

private:
	void executeScriptFile(const std::string& filename, bool setExecuteCommandAttr);

	bool interfaceExists(const std::string& name);

	void addInterfacesToModule(py::module& mod, py::dict&);

	void reloadScripts();

	void loadCommandScript(const std::string& scriptFilename);
};
typedef std::shared_ptr<ScriptingSystem> ScriptingSystemPtr;

} // namespace script
