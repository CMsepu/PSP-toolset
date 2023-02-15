#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <functional>

#include "psp_common.h"
#include "psp_session.h"

namespace cm = common;

namespace cmd{
	//Declare shorten type names
	using StringLambda = function<void(std::vector<std::string>&)>;
	using StringSet = unordered_set<std::string>;
	
	//Command base data
	class Data{
	private:
		//Store command basic data
		std::string name;
		StringSet aliases;
	public:
		Data(std::string name, StringSet aliases): name(name), aliases(aliases) {}
		
		//Check called name
		bool check_name(std::string _name) { return (_name == name || aliases.find(_name)!=aliases.end()); }
		
		//Get data
		std::string get_name() { return name; }
	};
	
	//Comand functionality
	class Func{
	private:
		//Store function info
		int argc;
		StringLambda lambda;
	public:
		Func(int argc, StringLambda lambda): argc(argc), lambda(lambda) {}
		
		//Run lambda
		void operator()(std::vector<std::string>& args) {
			if(int(args.size()) != argc) throw cm::EXIT_STATUS(cm::FAILURE, "WRONG AMMOUNT OF ARGS");
			return lambda(args);
		}
		
		//Get function data
		int get_argc() { return argc; }
	};
	
	//Command body
	class Command{
	private:
		//Store cmd structures
		Data* cmd_data;
		std::vector<Func*>* cmd_func = nullptr;
		
		//List of all commands
		static std::vector<Command*>* cmd_list; 
	public:
		//Initialize command
		Command(std::string name, StringSet aliases={}): cmd_data(new Data(name, aliases)) {
			if(cmd_list == nullptr) cmd_list = new vector<Command*>;
			cmd_list->push_back(this);
		}
		
		//Add new command instance
		void add_instance(int argc, StringLambda lambda) {
			if(cmd_func == nullptr) cmd_func = new std::vector<Func*>;
			cmd_func->push_back(new Func(argc, lambda));
		}
		
		static void run_command(std::string name, std::vector<std::string> args) {
			if(cmd_list == nullptr) throw cm::EXIT_STATUS(cm::FAILURE, "NO COMMANDS AVAILABLE!");
			for(Command* _cmd : *cmd_list) {
				if(_cmd->cmd_data->check_name(name)) try{ _cmd->operator()(args); } catch(cm::EXIT_STATUS& EX){ throw EX; };
			}
			throw cm::EXIT_STATUS(cm::FAILURE, "NO COMMAND " + name + " FOUND");
		}
		
		//Run function with correct argc
		void operator()(std::vector<std::string>& args) {
			for(Func* instance : *cmd_func) {
				if(instance->get_argc() == int(args.size())) return instance->operator()(args);
			}
			throw cm::EXIT_STATUS(cm::FAILURE, "WRONG AMMOUNTS OF ARGUMENTS TO COMMAND " + cmd_data->get_name());
		}
	};
	
	std::vector<Command*>* Command::cmd_list = nullptr;
}