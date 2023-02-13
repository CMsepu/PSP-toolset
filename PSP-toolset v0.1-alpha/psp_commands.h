#pragma once

#include <iostream>
#include <vector>
#include <functional>

#include "psp_common.h"

namespace cm = common;

namespace cmd{
	class Command{
	private:
		using StringLambda = function<void(std::vector<std::string>&)>;
	
		//Command data
		std::string name;
		std::vector<std::string> aliases;
		int argc;
		
		//Command functionality
		StringLambda func;
	public:
		//List of all commands
		static std::vector<Command*> cmd_list;
		
		//Locate command
		static Command* find_command(string _name, int _argc) {
			for(Command* c : cmd_list) {
				if(c->get_name() == _name && c->get_argc() == _argc) return c;
				if(c->get_argc() == _argc) {
					for(std::string alias : c->aliases) {
						if(alias == _name) return c;
					}
				}
			}
			throw cm::EXIT_STATUS(cm::FAILURE, "COULDNT FIND COMMAND " + _name + " WITH " + to_string(_argc) + " ARGUMENTS");
			return nullptr;
		}
		//Locate and run command
		static void run_command(string _name, int _argc, std::vector<std::string> args) {
			try{ find_command(_name, _argc)->operator()(args); }catch(cm::EXIT_STATUS& EX){ throw EX; }
		}
		
		//Initialize command
		Command(std::string name, int argc, StringLambda lambda, std::vector<std::string> aliases = {}): name(name), aliases(aliases), argc(argc), func(lambda) {
			cmd_list.push_back(this);
		}
		
		//Call command
		void operator()(std::vector<std::string> args) {
			return func(args);
		}
		
		//Get data
		int get_argc() { return argc; }
		std::string get_name() { return name; }
	};
	std::vector<Command*> Command::cmd_list;
	
	//Define macros
	#define cmd_declare(name, argc, func) Command name(#name, argc, func);
	#define cmd_declare_a(name, aliases, argc, func) Command name(#name, argc, func, aliases);
	using vec_str = std::vector<std::string>;
	//COMMANDS:
}