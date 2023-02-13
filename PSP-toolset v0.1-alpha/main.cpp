#include <iostream>
#include <unistd.h>

#include "psp_dir.h"
#include "psp_session.h"
#include "psp_common.h"

namespace cm = common;
namespace func = common::functionality;
namespace dir = psp_dir;

int main(int argc, char **argv) {
	//Declare main program streams
	cm::Program_stream new_s(std::cout, std::cerr, std::cin);
	string psp_path;
	//Get psp dir
	//Check already mounted devices
	try{ psp_path = dir::get_psp_dir_by_mounts(); } catch(cm::EXIT_STATUS& EX){ MS_ERR << EX.message << std::endl; };
	//Check unmounted devices and mount them
	if(psp_path.empty()) try{ psp_path = dir::get_psp_dir_by_devices(); } catch(cm::EXIT_STATUS& EX){ MS_ERR << EX.message << std::endl; return 0; };
	MS << "Found psp dir: " << psp_path << std::endl;
	//Create new session
	toolset::Session* new_session = new toolset::Session(psp_path);
	while(true) {
		//Get input
		MS << toolset::Session::get_operative_path() << toolset::Session::get_sym();
		std::string input;
		getline(MS_IN, input);
		try{new_session->pass(input);}catch(cm::EXIT_STATUS& EX){ MS_ERR << EX.message << std::endl; }
	}
	return 0;
}
