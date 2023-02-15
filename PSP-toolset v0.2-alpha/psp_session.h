#pragma once

#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>
#include "psp_commands.h"
namespace fs = std::filesystem;
namespace cm = common;

namespace toolset{
	class Session{
	private:
		fs::path psp_path;
		static fs::path operative_path;
		static char sym;
	public:
		//Start session
		Session(string _psp_path): psp_path(_psp_path) {
			operative_path = psp_path;
		}
		
		//Pass input to session class
		void pass(std::string line) {
			if(line.empty()) return;
			std::stringstream parse(line);
			std::string name;
			std::vector<std::string> args;
			parse >> name;
			
			//Get command and args from input
			std::string temp;
			while(parse >> temp) args.push_back(temp);
		}
		
		//Change operative path
		static void change_operative_path(fs::path p) {
			if(!fs::exists(p)) throw cm::EXIT_STATUS(cm::FAILURE, p.string() + " DOESNT EXIST");
			operative_path = p;
		}
		
		//Change input symbol
		static void change_symbol(const char symbol) {
			sym = symbol;
		}
		
		//Get data
		static char get_sym() { return sym; }
		static std::string get_operative_path() { return operative_path.string(); }
	};
	
	fs::path Session::operative_path;
	char Session::sym = '>';
}